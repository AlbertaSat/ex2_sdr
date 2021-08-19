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

#include "csp_buffer.h"

#ifdef __cplusplus
}
#endif

#include "golay.h"
#include "mpdu.hpp"

namespace ex2 {
  namespace sdr {

//    MACException::MACException(const std::string& message) :
//                       runtime_error(message) { }

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
      // @todo Not sure a C program can handle an exception... remove?
      if (m_instance == 0) {
        throw std::exception();
      }
      return m_instance;
    }

    MAC::MAC (RF_Mode::RF_ModeNumber rfModeNumber,
      ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme) :
                            m_rfModeNumber(rfModeNumber)
    {
      // TODO these are all updated when the error correction scheme changes
      // TODO This instance may well have been created when the MAC system is initialized,
      // which is a process TBD.
      m_errorCorrection = new ErrorCorrection(errorCorrectionScheme);

      // Use the FEC factory to get the current FEC codec
      m_FEC = FEC::makeFECCodec(errorCorrectionScheme);

      // Calculate how many codeword fragments are needed to send one codeword
      m_numCodewordFragments = m_errorCorrection->numCodewordFragments(MPDUHeader::MACPayloadLength());

      if (m_errorCorrection->getCodewordLen() % MPDUHeader::MACHeaderLength() != 0)
        m_numCodewordFragments++;
      m_messageLength = m_errorCorrection->getMessageLen();

      m_codewordFragmentCount = 0;
      m_userPacketFragementCount = 0;
      // @TODO, do we need a member var to keep track of the max packet frag count?
    }

    MAC::~MAC () {
      if (m_errorCorrection != NULL) {
        delete m_errorCorrection;
      }
    }

