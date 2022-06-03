#include "MACWrapper.h"
#include "sdr_driver.h"
#include "osal.h"

#ifdef __cplusplus
extern "C" {
#endif

bool fec_data_to_mpdu(mac_t *my_mac, uint8_t *packet, uint16_t len) {
    return receive_packet(my_mac, packet, len);
}

int fec_mpdu_to_data(mac_t *my_mac, const uint8_t *mpdu, uint8_t **data, int mtu) {
    uhf_packet_processing_status_t rc = process_uhf_packet(my_mac, mpdu, mtu);

    if (rc == PACKET_READY) {
        const uint8_t *raw_packet = get_raw_packet_buffer(my_mac);

        int len = get_raw_packet_length(my_mac);
        *data = os_malloc(len);
        if (*data) {
            memcpy(*data, raw_packet, len);
        }
        return len;
    }
    return 0;
}

mac_t *fec_create(rf_mode_number_t rfmode, error_correction_scheme_t error_correction_scheme) {
    return mac_create(rfmode, error_correction_scheme);
}

// Returns 0 if none exist, else returns mpdu size
int fec_get_next_mpdu(mac_t *my_mac, void **buf) {
    static uint32_t index = 0;
    const uint32_t length = mpdu_payloads_buffer_length(my_mac);
    if (index >= length) {
        index = 0;
        return 0;
    }
    const uint8_t *mpdusBuffer = mpdu_payloads_buffer(my_mac);
    const uint32_t mtu = raw_mpdu_length();
    *buf = (uint8_t *)(mpdusBuffer) + index;
    index += mtu;
    return mtu;
}

int fec_get_mtu() {
    return raw_mpdu_length();
}

#ifdef __cplusplus
}
#endif
