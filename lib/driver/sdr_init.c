#include <stdlib.h>
#include <string.h>
#include "sdr_driver.h"
#include "fec.h"
#include "osal.h"

sdr_uhf_conf_t *sdr_uhf_conf = 0;

void sdr_loopback_open(sdr_interface_data_t *ifdata);
extern os_task_func_t sdr_rx_task;

static int sdr_uhf_driver_init(sdr_uhf_conf_t *sdr_conf, const char *ifname) {
    sdr_interface_data_t *ifdata = sdr_conf->if_data;
    int rc;

    if (strcmp(ifname, SDR_IF_LOOPBACK_NAME) == 0) {
        sdr_loopback_open(ifdata);
    }
    else {
        if ((rc = sdr_uart_driver_init(sdr_conf))) {
            return rc;
        }
    }

    ifdata->rx_queue = os_queue_create(2, sdr_uhf_conf->mtu);
    ifdata->mac_data = fec_create(RF_MODE_3, NO_FEC);
    ifdata->rx_mpdu_index = 0;
    ifdata->rx_mpdu = os_malloc(sdr_uhf_conf->mtu);

    rc = os_task_create(sdr_rx_task, "sdr_rx", OS_RX_TASK_STACK_SIZE, (void *)sdr_conf, 0, NULL);

    return rc;
}

int sdr_uhf_interface_init(const sdr_uhf_conf_t *conf, const char *ifname) {
    sdr_uhf_conf = os_malloc(sizeof(sdr_uhf_conf_t));
    sdr_interface_data_t *ifdata = os_malloc(sizeof(sdr_interface_data_t));
    if (!sdr_uhf_conf || !ifdata) {
        os_free(sdr_uhf_conf);
        os_free(ifdata);
        sdr_uhf_conf = 0;
        return SDR_ERR_NOMEM;
    }

    memcpy(sdr_uhf_conf, conf, sizeof(sdr_uhf_conf_t));
    memset(ifdata, 0, sizeof(sdr_interface_data_t));
    sdr_uhf_conf->if_data = ifdata;

    int rc = sdr_uhf_driver_init(sdr_uhf_conf, ifname);
    if (rc) {
        os_free(sdr_uhf_conf);
        sdr_uhf_conf = 0;
        os_free(ifdata);
    }

    return rc;
}

