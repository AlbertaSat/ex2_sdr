#include <stdlib.h>
#include <string.h>
#include "sdr_driver.h"
#include "fec.h"
#include "osal.h"

void sdr_loopback_open(sdr_interface_data_t *ifdata);

static int sdr_driver_init(sdr_interface_data_t *ifdata, const char *ifname) {
    int rc;

    if (strcmp(ifname, SDR_IF_LOOPBACK_NAME) == 0) {
        sdr_loopback_open(ifdata);
    }
    else if (strcmp(ifname, SDR_IF_UHF_NAME) == 0) {
        if ((rc = sdr_uart_driver_init(ifdata))) {
            return rc;
        }
    }
    else if (strcmp(ifname, SDR_IF_SBAND_NAME) == 0) {
        if ((rc = sdr_sband_driver_init(ifdata))) {
            return rc;
        }
    }

    ifdata->rx_queue = os_queue_create(2, ifdata->mtu);
    ifdata->mac_data = fec_create(RF_MODE_3, NO_FEC);
    ifdata->rx_mpdu_index = 0;
    ifdata->rx_mpdu = os_malloc(ifdata->mtu);

    rc = os_task_create(sdr_rx_task, "sdr_rx", OS_RX_TASK_STACK_SIZE, (void *)ifdata, 0, NULL);

    return rc;
}

sdr_interface_data_t *sdr_interface_init(const sdr_conf_t *conf, const char *ifname) {
    sdr_conf_t *sdr_conf = os_malloc(sizeof(sdr_conf_t));
    sdr_interface_data_t *ifdata = os_malloc(sizeof(sdr_interface_data_t));
    if (!sdr_conf || !ifdata) {
        os_free(sdr_conf);
        os_free(ifdata);
        return NULL;
    }

    memcpy(sdr_conf, conf, sizeof(*sdr_conf));
    memset(ifdata, 0, sizeof(*ifdata));

    ifdata->mtu = SDR_UHF_MAX_MTU;
    ifdata->sdr_conf = sdr_conf;

    int rc = sdr_driver_init(ifdata, ifname);
    if (rc) {
        os_free(sdr_conf);
        os_free(ifdata);
        return NULL;
    }

    return ifdata;
}