    //    /*!
    //     * @brief The task that receives UHF radio packets from the CSP server,
    //     * breaks them into chunks that can be processed and fit into transparent
    //     * packets, and passes those in order to the UHF radio.
    //     *
    //     * @details
    //     *
    //     * @todo refactor this?
    //     *
    //     * @param taskParameters
    //     */    void
    //     MAC::queueSendToUHFTask( void *taskParameters ) {
    //       MAC *me = static_cast<MAC *>(taskParameters);
    //       uint16_t currentBuffer = 0;
    //       for (;;) {
    //         // Get current buffer
    //         csp_packet_t * packet = me->m_CSPToUHFBuffers[currentBuffer];
    //
    //         // Get CSP packet
    //
    //         // This task will block provided INCLUDE_vTaskSuspend is set to 1 in
    //         // FreeRTOSConfig.h. No CPU time is used while in the blocked state
    //         xQueueReceive( me->xSendToUHFQueue, packet, portMAX_DELAY );
    //
    //         if (packet) {
    //           // Parse the packet into messages, encode, and send to UHF radio
    //
    //           // Determine how many transparent mode packet messages are in the
    //           // CSP packet. Check for a remainder and round up as needed
    //           uint16_t numMessages = packet->length / me->m_messageLength;
    //           if (packet->length % me->m_messageLength != 0)
    //             numMessages++;
    //
    //           std::vector<uint8_t> messageBuffer;
    //           uint16_t cspBytesRemaining = packet->length;
    //           uint32_t i;
    //           uint16_t bytesToCopy = me->m_messageLength;
    //           for (i = 0; i < numMessages; i++) {
    //             if (cspBytesRemaining < me->m_messageLength) {
    //               bytesToCopy = cspBytesRemaining;
    //               // make sure the end of the buffer is zeros
    //               messageBuffer.resize(me->m_messageLength, 0);
    //             }
    //
    //             // copy part of the CSP packet into the messageBuffer
    //             messageBuffer.assign(packet->data + i * me->m_messageLength,
    //               packet->data + i * me->m_messageLength + bytesToCopy);
    //
    //             cspBytesRemaining -= bytesToCopy;
    //             // TODO update MPDU parameters, encode message, make MPDU, then send via UART to radio
    //
    //           } // for all the messages in the CSP packet
    //
    //         } // packet is not null
    //
    //       } // for(ever)
    //     }
    //
    //     /*!
    //      * @brief The task that receives UHF radio packets, assembles them into CSP
    //      * packets, and sends them via a queue to the CSP server.
    //      *
    //      * @details Packets come from the UHF radio via UART. They can be ESTTC or
    //      * transparent mode packets. ESTTC packets should be valid since they are
    //      * checked using the CRC16 defined for all but transparent mode packets, but
    //      * some simple checks should be done before passing to the application layer
    //      * (such as length consistency).
    //      *
    //      * Transparent mode packets are passed along regardless of the CRC16 check
    //      * since we employ FEC; packets may have errors and still be correctable.
    //      * Transparent mode packets are always 128 bytes. The MAC header is
    //      * protected by Golay encoding and the payload by the selected FEC scheme.
    //      * Thus, there is a check for length (128 bytes) and MAC header decoding
    //      * success.
    //      *
    //      * @todo refactor this?
    //      *
    //      * @param taskParameters
    //      */
    //     void
    //     MAC::queueReceiveFromUHFTask( void *taskParameters ) {
    //       MAC *me = static_cast<MAC *>(taskParameters);
    //
    //       // TODO There needs to be a way to safely update the FEC scheme and/or
    //       // the RF mode while this task is running and processing packets. Would
    //       // be best done when we know no UART packets are arriving...
    //
    //       // TODO Let's assume no FEC scheme and default RF Mode for now.
    //       // TODO Should these be task parameters?
    //       ErrorCorrection::ErrorCorrectionScheme ecScheme = ErrorCorrection::ErrorCorrectionScheme::NO_FEC;
    //       RF_Mode::RF_ModeNumber rfMode = RF_Mode::RF_ModeNumber::RF_MODE_3;
    //
    //       // TODO use the indices and packet len to check progress and for errors?
    //       uint8_t cwFragIndex = 0;         // Nothing receive yet, so 0
    //       uint16_t userPacketLen = 0;      // Nothing receive yet, so 0
    //       uint8_t userPacketFragIndex = 0; // Nothing receive yet, so 0
    //
    //       std::vector<uint8_t> uartPacket; //
    //       std::vector<uint8_t> cspData;
    //       std::vector<uint8_t> codeword;
    //
    //       // Initialize things needed to recieve transparent mode packets from the UHF radio
    //       bool goodUHFPacket = true;
    //       uartPacket.resize(0);
    //       cspData.resize(0);
    //       codeword.resize(0);
    //
    //       for( ;; )
    //       {
    //         /*!
    //          * Check sci for bytes. If we get some, we have to assume that the radio
    //          * has passed a whole packet to us.
    //          *
    //          * In transparent mode the packet is supposed to be 128 bytes long. The
    //          * first byte is Data Field 1, the packet length. If it is not 128, then
    //          * either we have an bit error in that field, or maybe a AX.25 or ESTTC
    //          * packet. All these possibilities must be checked.
    //          */
    //         if(sciIsRxReady(sciREG2) != 0) {
    //
    //           // A new packet arrived; get all the bytes. First byte is Data Field 1
    //           uartPacket.resize(0);
    //           uint8_t data = sciReceiveByte(sciREG2);
    //           uartPacket.push_back(data);
    //
    //           // Get the rest of the bytes
    //           while(sciIsRxReady(sciREG2)) {
    //             data = sciReceiveByte(sciREG2);
    //             uartPacket.push_back(data);
    //           }
    //
    //           // Should be a complete packet in uartPacket. Get the length and check
    //           // if ESTTC, AX.25, or transparent mode packet.
    //           uint8_t packetLength = uartPacket[0];
    //
    //           // TODO Not sure we need to check for ESTTC or AX.25 packets. Arash and
    //           // Charles say no, but thew work is partially done, so for now keep.
    //           if (me->isESTTCPacket(uartPacket) || me->isAX25Packet(uartPacket)) {
    //             // Send up to the CSP server
    //             // TODO implement this!
    //
    //           }
    //           else {
    //             // Might be a transparent mode packet. Let's try to make an MPDU
    //             try {
    //               // make an MPDU, which does some checking of the data validity
    //               MPDU recdMPDU(uartPacket);
    //
    //               // Valid MPDU, so add payload to current codeword
    //               std::vector<uint8_t> cw = recdMPDU.getCodeword();
    //               codeword.insert(codeword.end(), cw.begin(), cw.end());
    //               cwFragIndex++;
    //
    //               if (codeword.size() > me->m_errorCorrection->getCodewordLen() ||
    //                   (cwFragIndex > me->m_numCodewordFragments) ||
    //                   (cwFragIndex != recdMPDU.getMpduHeader()->getCodewordFragmentIndex())) {
    //                 // TODO this is an error. We need to dump the current codeword
    //                 // and wait for a new UART packet codeword that starts at
    //                 // index = 0
    //               }
    //               else {
    //                 if (codeword.size() == me->m_errorCorrection->getCodewordLen()) {
    //
    //                   // We have the full codeword, so decode
    //                   std::vector<uint8_t> message = me->m_errorCorrection->decode(codeword);
    //
    //                   if (message.size() > 0) {
    //                     cspData.insert(cspData.end(), message.begin(), message.end());
    //
    //                     if (cspData.size() >= recdMPDU.getMpduHeader()->getUserPacketLength()) {
    //                       // TODO Shouldn't be greater than the packet length in the
    //                       // header, but maybe we send it anyway?
    //
    //                       // TODO inspect the cspData to make sure it is an actual
    //                       // CSP packet. If it is, put on the queue up to the CSP
    //                       // server
    //
    //                       // xQueuSend( ... )
    //
    //                       // Reset the buffers and counters
    //                       cspData.resize(0);
    //
    //                     }
    //                   }
    //                 }
    //               } // check for valid codeword
    //               codeword.resize(0);
    //             }
    //             catch (MPDUException& e) {
    //               // Log the error
    //               //              std::cerr << e.what() << std::endl;
    //               // TODO throw this error further?
    //               //              throw e;
    //               codeword.resize(0);
    //             }
    //
    //           }
    //
    //         }
    //
    //       }
    //
    //     } // queueSendTask

