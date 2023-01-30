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
#include <sdr_driver.h>

struct rx_fwd_ctx {
  int fd;
} rx_fwd;

static int cnt = 0;

struct csp_hdr {
  char padding[10];
  uint16_t len;
  uint32_t id;
  uint16_t bs;
  uint16_t pktno;
  uint8_t data[0];
};

static void sdr_rx_fwd(void *udata, uint8_t *data, size_t len, void *unused) {
    struct rx_fwd_ctx *ctx = (struct rx_fwd_ctx *) udata;

    struct csp_hdr *hdr = (struct csp_hdr *) data;
    printf("csp len 0x%x, id 0x%x, bs 0x%x, cnt 0x%x\n",
	    hdr->len, hdr->id, hdr->bs, hdr->pktno);
 
    int rc = write(ctx->fd, data + 20, len - 20);
    if (rc < 0) {
         printf("sdr_rx_fwd write error: %s", strerror(errno));
    }
    else if (rc != (int) (len - 20)) {
      printf("sdr_rx_fwd: short write %d<%d\n", rc, (int) (len - 20));
    }
    ++cnt;
    printf("fwd pkt %d\n", cnt);
}

static int sdr_rx_fwd_open(const char *host, uint16_t port) {
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

void sdr_rx_fwd_init(sdr_interface_data_t *ifdata) {
    static int started = 0;

    if (!started) {
        if ((rx_fwd.fd = sdr_rx_fwd_open("127.0.0.1", 40000)) < 0) return;
	
	printf("sdr_rx_fwd using fd %d\n", rx_fwd.fd);
	started = 1;
    }

    printf("sdr_fx_fwd_init new cb: %p\n", sdr_rx_fwd);
    ifdata->sdr_conf->rx_callback = sdr_rx_fwd;
    ifdata->sdr_conf->rx_callback_data = &rx_fwd;
}
