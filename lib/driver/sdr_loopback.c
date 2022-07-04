#include <stdbool.h>
#include "sdr_driver.h"
#include "osal.h"

static sdr_interface_data_t *loop_ifdata;

static int sdr_loopback_tx(int fd, const void *buf, size_t len) {
    if (os_queue_enqueue(loop_ifdata->rx_queue, (const uint8_t *)buf) != true) {
        return SDR_ERR_TIMEOUT;
    }

    return SDR_ERR_NONE;
}

void sdr_loopback_open(sdr_interface_data_t *ifdata) {
    loop_ifdata = ifdata;
    ifdata->tx_func = sdr_loopback_tx;
}