    uint32_t
    MAC::processUHFPacket(const uint8_t *uhfPayload, const uint32_t payloadLength) {

      uint32_t bitErrors = 0;

      // Make an MPDU from the @p uhfPayload
      std::vector<uint8_t> p;
      p.assign(uhfPayload, uhfPayload+payloadLength);
      MPDU mpdu(p);

      // @todo mutex here?

      // Process the MPDU
      if (mpdu.getMpduHeader()->getUserPacketFragmentIndex() == 0 &&
          mpdu.getMpduHeader()->getCodewordFragmentIndex() == 0) {
        // this is the first user packet fragment and the first codeword fragment
        // of that packet fragment, so we are receiving a new CSP packet

        // New CSP packet, empty the buffer
        // @todo rename this buffer?
        m_receiveUHFBuffer.resize(0);
        // New codeword, empty the buffer
        m_codewordBuffer.resize(0);

        // @TODO check that the header matches the FEC we are using?
        // @TODO check it is long enough to have the CSP info needed

        // append the current uhf payload to the codeword buffer
        m_codewordBuffer.insert(m_codewordBuffer.end(), p.begin(), p.end());

        // @todo get the expected number of codeword and packet fragments
        // Should be a static function in ErrorCorrection

        // Init fragment counters
        m_codewordFragmentCount = 1;
        m_userPacketFragementCount = 0;

        // @TODO Update counters and check for counters complete.
        // Do we need only the one packet?
        if (m_codewordFragmentCount == m_numCodewordFragments) {
          PPDU_u8::payload_t decodedPayload;
          bitErrors = m_FEC->decode(mpdu.getCodeword(), 100.0, decodedPayload);

          // @TODO check bitErrors and reject/reset if too many?

        }
//        m_userPacketFragementCount = 0;

      }
      else {

      }

      return bitErrors;
      //
      //       // TODO There needs to be a way to safely update the FEC scheme and/or
      //       // the RF mode while this task is running and processing packets. Would
      //       // be best done when we know no UART packets are arriving...
      //
      //
      //       // TODO use the indices and packet len to check progress and for errors?
      //       uint8_t cwFragIndex = 0;         // Nothing receive yet, so 0
      //       uint16_t userPacketLen = 0;      // Nothing receive yet, so 0
      //       uint8_t userPacketFragIndex = 0; // Nothing receive yet, so 0
      //
      //       std::vector<uint8_t> uartPacket; //
      //       std::vector<uint8_t> cspData;
      //       std::vector<uint8_t> codeword;
      //
      //       // Initialize things needed to recieve transparent mode packets from the UHF radio
      //       bool goodUHFPacket = true;
      //       uartPacket.resize(0);
      //       cspData.resize(0);
      //       codeword.resize(0);
      //
      //       for( ;; )
      //       {
      //         /*!
      //          * Check sci for bytes. If we get some, we have to assume that the radio
      //          * has passed a whole packet to us.
      //          *
      //          * In transparent mode the packet is supposed to be 128 bytes long. The
      //          * first byte is Data Field 1, the packet length. If it is not 128, then
      //          * either we have an bit error in that field, or maybe a AX.25 or ESTTC
      //          * packet. All these possibilities must be checked.
      //          */
      //         if(sciIsRxReady(sciREG2) != 0) {
      //
      //           // A new packet arrived; get all the bytes. First byte is Data Field 1
      //           uartPacket.resize(0);
      //           uint8_t data = sciReceiveByte(sciREG2);
      //           uartPacket.push_back(data);
      //
      //           // Get the rest of the bytes
      //           while(sciIsRxReady(sciREG2)) {
      //             data = sciReceiveByte(sciREG2);
      //             uartPacket.push_back(data);
      //           }
      //
      //           // Should be a complete packet in uartPacket. Get the length and check
      //           // if ESTTC, AX.25, or transparent mode packet.
      //           uint8_t packetLength = uartPacket[0];
      //
      //           // TODO Not sure we need to check for ESTTC or AX.25 packets. Arash and
      //           // Charles say no, but thew work is partially done, so for now keep.
      //           if (me->isESTTCPacket(uartPacket) || me->isAX25Packet(uartPacket)) {
      //             // Send up to the CSP server
      //             // TODO implement this!
      //
      //           }
      //           else {
      //             // Might be a transparent mode packet. Let's try to make an MPDU
      //             try {
      //               // make an MPDU, which does some checking of the data validity
      //               MPDU recdMPDU(uartPacket);
      //
      //               // Valid MPDU, so add payload to current codeword
      //               std::vector<uint8_t> cw = recdMPDU.getCodeword();
      //               codeword.insert(codeword.end(), cw.begin(), cw.end());
      //               cwFragIndex++;
      //
      //               if (codeword.size() > me->m_errorCorrection->getCodewordLen() ||
      //                   (cwFragIndex > me->m_numCodewordFragments) ||
      //                   (cwFragIndex != recdMPDU.getMpduHeader()->getCodewordFragmentIndex())) {
      //                 // TODO this is an error. We need to dump the current codeword
      //                 // and wait for a new UART packet codeword that starts at
      //                 // index = 0
      //               }
      //               else {
      //                 if (codeword.size() == me->m_errorCorrection->getCodewordLen()) {
      //
      //                   // We have the full codeword, so decode
      //                   std::vector<uint8_t> message = me->m_errorCorrection->decode(codeword);
      //
      //                   if (message.size() > 0) {
      //                     cspData.insert(cspData.end(), message.begin(), message.end());
      //
      //                     if (cspData.size() >= recdMPDU.getMpduHeader()->getUserPacketLength()) {
      //                       // TODO Shouldn't be greater than the packet length in the
      //                       // header, but maybe we send it anyway?
      //
      //                       // TODO inspect the cspData to make sure it is an actual
      //                       // CSP packet. If it is, put on the queue up to the CSP
      //                       // server
      //
      //                       // xQueuSend( ... )
      //
      //                       // Reset the buffers and counters
      //                       cspData.resize(0);
      //
      //                     }
      //                   }
      //                 }
      //               } // check for valid codeword
      //               codeword.resize(0);
      //             }
      //             catch (MPDUException& e) {
      //               // Log the error
      //               //              std::cerr << e.what() << std::endl;
      //               // TODO throw this error further?
      //               //              throw e;
      //               codeword.resize(0);
      //             }
      //
      //           }
      //
      //         }
      //
      //       }
      //

    }

