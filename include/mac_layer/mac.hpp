/*!
 * @file mac.h
 * @author Steven Knudsen
 * @date May 25, 2021
 *
 * @details The MAC class. It handles packets received from the application
 * layer, processes them to make transparent mode packets, and sends them to
 * the PHY. It also receives transparent mode packets, processes them, and
 * reassembles them into packets to be sent to the application layer.
 *
 * @copyright University of Alberta, 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#ifndef EX2_SDR_MAC_LAYER_MAC_H_
#define EX2_SDR_MAC_LAYER_MAC_H_

#include <mutex>
#include <stdexcept>
#include <queue>
#include <vector>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#include "FEC.hpp"
#include "ppdu_cf.hpp"
#include "ppdu_u8.hpp"
#include "mpdu.hpp"
#include "rfMode.hpp"

#define MAC_MAX_RX_BUFFERS MAC_SERVICE_QUEUE_LENGTH
#define MAC_MAX_TX_BUFFERS MAC_SERVICE_QUEUE_LENGTH


namespace ex2
{
  namespace sdr
  {

    /*!
     * @brief This is the media access controller (MAC)
     *
     * @details The MAC class is a singleton that instantiates two tasks that
     * manage queues to the upper level driver. One queue transports packets
     * from the driver to the MAC that are to be transmitted by the radio.
     * The other queue is used by the MAC to send packets up to the driver
     * that are reconstructed from packets received by the Radio.
     *
     *
     * @todo Add diagrams (state machine, messaging, other)?
     */
    class MAC
    {
    public:

      /*!
       * @brief Constructor
       *
       * @param rfModeNumber The UHF radio modulation in use.
       * @param errorCorrectionScheme The FEC scheme in use.
       */
      MAC (RF_Mode::RF_ModeNumber rfModeNumber,
        ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme);

      ~MAC ();

      /*!
       * @brief Accessor
       *
       * @return The ErrorCorrectionScheme in use
       */
      ErrorCorrection::ErrorCorrectionScheme
      getErrorCorrectionScheme () const
      {
        return m_errorCorrection->getErrorCorrectionScheme();
      }

      /*!
       * @brief Accessor
       *
       * @param ecScheme The ErrorCorrectionScheme to use
       */
      void setErrorCorrectionScheme (
        ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme);

      /*!
       * @brief Accessor
       *
       * @return The UHF Radio RF Mode in use
       */
      RF_Mode::RF_ModeNumber
      getRFModeNumber () const
      {
        return m_rfModeNumber;
      }

      /*!
       * @brief Accessor
       *
       * @param rfModeNumber The UHF Radio RF Mode to use
       */
      void
      setRFModeNumber (RF_Mode::RF_ModeNumber rfModeNumber)
      {
        m_rfModeNumber = rfModeNumber;
      }

      /************************************************************************/
      /* Receive from PHY (UHF Radio) methods                                 */
      /************************************************************************/

      enum class MAC_UHFPacketProcessingStatus : uint16_t {
        // All the necessary UHF packets have been received and an application
        // packet was formed and must be removed.
        PACKET_READY = 0x0000,
        // Not all the necessary UHF packets have been received, but there are
        // no more expected. An application packet was formed and must be
        // removed. Then the last UHF packet must be resubmitted.
        PACKET_READY_RESUBMIT_PREVIOUS_PACKET = 0x0001,
        // @todo this is not used at this point, eliminate?
        // Not all the necessary UHF packets have been received, waiting for
        // the next one.
        READY_FOR_NEXT_UHF_PACKET = 0x0002
        // @todo is another value needed to indicate that the first packet of
        // a user packet was not received? Or is READY_FOR_NEXT_UHF_PACKET good
        // enough? See current logic in mac.cpp
      };

      /*!
       * @brief Process the received UHF data as an MPDU.
       *
       * @details Process each MPDU received (UHF received data in transparent
       * mode) until a full application packet is received or there is an error.
       *
       * @param uhfPayload The transparent mode data received from the UHF radio
       * @param payloadLength The number of transparent mode data bytes received
       *
       * @return The status of the process operation. If @p PACKET_READY,
       * a raw application packet is in the raw buffer.
       */
      MAC_UHFPacketProcessingStatus processUHFPacket(const uint8_t *uhfPayload, const uint32_t payloadLength);

      /*!
       * @brief When ready, the raw packet buffer can be retrieved.
       *
       * @return Pointer to the raw packet buffer
       */
      const uint8_t *
      getRawPacketBuffer () const
      {
        return &*m_rawPacket.begin();
      }

      /*!
       * @brief When ready, the raw packet buffer length can be returned.
       *
       * @return The raw packet buffer length in bytes
       */
      uint32_t
      getRawPacketBufferLength () const
      {
        return m_rawPacket.size();
      }

      /*!
       * @brief When ready, the length of the packet data in the raw
       * packet buffer can be returned.
       *
       * @return The length of the packet data in the raw packet buffer
       */
      uint32_t
      getRawPacketLength () const
      {
        return m_rawPacket.size();
      }

      /************************************************************************/
      /* Send to PHY (UHF Radio) methods                                      */
      /************************************************************************/

      /*!
       * @brief Receive and encode new packet.
       *
       * @details Process the received packet to create MPDUs ready for
       * transmission by the UHF radio in transparent mode. If the method returns
       * true, then there will be raw MPDUs in the mpdu payloads buffer
       *
       * @param packet
       * @param len
       *
       * @return True if the packet was encoded, false otherwise
       */
        bool receivePacket(uint8_t *packet, uint16_t len);

      /*!
       * @brief Pointer to MPDU payloads buffer.
       *
       * @details This is needed for the C wrapper accessor
       *
       * @return pointer to the MPDU payloads buffer.
       */
      uint8_t * mpduPayloadsBuffer();

      /*!
       * @breif The number of bytes in the MPDU payloads buffer.
       *
       * @return Number of bytes in the MPDU payloads buffer.
       */
      uint32_t mpduPayloadsBufferLength() const;


    private:

      void m_updateErrorCorrection(
        ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme);

      void m_decodePacket();

      // member vars that define the MAC operation
      ErrorCorrection *m_errorCorrection = 0;

      FEC *m_FEC = 0;

      RF_Mode::RF_ModeNumber m_rfModeNumber;

      // Mutexs to ensure that in progress packet processing is not corrupted
      // by an inadvertant change in FEC parameters
//      std::mutex m_ecSchemeMutex;

      // buffers needed to fragment a packet prior to transmission
      std::vector<uint8_t> m_codewordBuffer;
      std::vector<uint8_t> m_transparentModePayloads;

      // member vars to track received packet fragments
      bool m_firstFragmentReceived;
      uint16_t m_currentPacketLength;
      uint16_t m_expectedMPDUs;
      uint16_t m_mpduCount;

      float m_SNREstimate;

      std::vector<uint8_t> m_rawPacket;
    };

  } // namespace sdr
} // namespace ex2

#endif /* EX2_SDR_MAC_LAYER_MAC_H_ */

