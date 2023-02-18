#include <stdio.h>
#include <assert.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/errno.h>
#include <osal.h>
#include <pthread.h>
#include <sdr_driver.h>
#include "codec2/codec2.h"
//#include "codec2/c2file.h"

typedef void (*rx_done_t)(void *arg);

struct rx_fwd_ctx {
  int fd;
  int blksz;
  int buflen;
  int offset;
  unsigned char *buf;
  rx_done_t rx_done;
  pthread_mutex_t wait_lock;
  pthread_cond_t wait_cond;
} rx_fwd;

struct csp_ftp_hdr {
  char padding[10];
  uint16_t len;
  uint32_t id;
  uint16_t bs;
  uint16_t pktno;
  uint8_t data[0];
};

static void sdr_rx_buffer(void *arg, uint8_t *data, size_t len, void *unused) {
    struct rx_fwd_ctx *ctx = (struct rx_fwd_ctx *) arg;

    struct csp_ftp_hdr *hdr = (struct csp_ftp_hdr *) data;

    // len includes the CSP header, the ftp header, and the CSP checksum
    int data_len = ntohs(hdr->bs);
    if (ctx->blksz == 0) ctx->blksz = data_len;

    if ((ctx->offset + data_len) >= ctx->buflen) {
	// We're at the end of the buffer.
	data_len = ctx->buflen - ctx->offset;
	printf("warning: only %d buffer bytes left\n", data_len);
    }
    if (data_len > 0) {
        memcpy(ctx->buf + ctx->offset, data + sizeof(*hdr), data_len);
        ctx->offset += data_len;
    }

    if (data_len < ctx->blksz) {
        printf("rx_buffer is done\n");
        ctx->rx_done(arg);
    }
}

#if 0
static
#endif
int rx_fwd_open(const char *host, uint16_t port) {
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

#define BS 512

#if 0
static
#endif
void rx_fwd_buffer(void *arg) {
    struct rx_fwd_ctx *ctx = (struct rx_fwd_ctx *) arg;

    pthread_mutex_lock(&(ctx->wait_lock));
    pthread_cond_signal(&(ctx->wait_cond));
    pthread_mutex_unlock(&(ctx->wait_lock));

#if 0
    int fd = rx_fwd_open("127.0.0.1", 40000);
#else
    int fd = creat("nv.wav", 0644);
#endif
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

#if 1
static
#endif
void rx_decode_c2_buffer(void *arg) {
    struct rx_fwd_ctx *ctx = (struct rx_fwd_ctx *) arg;

    pthread_mutex_lock(&(ctx->wait_lock));
    pthread_cond_signal(&(ctx->wait_cond));
    pthread_mutex_unlock(&(ctx->wait_lock));

    int mode = CODEC2_MODE_2400;
    struct CODEC2 *decoder = codec2_create(mode);
    int nsam = codec2_samples_per_frame(decoder);
    short samples[nsam];
    int bpf = codec2_bytes_per_frame(decoder);

    int fd = creat("nv.wav", 0644);
    if (fd < 0) {
        perror("creat");
	return;
    }

    printf("samples/frame %d, bytes/frame %d\n", nsam, bpf);
    for (int off=0; off<ctx->offset; off+=bpf) {
        printf("%d\n", off);
	fflush(stdout);
        codec2_decode(decoder, samples, ctx->buf + off);
	int len = write(fd, samples, nsam*2);
	if (len < nsam) {
	    if (len < 0) {
	        perror("write");
	    }
	    else {
	        printf("short write? %d < %d\n", len, nsam);
	    }
	    break;
	}
    }
    close(fd);
}

void sdr_rx_fwd_init(sdr_interface_data_t *ifdata) {
    static int started = 0;

    if (!started) {
        rx_fwd.blksz = 0;
	rx_fwd.buflen = 1024*1024; // start with 1 MByte
	rx_fwd.offset = 0;
	if (!(rx_fwd.buf = (unsigned char *) malloc(rx_fwd.buflen))) {
	    printf("sdr_rx_fwd_init: malloc failed\n");
	    return;
	}

	// rx_fwd.rx_done = rx_fwd_buffer;
	rx_fwd.rx_done = rx_decode_c2_buffer;

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

    sdr_conf_t sdr_conf = *(ifdata->sdr_conf);

    printf("sdr_rx_fwd_init new cb: %p\n", sdr_rx_buffer);
    ifdata->sdr_conf->rx_callback = sdr_rx_buffer;
    ifdata->sdr_conf->rx_callback_data = &rx_fwd;

    pthread_mutex_lock(&rx_fwd.wait_lock);
    pthread_cond_wait(&rx_fwd.wait_cond, &rx_fwd.wait_lock);
    pthread_mutex_unlock(&rx_fwd.wait_lock);

    printf("sdr_rx_fwd_init restored cb: %p\n", sdr_conf.rx_callback);
    ifdata->sdr_conf->rx_callback = sdr_conf.rx_callback;
    ifdata->sdr_conf->rx_callback_data = sdr_conf.rx_callback_data;
}