    /*!
     * @brief Reset the processing of received UHF data.
     *
     * @details Some kind of error (dropped packet, bad packet, etc.) has
     * happened and the current CSP packet cannot be recovered. Reset the
     * processing.
     */
    void
    MAC::resetUHFProcessing() {

    }

    /*!
     * @brief A complete CSP packet is ready to be sent up to the the CSP Server.
     *
     * @return true if there is a CSP packet, false otherwise
     */
    bool
    MAC::isCSPPacketReady() {

      // @TODO implement function
      return false;

    }

    /*!
     * @brief Accessor
     *
     * @details After the CSP packet has been retrieved, call @p resetUHFProcessing.
     *
     * @return Pointer to the completed CSP packet.
     */
    csp_packet_t *
    MAC::getCSPPacket() {

      // @TODO implement function
      return NULL;
    }

    /*!
     * @brief Receive a new CSP packet.
     *
     * @details Initialize processing of the received CSP packet. This function
     * should be followed by repeated calls to @p nextMPDU until all MPDUs
     * corresponding to the current CSP have been created and transimitted.
     *
     * @param[in] cspPacket The CSP packet to be transmitted by the UHF radio.
     *
     * @return status of the operation
     */
    uint32_t
    MAC::receiveCSPPacket(csp_packet_t * cspPacket) {
      // Lock the error correction scheme so that all of this
      // csp packet is processed using the same FEC scheme
      std::unique_lock<std::mutex> lck(m_ecSchemeMutex); // how to do this for the whole receive process?

      // prepare for chunking up the packet, encoding chunks, and passing them
      // to a listener for action

      uint32_t numMPDUs = MPDU::numberOfMPDUs(cspPacket, *m_errorCorrection);
      uint32_t cspPacketLength = ;
      uint32_t messageLength = m_errorCorrection->getMessageLen();
      uint32_t cspMessageOffset = 0;

      // invoke chunk iterator, which creates an encoded chunk then signals it's ready
      getNextCSPMPDU(true); // true means this is the start of a new csp packet

      // Set up so that listener can poll for chunks

      m_cspChunkAvailable = true;

      for (uint32_t m = 0; m < numMPDUs; m++) {

        // new mpdu header with current counts

        // get current csp data
        std::vector<uint8_t> cspChunk(m_cspChunk(cspPacket, cspPacketLength, cspMessageOffset),);

        // make MPDU

        // Notify listener a packet is ready

        cspMessageOffset += messageLength;
      }
      return 0;
    }

