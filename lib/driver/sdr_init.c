#include <stdlib.h>
#include <string.h>
#include "sdr_driver.h"
#include "fec.h"
#include "osal.h"

void sdr_loopback_open(sdr_interface_data_t *ifdata);

static int sdr_uhf_driver_init(sdr_interface_data_t *ifdata, const char *ifname) {
    sdr_uhf_conf_t *sdr_conf = ifdata->sdr_conf;
    int rc;

    if (strcmp(ifname, SDR_IF_LOOPBACK_NAME) == 0) {
        sdr_loopback_open(ifdata);
    }
    else {
        if ((rc = sdr_uart_driver_init(ifdata))) {
            return rc;
        }
    }

    ifdata->rx_queue = os_queue_create(2, sdr_conf->mtu);
    ifdata->mac_data = fec_create(RF_MODE_3, NO_FEC);
    ifdata->rx_mpdu_index = 0;
    ifdata->rx_mpdu = os_malloc(sdr_conf->mtu);

    rc = os_task_create(sdr_rx_task, "sdr_rx", OS_RX_TASK_STACK_SIZE, (void *)ifdata, 0, NULL);

    return rc;
}

sdr_interface_data_t *sdr_uhf_interface_init(const sdr_uhf_conf_t *conf, const char *ifname) {
    sdr_uhf_conf_t *sdr_conf = os_malloc(sizeof(sdr_uhf_conf_t));
    sdr_interface_data_t *ifdata = os_malloc(sizeof(sdr_interface_data_t));
    if (!sdr_conf || !ifdata) {
        os_free(sdr_conf);
        os_free(ifdata);
        return NULL;
    }

    memcpy(sdr_conf, conf, sizeof(sdr_uhf_conf_t));
    memset(ifdata, 0, sizeof(sdr_interface_data_t));
    ifdata->sdr_conf = sdr_conf;
    sdr_conf->if_data = ifdata;

    int rc = sdr_uhf_driver_init(ifdata, ifname);
    if (rc) {
        os_free(sdr_conf);
        os_free(ifdata);
        return NULL;
    }

    return ifdata;
}

