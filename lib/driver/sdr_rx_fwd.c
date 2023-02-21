#include <stdio.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/errno.h>
#include <osal.h>
#include <sdr_driver.h>
#include "sdr_rx_fwd.h"

struct csp_ftp_hdr {
  char padding[10];
  uint16_t len;
  uint32_t id;  // last field of the CSP hdr
  uint16_t bs;
  uint16_t pktno;
  uint8_t data[0];
};

// The Northern Voices CSP destination port
#define NV_DPORT 24

static void sdr_rx_buffer(void *arg, uint8_t *data, size_t len, void *unused) {
    struct rx_fwd_ctx *ctx = (struct rx_fwd_ctx *) arg;

    struct csp_ftp_hdr *hdr = (struct csp_ftp_hdr *) data;

    /* We only care want to receive Northern Voices broadcasts. Therefore, we
     * discard all CSP packets not aimed at NV_DPORT.
     */
    int id = ntohl(hdr->id);
    if (((id >> 14) & 0x3f) != NV_DPORT) {
        /* The dport is a 6-bit field 3rd from the right, after sport (6 bits)
	 * and flags (8 bits).
	 */
        return;
    }

    // len includes the CSP header, the ftp header, and the CSP checksum
    int data_len = ntohs(hdr->bs);
    if (ctx->blksz == 0) ctx->blksz = data_len;

    if ((ctx->offset + data_len) >= ctx->buflen) {
	// We're at the end of the buffer.
	data_len = ctx->buflen - ctx->offset;
	printf("warning: only %d buffer bytes left\n", data_len);
    }

    if (data_len > 0) {
        memcpy(ctx->buf + ctx->offset, hdr->data, data_len);
        ctx->offset += data_len;
    }

    /* We don't know the total size of the download, all we know is that the
     * transmitter sends uniform ctx->blksz chunks until the last block, which
     * is however big it needs to be to hold the remaining data.
     * We are fervently hoping that the last bs is 0 if the size of the file
     * evenly divides ctx->blksz.
     */
    if (data_len < ctx->blksz) {
        ctx->rx_done(arg);
    }
}

static int rx_fwd_open(const char *host, uint16_t port) {
    struct sockaddr_in servaddr;

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("TCP socket creation failed: %d\n", errno);
        return -1;
    }
    memset(&servaddr, 0, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(host);
    servaddr.sin_port = htons(port);

    // connect the client socket to server socket
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection to port %d failed: %d\n", port, errno);
        return -2;
    }

    return sockfd;
}

void sdr_rx_signal_waiters(struct rx_fwd_ctx *ctx) {
    pthread_mutex_lock(&(ctx->wait_lock));
    pthread_cond_signal(&(ctx->wait_cond));
    pthread_mutex_unlock(&(ctx->wait_lock));
}

void rx_fwd_buffer(void *arg) {
    struct rx_fwd_ctx *ctx = (struct rx_fwd_ctx *) arg;

    // We have all the data so the caller can restore the receiver cal'backs
    sdr_rx_signal_waiters(ctx);

    // Always connect to localhost for now
    int fd = rx_fwd_open("127.0.0.1", SDR_RX_FWD_PORT);
    if (fd < 0) return;

    int remaining = ctx->offset;
    int offset = 0;
    while (remaining > 0) {
	int rc = write(fd, ctx->buf + offset, remaining);
	if (rc < 0) {
	    printf("rx_fwd_buffer write error: %s", strerror(errno));
	    break;
	}
	else if (rc == 0) {
	    printf("rx_fwd_buffer lost connection?\n");
	    break;
	}

	remaining -= rc;
	offset += rc;
    }

    printf("rx_fwd_buffer: wrote %d bytes\n", ctx->offset - remaining);
    close(fd);
}

struct rx_fwd_ctx rx_fwd;

void sdr_rx_fwd_init(sdr_interface_data_t *ifdata, rx_done_t forwarder) {
    static int started = 0;

    if (!started) {
        /* Because the satellite UHF transmission rate is so low (~2400BAUD)
	 * we buffer the entire file before forwarding it on. A 1MB buffer
	 * should be big enough for most files, also because the bandwidth
	 * is so low.
	 */
        rx_fwd.blksz = 0;
	rx_fwd.buflen = 1024*1024; // start with 1 MByte
	rx_fwd.offset = 0;
	if (!(rx_fwd.buf = (unsigned char *) malloc(rx_fwd.buflen))) {
	    printf("sdr_rx_fwd_init: malloc failed\n");
	    return;
	}

	/* Allow the caller to provide alternative forwarders */
	if (forwarder) {
	    rx_fwd.rx_done = forwarder;
	}
	else {
	    rx_fwd.rx_done = rx_fwd_buffer;
	}

	int rc = pthread_mutex_init(&rx_fwd.wait_lock, NULL);
	if (rc) {
	    printf("sdr_rx_fwd_init: mutex_init failed %s\n", strerror(rc));
	    return;
	}
	if ((rc = pthread_cond_init(&rx_fwd.wait_cond, NULL))) {
	    printf("sdr_rx_fwd_init: cond_init failed %s\n", strerror(rc));
	    return;
	}
	started = 1;
    }

    // Remember the current settings so they can be restored later
    sdr_conf_t sdr_conf = *(ifdata->sdr_conf);

    /* These instructions change the RX callback. I know they should be set
     * atomically, but this is really only for the test harness. When we 
     * release the customer version we won't be switching receivers.
     */
    ifdata->sdr_conf->rx_callback = sdr_rx_buffer;
    ifdata->sdr_conf->rx_callback_data = &rx_fwd;

    // The condition blocks this thread until the transmission is received.
    pthread_mutex_lock(&rx_fwd.wait_lock);
    pthread_cond_wait(&rx_fwd.wait_cond, &rx_fwd.wait_lock);
    pthread_mutex_unlock(&rx_fwd.wait_lock);

    ifdata->sdr_conf->rx_callback = sdr_conf.rx_callback;
    ifdata->sdr_conf->rx_callback_data = sdr_conf.rx_callback_data;
}