    /*!
     * @brief A kind of iterator that provides MPDUs corresponding to a CSP packet.
     *
     * @details Each invocation of this function returns the next MPDU
     * corresponding to a CSP packet that was passed to @p newCSPPacket.
     *
     * @param[in out] mpdu The MPDU contents are replaced by the next MPDU
     * corresponding to the received CSP packet, or zeroed if there are no more.
     *
     * @return true if there is at least one more MPDU, in which case the @p
     * mpdu contains a valid MPDU that should be transmitted. false if there
     * are no more MPDUs.
     */
    bool
    MAC::nextMPDU(MPDU &mpdu){

      // @TODO implement function
      return false;
    }


    bool
    MAC::isESTTCPacket(std::vector<uint8_t> &packet) {
      bool isPacket = false;
      // First byte should be the Data Field 1, the Data Field 2 length in bytes
      // Thus, the length of packet should be the value of the first byte + 1

      if ((uint32_t)(packet[0] + 1) == packet.size()) {
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

      if ((uint32_t)(packet[0] + 1) == packet.size()) {
        // The AX.25 frame starts with 9 bytes that have the value 0x7E
        uint8_t i;
        isPacket = true;
        for (i = 1; i <= 9; i++) {
          isPacket = isPacket & (packet[i] == 0x7E);
        }
      } // packet length is good

      return isPacket;

    }

  } /* namespace sdr */
} /* namespace ex2 */

