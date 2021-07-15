/*!
 * @file mac.h
 * @author Steven Knudsen
 * @date May 25, 2021
 *
 * @details The MAC class. It handles CSP packets received from the application
 * layer, processes them to make transparent more packets, and sends them to
 * the PHY. It also receives transparent mode packets, processes them, and
 * reassembles them into CSP packets to be sent to the application layer.
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
#include <vector>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "csp_types.h"

#ifdef __cplusplus
}
#endif

#include "FEC.hpp"
#include "ppdu_cf.hpp"
#include "ppdu_u8.hpp"
#include "mpdu.hpp"
#include "rfMode.hpp"

// TODO This definition belongs somewhere else
#define CSP_MTU_LENGTH ( 4096 )

// TODO This definition belongs to the MAC service
#define MAC_SERVICE_QUEUE_LENGTH ( 5 )

#define MAC_MAX_RX_BUFFERS MAC_SERVICE_QUEUE_LENGTH
#define MAC_MAX_TX_BUFFERS MAC_SERVICE_QUEUE_LENGTH


namespace ex2
{
  namespace sdr
  {

    class MACException: public std::runtime_error {

    public:
      MACException(const std::string& message);
    };

    /*!
     * @brief This is the media access controller (MAC)
     *
     * @details The MAC class is a singleton that instantiates two tasks that
     * manage queues to the CSP server. One queue transports CSP packets from
     * the CSP server to the MAC that are to be transmitted by the UHF radio.
     * The other queue is used by the MAC to send CSP packets up to the CSP
     * server that are reconstructed from packets received by the UHF Radio.
     *
     *
     * @todo Add diagrams (state machine, messaging, other)?
     */
    class MAC
    {
    public:

      /*!
       * @brief Return a pointer to singleton instance of Configuration.
       *
       * @details Provides access to the Configuration.
       *
       * @warning This class may not be thread-safe.
       *
       * @param rfModeNumber The UHF radio modulation in use.
       * @param errorCorrectionScheme The FEC scheme in use.
       * @return pointer to the @p MAC instance
       */
      static MAC *
      instance (RF_Mode::RF_ModeNumber rfModeNumber,
        ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme
      );

      static MAC * instance();

      ~MAC ();

      // @TODO keep this old code for use in the wrapper
//      /*!
//       * @brief The task that receives CSP packets via a queue from the CSP
//       * server.
//       *
//       * @details The task monitors the queue and when a CSP packet arrives, it
//       * processes it into Transparent Mode packets that are sent in sequence
//       * to the UHF Radio via the UART interface.
//       *
//       * @param taskParameters Just a placeholder
//       */
//       static void queueSendToUHFTask( void *taskParameters );
//
//      /*!
//       * @brief The task that send CSP packets via a queue to the CSP server.
//       *
//       * @details The task monitors the UART interface from the UHF Radio for
//       * Transparent Mode packets. It processes those received and reconstructs
//       * the source CSP packet. When a CSP packet is complete, it is sent via a
//       * queue to the CSP server.
//       *
//       * @param taskParameters Just a placeholder
//       */
//       static void queueReceiveFromUHFTask( void *taskParameters );
//
//      /*!
//       * @brief Accessor
//       *
//       * @return The Send queue handle
//       */
//      QueueHandle_t
//      getSendQueueHandle () const
//      {
//        return xSendToUHFQueue;
//      }
//
//      /*!
//       * @brief Accessor
//       *
//       * @return The Receive queue handle
//       */
//      QueueHandle_t
//      getRecvQueueHandle () const
//      {
//        return xRecvFromUHFQueue;
//      }

      /*!
       * @brief Process the received UHF data as an MPDU.
       *
       * @details Process each MPDU received (UHF received data in transparent
       * mode) until a full CSP packet is received or there is an error.
       *
       * @param mpdu
       *
       * @throw MACException out of sequence
       * @throw MACException CSP complete
       */
      void processUHFPacket(MPDU &mpdu);

      /*!
       * @brief Reset the processing of received UHF data.
       *
       * @details Some kind of error (dropped packet, bad packet, etc.) has
       * happened and the current CSP packet cannot be recovered. Reset the
       * processing.
       */
      void resetUHFProcessing();

      /*!
       * @brief A complete CSP packet is ready to be sent up to the the CSP Server.
       *
       * @return true if there is a CSP packet, false otherwise
       */
      bool isCSPPacketReady();

      /*!
       * @brief Accessor
       *
       * @details After the CSP packet has been retrieved, call @p resetUHFProcessing.
       *
       * @return Pointer to the completed CSP packet.
       */
      csp_packet_t * getCSPPacket();

      /*!
       * @brief Receive a new CSP packet.
       *
       * @details Initialize processing of the received CSP packet. This function
       * should be followed by repeated calls to @p nextMPDU until all MPDUs
       * corresponding to the current CSP have been created and transimitted.
       *
       * @param cspPacket
       */
      void newCSPPacket(csp_packet_t * cspPacket);

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
      bool nextMPDU(MPDU &mpdu);

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
      void
      setErrorCorrectionScheme (
        ErrorCorrection::ErrorCorrectionScheme ecScheme)
      {
        // Lock things to ensure that a currently processing packet is incorrectly
        // encoded or decoded
        std::unique_lock<std::mutex> lck(m_ecSchemeMutex);
        m_errorCorrection->setErrorCorrectionScheme(ecScheme);
      }

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
      setRFModeNumber (
        RF_Mode::RF_ModeNumber rfModeNumber)
      {
        m_rfModeNumber = rfModeNumber;
      }

    private:

      static MAC* m_instance;

      /*!
       * @brief Constructor
       *
       * @param rfModeNumber The UHF radio modulation in use.
       * @param errorCorrectionScheme The FEC scheme in use.
       */
      MAC (RF_Mode::RF_ModeNumber rfModeNumber,
        ErrorCorrection::ErrorCorrectionScheme errorCorrectionScheme);

      bool isESTTCPacket(std::vector<uint8_t> &packet);

      bool isAX25Packet(std::vector<uint8_t> &packet);

      ErrorCorrection *m_errorCorrection;

      FEC *m_FEC;

      RF_Mode::RF_ModeNumber m_rfModeNumber;

      uint16_t m_numCodewordFragments;
      uint16_t m_messageLength;

      // Mutexs to ensure that in progress packet processing is not corrupted
      // by an inadvertant change in FEC parameters
      std::mutex m_ecSchemeMutex;

      std::vector<uint8_t> m_receiveUHFBuffer;
      uint32_t m_codewordFragmentCount;
      uint32_t m_userPacketFragementCount;
    };

  } // namespace sdr
} // namespace ex2

#endif /* EX2_SDR_MAC_LAYER_MAC_H_ */

