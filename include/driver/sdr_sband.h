#ifndef SDR_SBAND_H_
#define SDR_SBAND_H_

#include <osal.h>

#ifdef OS_POSIX
#include <stdbool.h>

// These don't exist (and aren't needed) on Linux
bool sband_enter_conf_mode(void);
bool sband_enter_sync_mode(void);
bool sband_enter_data_mode(void);

void sband_sync(void);

int sband_transmit_ready(void);

bool sband_buffer_count(uint16_t *cnt);

#else
#include <sband.h>
#endif // OS_POSIX

/* Send an S-Band Sync word every sync interval bytes */
#define SBAND_SYNC_INTERVAL 8*1024

#define SBAND_FIFO_DEPTH 20*1024

/* The manual says the radio drains in about 41msec, or about 512bytes/msec */
#define SBAND_DRAIN_RATE 512

typedef struct sdr_sband_conf {
    uint32_t bytes_until_sync;
    uint16_t state;
    uint16_t fifo_count;
    uint8_t  fillx;
    uint8_t  drainx;
    uint16_t fill_cnt[16];
    uint16_t drain_cnt[16];
} sdr_sband_conf_t;

struct sdr_interface_data;

int sdr_sband_driver_init(struct sdr_interface_data *ifdata);

int sdr_sband_tx(struct sdr_interface_data *ifdata, uint8_t *data, uint16_t len);

void sdr_sband_tx_start(struct sdr_interface_data *ifdata);
void sdr_sband_tx_stop(struct sdr_interface_data *ifdata);

#endif /* SDR_SBAND_H_ */

