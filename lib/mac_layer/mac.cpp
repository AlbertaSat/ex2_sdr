/*!
 * @file mac.cpp
 * @author Steven Knudsen
 * @date June 18, 2019
 *
 * @details
 *
 * @copyright Xiphos Systems Corp. 2019
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#include "mac.hpp"

#include <cmath>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

#include "HL_sci.h"
#include "HL_reg_sci.h"
#include "queue.h"

#ifdef __cplusplus
}
#endif

#include "golay.h"
#include "mpdu.hpp"

namespace ex2 {
  namespace sdr {


    MAC* MAC::m_instance = 0;

    MAC*
    MAC::instance (RF_Mode::RF_ModeNumber rfModeNumber,
      ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme)
    {
      if (m_instance == 0)
      {
        m_instance = new MAC (rfModeNumber, errorCorrectionScheme);
      }
      return m_instance;
    }

    MAC*
    MAC::instance() {
      if (m_instance == 0)
        throw std::exception();
      return m_instance;
    }


    MAC::MAC (RF_Mode::RF_ModeNumber rfModeNumber,
      ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme) :
            m_rfModeNumber(rfModeNumber)
    {
      // TODO these are all updated when the error correction scheme changes
      m_errorCorrection = new ErrorCorrection(errorCorrectionScheme);
      m_numCodewordFragments = m_errorCorrection->getCodewordLen() / MPDUHeader::MACHeaderLength();
      if (m_errorCorrection->getCodewordLen() % MPDUHeader::MACHeaderLength() != 0)
        m_numCodewordFragments++;
      m_messageLength = m_errorCorrection->getMessageLen();

      xSendQueue = xQueueCreate( QUEUE_LENGTH, sizeof( uint32_t ) );
      xRecvQueue = xQueueCreate( QUEUE_LENGTH, sizeof( uint32_t ) );

      if( xSendQueue != NULL )
      {
        /* Start the two tasks as described in the comments at the top of this
        file. */
        xTaskCreate( queueSendTask,        /* The function that implements the task. */
          "SendToUHF",                     /* The text name assigned to the task - for debug only as it is not used by the kernel. */
          configMINIMAL_STACK_SIZE,        /* The size of the stack to allocate to the task. */
          NULL,                            /* The parameter passed to the task - not used in this simple case. */
          mainQUEUE_RECEIVE_TASK_PRIORITY, /* The priority assigned to the task. */
          NULL );                          /* The task handle is not required, so NULL is passed. */
      }

      if( xRecvQueue != NULL )
      {
        /* Start the two tasks as described in the comments at the top of this
        file. */
        xTaskCreate( queueSendTask,        /* The function that implements the task. */
          "RecvFromUHF",                   /* The text name assigned to the task - for debug only as it is not used by the kernel. */
          configMINIMAL_STACK_SIZE,        /* The size of the stack to allocate to the task. */
          NULL,                            /* The parameter passed to the task - not used in this simple case. */
          mainQUEUE_SEND_TASK_PRIORITY,    /* The priority assigned to the task. */
          NULL );                          /* The task handle is not required, so NULL is passed. */
      }
    }

    MAC::~MAC () {
      if (m_errorCorrection != NULL) {
        ~m_errorCorrection;
      }
    }

    /*!
     * @brief The task that receives CSP packets via a queue from the CSP
     * server and decomposes them into Transparent Mode packets to be
     * sent to the UHF Radio for transmission.
     *
     * @details
     * @todo refactor this?
     *
     * @param taskParameters
     */
    void
    MAC::queueReceiveTask( void *taskParameters ) {
      std::vector<uint8_t> messageBuffer;
      for (;;) {

        if (queue not empty) {

          // Get CSP packet

          // Parse the packet into messages, encode, and send to UHF radio
          uint16_t numMessages = cspLength / m_messageLength;
          if (cspLength % m_messageLength != 0)
            numMessages++;
          messageBuffer.resize(m_messageLength,0);
          uint16_t cspBytesRemaining = cspLength;
          for (uint16_t i = 0; i < numMessages; i++) {
            uint16_t cspIndex = 0;
            uint16_t bytesToCopy = m_messageLength;
            if (cspBytesRemaining < m_messageLength) bytesToCopy = cspBytesRemaining;
            for (uint16_t j = 0; j < bytesToCopy; j++) {
              messageBuffer[j] = cspBuffer[cspIndex++]
            }

            // TODO update MPDU parameters, encode message, make MPDU, then send via UART to radio

          }

        } // queue not empty
      }
    }

    /*!
     * @brief The task that receives UHF radio packets, assembles them into CSP
     * packets, and sends them via a queue to the CSP server.
     *
     * @details Packets come from the UHF radio via UART. They can be ESTTC or
     * transparent mode packets. ESTTC packets should be valid since they are
     * checked using the CRC16 defined for all but transparent mode packets, but
     * some simple checks should be done before passing to the application layer
     * (such as length consistency).
     *
     * Transparent mode packets are passed along regardless of the CRC16 check
     * since we employ FEC; packets may have errors and still be correctable.
     * Transparent mode packets are always 128 bytes. The MAC header is
     * protected by Golay encoding and the payload by the selected FEC scheme.
     * Thus, there is a check for length (128 bytes) and MAC header decoding
     * success.
     *
     * @todo refactor this?
     *
     * @param taskParameters
     */
    void
    MAC::queueSendTask( void *taskParameters ) {

      // TODO There needs to be a way to safely update the FEC scheme and/or
      // the RF mode while this task is running and processing packets. Would
      // be best done when we know no UART packets are arriving...

      // TODO Let's assume no FEC scheme and default RF Mode for now.
      // TODO Should these be task parameters?
      ErrorCorrection::ErrorCorrectionScheme ecScheme = ErrorCorrection::ErrorCorrectionScheme::NO_FEC;
      RF_Mode::RF_ModeNumber rfMode = RF_Mode::RF_ModeNumber::RF_MODE_3;

      // TODO use the indices and packet len to check progress and for errors?
      uint8_t cwFragIndex = 0;         // Nothing receive yet, so 0
      uint16_t userPacketLen = 0;      // Nothing receive yet, so 0
      uint8_t userPacketFragIndex = 0; // Nothing receive yet, so 0

      std::vector<uint8_t> uartPacket; //
      std::vector<uint8_t> cspData;
      std::vector<uint8_t> codeword;

      // Initialize things needed to recieve transparent mode packets from the UHF radio
      bool goodUHFPacket = true;
      uartPacket.resize(0);
      cspData.resize(0);
      codeword.resize(0);

      for( ;; )
      {
        /*!
         * Check sci for bytes. If we get some, we have to assume that the radio
         * has passed a whole packet to us.
         *
         * In transparent mode the packet is supposed to be 128 bytes long. The
         * first byte is Data Field 1, the packet length. If it is not 128, then
         * either we have an bit error in that field, or maybe a AX.25 or ESTTC
         * packet. All these possibilities must be checked.
         */
        if(sciIsRxReady(sciREG2) != 0) {

          // A new packet arrived; get all the bytes. First byte is Data Field 1
          uartPacket.resize(0);
          uint8_t data = sciReceiveByte(sciREG2);
          uartPacket.push_back(data);

          // Get the rest of the bytes
          while(sciIsRxReady(sciREG2)) {
            data = sciReceiveByte(sciREG2);
            uartPacket.push_back(data);
          }

          // Should be a complete packet in uartPacket. Get the length and check
          // if ESTTC, AX.25, or transparent mode packet.
          uint8_t packetLength = uartPacket[0];

          // TODO Not sure we need to check for ESTTC or AX.25 packets. Arash and
          // Charles say no, but thew work is partially done, so for now keep.
          if (isESTTCPacket(uartPacket) || isAX25Packet(uartPacket)) {
            // Send up to the CSP server
            // TODO implement this!

          }
          else {
            // Might be a transparent mode packet. Let's try to make an MPDU
            try {
              // make an MPDU, which does some checking of the data validity
              MPDU recdMPDU(uartPacket);

              // Valid MPDU, so add payload to current codeword
              std::vector<uint8_t> cw = recdMPDU.getCodeword();
              codeword.insert(codeword.end(), cw.begin(), cw.end());
              cwFragIndex++;

              if (codeword.size() > m_errorCorrection->getCodewordLen() ||
                  (cwFragIndex > m_numCodewordFragments) ||
                  (cwFragIndex != recdMPDU.getMpduHeader()->getCodewordFragmentIndex())) {
                // TODO this is an error. We need to dump the current codeword
                // and wait for a new UART packet codeword that starts at
                // index = 0
              }
              else {
                if (codeword.size() == m_errorCorrection->getCodewordLen()) {

                  // We have the full codeword, so decode
                  std::vector<uint8_t> message = m_errorCorrection->decode(codeword);

                  if (message.size() > 0) {
                    cspData.insert(cspData.end(), message.begin(), message.end());

                    if (cspData.size() >= recdMPDU.getMpduHeader()->getUserPacketLength()) {
                      // TODO Shouldn't be greater than the packet length in the
                      // header, but maybe we send it anyway?

                      // TODO inspect the cspData to make sure it is an actual
                      // CSP packet. If it is, put on the queue up to the CSP
                      // server

                      // xQueuSend( ... )

                      // Reset the buffers and counters
                      cspData.resize(0);

                    }
                  }
                }
              } // check for valid codeword
              codeword.resize(0);
            }
            catch (MPDUException& e) {
              // Log the error
              //              std::cerr << e.what() << std::endl;
              // TODO throw this error further?
              //              throw e;
              codeword.resize(0);
            }

          }

        }

      }

    } // queueSendTask

    bool
    MAC::isESTTCPacket(std::vector<uint8_t> &packet) {
      bool isPacket = false;
      // First byte should be the Data Field 1, the Data Field 2 length in bytes
      // Thus, the length of packet should be the value of the first byte + 1

      if (packet[0] + 1 == packet.size()) {
        if ((packet[1] == 'E') &&
            (packet[2] == 'S') &&
            (packet[3] == '+')) {
          isPacket = true;
        }
        if ((packet[1] == 'O') &&
            (packet[2] == 'K')) {
          isPacket = true;
        }
        if ((packet[1] == '+') &&
            (packet[2] == 'E') &&
            (packet[3] == 'S')) {
          isPacket = true;
        }
        if ((packet[1] == 'E') &&
            (packet[2] == 'R') &&
            (packet[3] == 'R')) {
          isPacket = true;
        }
      } // packet length is good

      return isPacket;

    } // isESTTCPacket

    bool
    MAC::isAX25Packet(std::vector<uint8_t> &packet) {
      bool isPacket = false;
      // First byte should be the Data Field 1, the Data Field 2 length in bytes
      // Thus, the length of packet should be the value of the first byte + 1

      if (packet[0] + 1 == packet.size()) {
        // The AX.25 frame starts with 9 bytes that have the value 0x7E
        uint8_t i;
        isPacket = true;
        for (i = 1; i <= 9; i++) {
          isPacket = isPacket & (packet[i] == 0x7E);
        }
      } // packet length is good

      return isPacket;

    }

    void processReceivedCSP(csp_packet_t *packet);


  } /* namespace sdr */
} /* namespace ex2 */

