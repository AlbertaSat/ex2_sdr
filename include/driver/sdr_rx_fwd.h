#ifndef SDR_RX_FWD_H_
#define SDR_RX_FWD_H_

#include <pthread.h>
#include <sdr_driver.h>

/* This include file serves a dual purpose: to provide the sdr_rx_fwd_init()
 * prototype for the C-python binding library; and to provide the internal
 * forwarding API to other forwarders.
 */

// The TCP port the forwarder connects to
#define SDR_RX_FWD_PORT 40000

// The forwarder is called when the transmission has been received.
typedef void (*rx_done_t)(void *arg);

struct rx_fwd_ctx {
  int blksz;
  int buflen;         // size of the buffer
  int offset;         // amount of buffer that has been used
  unsigned char *buf; // holds the entire transmission
  rx_done_t rx_done;
  pthread_mutex_t wait_lock;
  pthread_cond_t wait_cond;
};

void sdr_rx_fwd_init(sdr_interface_data_t *ifdata, rx_done_t rx_forwarder);

// Called by the forwarder to unblock sdr_rx_fwd_init
void sdr_rx_signal_waiters(struct rx_fwd_ctx *ctx);

#endif // SDR_RX_FWD_H_
