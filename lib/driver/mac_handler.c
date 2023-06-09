#include <string.h>
//#include "sdr_driver.h"
#include "mac_handler.h"
#include "osal.h"

int mpdu_to_buffer(mac_t *my_mac, const uint8_t *mpdu, uint8_t **data, int mtu) {
    packet_processing_status_t rc = process_packet(my_mac, mpdu, mtu);

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

// Returns 0 if none exist, else returns mpdu size
int get_next_mpdu(mac_t *my_mac, void **buf) {
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

int get_mtu_length() {
    return raw_mpdu_length();
}
