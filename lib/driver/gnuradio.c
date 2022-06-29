/*
 * Copyright (C) 2021  University of Alberta
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
/**
 * @file gnuradio.c
 * @author Josh Lazaruk
 * @date 2022-05-12
 */

#ifdef OS_POSIX
#include <stdio.h>
#define ex2_log printf
#endif // CSP_POSIX

#ifdef SDR_GNURADIO
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

#define SA struct sockaddr

#define RX_TASK_STACK_SIZE 4096

#define PREAMBLE_B 0xAA
#define SYNCWORD 0x7E
#define PREAMBLE_LEN 16
#define POSTAMBLE_LEN 10
#define PACKET_LEN 128
#define CRC16_LEN 2
#define SYNCWORD_LEN 1
#define LEN_ID_LEN 1
#define RADIO_LEN PREAMBLE_LEN + SYNCWORD_LEN + LEN_ID_LEN + PACKET_LEN + CRC16_LEN + POSTAMBLE_LEN
//^^radio_len = preamble + sync word + length indicator + data + crc + postamble

static int sockfd;

uint16_t crc16(uint8_t * pData, int length) {
    uint8_t i;
    uint16_t wCrc = 0xffff;
    while (length--) {
        wCrc ^= *(unsigned char *)pData++ << 8;
        for (i=0; i < 8; i++)
            wCrc = wCrc & 0x8000 ? (wCrc << 1) ^ 0x1021 : wCrc << 1;
    }
    return wCrc & 0xffff;
}

static int sdr_gnuradio_tx(int fd, const void * data, size_t len) {
    //apply framing according to UHF user manual protocol
    uint8_t * crc_command = os_malloc(LEN_ID_LEN + len);
    crc_command[0] = (uint8_t)len;

    for(size_t i = 0; i < len; i++) {
        crc_command[LEN_ID_LEN+i] = ((uint8_t *)data)[i];
    }

    uint16_t crc_res = crc16(crc_command, LEN_ID_LEN + len);

    uint8_t radio_command[RADIO_LEN] = {0};
    for(int i = 0; i < PREAMBLE_LEN; i++){
        radio_command[i] = PREAMBLE_B;
    }

    for(int i = POSTAMBLE_LEN; i > 0; i--){
        radio_command[RADIO_LEN - i] = PREAMBLE_B;
    }

    radio_command[PREAMBLE_LEN] = SYNCWORD;
    radio_command[PREAMBLE_LEN + SYNCWORD_LEN] = len;
    for(size_t i = 0; i < len; i++) {
        radio_command[PREAMBLE_LEN + SYNCWORD_LEN + LEN_ID_LEN + i] = ((uint8_t *)data)[i];
    }

    radio_command[PREAMBLE_LEN + SYNCWORD_LEN + LEN_ID_LEN + len] = ((uint16_t)crc_res >> 8) & 0xFF;
    radio_command[PREAMBLE_LEN + SYNCWORD_LEN + LEN_ID_LEN + len + 1] = ((uint16_t)crc_res >> 0) & 0xFF;
    
    //send to radio via tcp
#if 1
    FILE *fptr = fopen("output2.bin","w");
    fwrite(radio_command, sizeof(uint8_t), RADIO_LEN, fptr);
    fclose(fptr);
    int status = system("cat output2.bin | nc -w 1 127.0.0.1 1235");
    if(status == -1){
        printf("System call failed\n");
        exit(1);
    }
#endif
    os_free(crc_command);

    return CSP_ERR_NONE;
}

typedef struct gnuradio_context {
    int mtu;
	int rxfd;
	sdr_rx_callback_t rx_callback;
	void *user_data;
} gnuradio_context_t;

static void * gnuradio_rx_thread(void * arg) {
	gnuradio_context_t *ctx = arg;
	uint8_t *data = os_malloc(ctx->mtu);

	while (1) {
		int length = read(ctx->rxfd, data, ctx->mtu);
        if (length == 0) {
			printf("%s: connection closed\n", __FUNCTION__);
            return NULL;
        }
		if (length == -1) {
			printf("%s: read() failed: %d", __FUNCTION__, errno);
            return NULL;
		}
        if (length < ctx->mtu) {
			printf("%s: short read %d<%d", __FUNCTION__, length, ctx->mtu);
		}
        ctx->rx_callback(ctx->user_data, data, length, NULL);
	}
	return NULL;
}

//Inits tcp tx and rx
//starts rx thread
static int gnuradio_tcp_open(const char *host, uint16_t port) {
    struct sockaddr_in servaddr;
   
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("TCP socket w/ gnuradio creation failed: %d\n", errno);
        return -1;
    }
    else
        printf("TCP socket w/ gnuradio successfully created..\n");
    memset(&servaddr, 0, sizeof(servaddr));
   
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(host);
    servaddr.sin_port = htons(port);
   
    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection to port %d failed: %d\n", port, errno);
        return -2;
    }
    else
        printf("Connected to the gnuradio TCP server..\n");

    return sockfd;
}

int sdr_gnuradio_init(sdr_interface_data_t *ifdata) {
    int fd;

	gnuradio_context_t *ctx = os_malloc(sizeof(gnuradio_context_t));
	if (ctx == NULL) {
		printf("%s: Error allocating context\n", __FUNCTION__);
		return -1;
	}

    int rxfd = gnuradio_tcp_open("localhost", 4321);

    ctx->mtu = ifdata->sdr_conf->mtu;
	ctx->rxfd = fd;
	ctx->rx_callback = sdr_rx_isr;
	ctx->user_data = ifdata;

    if (rx_callback) {
		if (os_thread_create(gnuradio_rx_thread, "gnuradio_rx", 0, ctx, 0, 0)) {
			printf("%s: os_thread_create() failed to create RX thread\n", __FUNCTION__);
			os_free(ctx);
			close(rxfd);
			return -2;
		}
	}

    int txfd = gnuradio_tcp_open("localhost", 1235);
    ifdata->fd = txfd;
    ifdata->tx_func = (sdr_tx_t) sdr_gnuradio_tx;

    return 0;
}

#endif /* SDR_GNURADIO */
