/*
 * sband_driver.c
 *
 *  Created on: Aug. 1, 2022
 *      Author: Robert Taylor
 */

#include <sdr_sband.h>
#include "sdr_driver.h"
#include "fec.h"

#define SBAND_IDLE 0
#define SBAND_FIRST_FILL 1
#define SBAND_FILL 2
#define SBAND_DRAIN 3

void sdr_sband_tx_start(sdr_interface_data_t *ifdata) {
    sdr_sband_conf_t *sband_conf = &(ifdata->sdr_conf->sband_conf);

    /* The S-band transmit rate is converted to a bandwidth multiplier.
     * Specifically, full Tx rate is 512 bytes/msec, and given the multiplier 1.
     * Half rate is 256 bytes/msec, so the multiplier is 2 (i.e. divide by 2).
     */
    int rate = sband_get_rate();
    switch(rate) {
    case S_RATE_FULL:
        sband_conf->ratex = 1;
        break;
    case S_RATE_HALF:
        sband_conf->ratex = 2;
        break;
    case S_RATE_QUARTER:
        sband_conf->ratex = 4;
        break;
    default:
        /* Of course, this shouldn't happen but it did :-(
         * The error should have been logged in sband_get_rate() so we'll just
         * give it a bit of extra time here (although it's likely broken).
         */
        sband_conf->ratex = 5;
        break;
    }
    sband_conf->state = SBAND_FIRST_FILL;
    sband_enter_sync_mode();
}

/* S-band drain tunables. The manual says that the 20KB FIFO should drain in
 * 41msec at full data rate. We have observed the transmitter become ready in
 * approximately 120msec at 1/4 data rate. The retry count and the sleep time
 * per retry can be adjusted so we never overflow or underflow the FIFO.
 */
#define SBAND_DRAIN_RATE 512
#define SBAND_DRAIN_MSEC 2

inline static void sband_drain_fifo(sdr_sband_conf_t *sband_conf) {
    int retries = 0;
    int drain_rate = SBAND_DRAIN_RATE/sband_conf->ratex;
    int drain_count = (sband_conf->fifo_count/drain_rate)/SBAND_DRAIN_MSEC;
    while (!sband_transmit_ready() && retries<drain_count) {
        os_sleep_ms(SBAND_DRAIN_MSEC);
        ++retries;
    }
    if (retries == 0) {
        /* s-band transmit was immediately ready. That means we're filling too
         * slowly.
         */
        sband_conf->too_slow++;
    }
    else if (retries >= drain_count) {
        /* s-band transmit never signalled ready. That means we're not waiting
         * long enough or we're filling too much too fast.
         */
        sband_conf->too_fast++;
    }

    sband_conf->fifo_count = SBAND_FIFO_READY_COUNT;
}

void sdr_sband_tx_stop(sdr_interface_data_t *ifdata) {
    sdr_sband_conf_t *sband_conf = &ifdata->sdr_conf->sband_conf;

    sband_conf->state = SBAND_IDLE;

    // We should let the FIFO drain before turning off the radio
    sband_drain_fifo(sband_conf);
    // The transmitter signals ready when there is still 2KB in the FIFO.
    os_sleep_ms(20);

    sband_enter_conf_mode();
}

int sdr_sband_tx(sdr_interface_data_t *ifdata, uint8_t *data, uint16_t len) {
    sdr_sband_conf_t *sband_conf = &ifdata->sdr_conf->sband_conf;

    if (fec_data_to_mpdu(ifdata->mac_data, data, len)) {
        uint8_t *buf;
        size_t mtu = (size_t)fec_get_next_mpdu(ifdata->mac_data, (void **)&buf);
        while (mtu != 0) {
            if (sband_conf->bytes_until_sync == 0) {
                sband_sync();
                sband_conf->bytes_until_sync = SBAND_SYNC_INTERVAL;
            }

            (ifdata->tx_func)(ifdata->fd, buf, mtu);

            sband_conf->bytes_until_sync -= mtu;
            sband_conf->fifo_count += mtu;

            if (sband_conf->fifo_count >= (SBAND_FIFO_DEPTH - 1024)) {
                if (sband_conf->state == SBAND_FIRST_FILL) {
                    sband_enter_data_mode();
                }

                sband_drain_fifo(sband_conf);

                sband_conf->state = SBAND_FILL;
            }
            mtu = fec_get_next_mpdu(ifdata->mac_data, (void **)&buf);
        }
    }

    return 0;
}


