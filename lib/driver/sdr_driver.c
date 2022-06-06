#include "sdr_driver.h"
#include "fec.h"
#include "osal.h"

/* From the EnduroSat manual, these delays assume the UART speed is 19.2KBaud */
static int sdr_uhf_baud_rate_delay[] = {
    [SDR_UHF_1200_BAUD] = 920,
    [SDR_UHF_2400_BAUD] = 460,
    [SDR_UHF_4800_BAUD] = 240,
    [SDR_UHF_9600_BAUD] = 120,
    [SDR_UHF_19200_BAUD] = 60
};

int sdr_uhf_tx(sdr_interface_data_t *ifdata, uint8_t *data, uint16_t len) {
    sdr_uhf_conf_t *conf = ifdata->sdr_conf;

    if (fec_data_to_mpdu(ifdata->mac_data, data, len)) {
        uint8_t *buf;
        int delay_time = sdr_uhf_baud_rate_delay[conf->uhf_baudrate];
        size_t mtu = (size_t)fec_get_next_mpdu(ifdata->mac_data, (void **)&buf);
        while (mtu != 0) {
            (ifdata->tx_func)(ifdata->fd, buf, mtu);
            mtu = fec_get_next_mpdu(ifdata->mac_data, (void **)&buf);
            os_sleep_ms(delay_time);
        }
    }

    return 0;
}

void sdr_rx_isr(void *cb_data, void *buf, size_t len, void *pxTaskWoken) {
    sdr_interface_data_t *ifdata = (sdr_interface_data_t *)cb_data;
    sdr_uhf_conf_t *sdr_conf = ifdata->sdr_conf;

    uint8_t *ptr = buf;
    for (size_t i=0; i<len; i++) {
        ifdata->rx_mpdu[ifdata->rx_mpdu_index] = ptr[i];
        ifdata->rx_mpdu_index++;
        if (ifdata->rx_mpdu_index >= sdr_conf->mtu) {
            ifdata->rx_mpdu_index = 0;
            // This is an isr, if this fails there's nothing that can be done
            os_queue_enqueue(ifdata->rx_queue, ifdata->rx_mpdu);
        }
    }
}

os_task_return_t sdr_rx_task(void *param) {
    sdr_interface_data_t *ifdata = (sdr_interface_data_t *)param;
    sdr_uhf_conf_t *sdr_conf = ifdata->sdr_conf;

    uint8_t *mpdu = os_malloc(sdr_conf->mtu);
    uint8_t *data = 0;

    while (1) {
        if (os_queue_dequeue(ifdata->rx_queue, mpdu) != true) {
            continue;
        }
        
        int plen = fec_mpdu_to_data(ifdata->mac_data, mpdu, &data, sdr_conf->mtu);
        if (plen) {
            sdr_conf->rx_callback(ifdata, data, plen);
            os_free(data);
        }
    }
}