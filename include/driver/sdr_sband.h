#ifndef SDR_SBAND_H_
#define SDR_SBAND_H_

#include <stdbool.h>
#include <osal.h>

#ifdef OS_POSIX

// @TODO: include some constants that are already defined in sTransmitter.h under ex2_hal
#define S_RATE_FULL 0
#define S_RATE_HALF 1
#define S_RATE_QUARTER 2

// @TODO: include some constants that are already defined in sband.h under ex2_hal

/* Send an S-Band Sync word every sync interval bytes */
#define SBAND_SYNC_INTERVAL 8 * 1024

/* The depth of our TX FIFO */
#define SBAND_FIFO_DEPTH 20 * 1024
#define SBAND_FIFO_READY_COUNT 2561

//#include <stdbool.h>

// These don't exist (and aren't needed) on Linux
int sband_get_rate(void);

bool sband_enter_conf_mode(void);
bool sband_enter_sync_mode(void);
bool sband_enter_data_mode(void);

void sband_sync(void);

int sband_transmit_ready(void);

bool sband_buffer_count(uint16_t *cnt);

#else
#include <sband.h>
#endif // OS_POSIX

/* The manual says the radio drains in about 41msec, or about 512bytes/msec
 * at full data rate. Half data rate drains in 82msec, etc.
 */
#define SBAND_DRAIN_RATE 512

typedef struct sdr_sband_conf {
    uint32_t bytes_until_sync;
    uint8_t state;
    uint8_t ratex;        // transmit rate multiplier: full, half, or quarter
    uint16_t fifo_count;
    uint16_t too_slow;   // Incremented when we are filling too slowly
    uint16_t too_fast;   // Incremented when we are draining too slowly 
} sdr_sband_conf_t;

struct sdr_interface_data;

int sdr_sband_driver_init(struct sdr_interface_data *ifdata);

int sdr_sband_tx(struct sdr_interface_data *ifdata, uint8_t *data, uint16_t len);

void sdr_sband_tx_start(struct sdr_interface_data *ifdata);
void sdr_sband_tx_stop(struct sdr_interface_data *ifdata);

#endif /* SDR_SBAND_H_ */

