#include <stdlib.h>
#include <string.h>
#include "sdr_driver.h"
//#include "fec.h"
#include "mac_handler.h"
#include "osal.h"

void sdr_loopback_open(sdr_interface_data_t *ifdata);

static int sdr_driver_init(sdr_interface_data_t *ifdata, const char *ifname) {
    int rc;

    if (strcmp(ifname, SDR_IF_LOOPBACK_NAME) == 0) {
        sdr_loopback_open(ifdata);
    }
    else if (strcmp(ifname, SDR_IF_UHF_NAME) == 0) {
        /* For UHF we can receive using either gnuradio or uart */
#ifdef SDR_GNURADIO
        if ((rc = sdr_gnuradio_driver_init(ifdata))) {
            return rc;
        }
#else
        if ((rc = sdr_uart_driver_init(ifdata))) {
            return rc;
        }
#endif
    }
    else if (strcmp(ifname, SDR_IF_SBAND_NAME) == 0) {
        /* For S-Band we receive on Linux and transmit on FreeRTOS */
#ifdef OS_POSIX
        if ((rc = sdr_gnuradio_driver_init(ifdata))) {
            return rc;
        }
#else
        if ((rc = sdr_sband_driver_init(ifdata))) {
            return rc;
        }
#endif
    }

    ifdata->rx_queue = os_queue_create(2, ifdata->mtu);
    error_correction_scheme_t correction_scheme;
    if (ifdata->sdr_conf->use_fec) {
        correction_scheme = CCSDS_CONVOLUTIONAL_CODING_R_1_2;
    } else {
        correction_scheme = NO_FEC;
    }
    ifdata->mac = mac_create(RF_MODE_3, correction_scheme);

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

