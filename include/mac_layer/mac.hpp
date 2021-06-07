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

#include <vector>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "csp_types.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"

#include "queue.h"

#ifdef __cplusplus
}
#endif

#include "ppdu_cf.hpp"
#include "ppdu_u8.hpp"
//#include "configuration.h"
#include "mpdu.hpp"
#include "rfMode.hpp"

// For FreeRTOS queues
#define QUEUE_LENGTH ( 5 )

/* Priorities at which the tasks are created. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY   ( tskIDLE_PRIORITY + 2 )
#define mainQUEUE_SEND_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1 )

namespace ex2
{
  namespace sdr
  {
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

      /*!
       * @brief The task that receives CSP packets via a queue from the CSP
       * server.
       *
       * @details The task monitors the queue and when a CSP packet arrives, it
       * processes it into Transparent Mode packets that are sent in sequence
       * to the UHF Radio via the UART interface.
       *
       * @param taskParameters Just a placeholder
       */
      static void queueReceiveTask( void *taskParameters );

      /*!
       * @brief The task that send CSP packets via a queue to the CSP server.
       *
       * @details The task monitors the UART interface from the UHF Radio for
       * Transparent Mode packets. It processes those received and reconstructs
       * the source CSP packet. When a CSP packet is complete, it is sent via a
       * queue to the CSP server.
       *
       * @param taskParameters Just a placeholder
       */
      static void queueSendTask( void *taskParameters );

      /*!
       * @brief Accessor
       *
       * @return The Send queue handle
       */
      QueueHandle_t
      getSendQueueHandle () const
      {
        return xSendQueue;
      }

      /*!
       * @brief Accessor
       *
       * @return The Receive queue handle
       */
      QueueHandle_t
      getRecvQueueHandle () const
      {
        return xRecvQueue;
      }

      /*!
       * @brief Accessor
       *
       * @return The ErrorCorrectionScheme in use
       */
      ErrorCorrection::ErrorCorrectionScheme
      getMErrorCorrectionScheme () const
      {
        return m_errorCorrection->getErrorCorrectionScheme();
      }

      /*!
       * @brief Accessor
       *
       * @param mErrorCorrectionScheme The ErrorCorrectionScheme to use
       */
      void
      setMErrorCorrectionScheme (
        ErrorCorrection::ErrorCorrectionScheme ecScheme)
      {
        m_errorCorrection->setErrorCorrectionScheme(ecScheme);
      }

      /*!
       * @brief Accessor
       *
       * @return The UHF Radio RF Mode in use
       */
      RF_Mode::RF_ModeNumber
      getMRfModeNumber () const
      {
        return m_rfModeNumber;
      }

      /*!
       * @brief Accessor
       *
       * @param mRfModeNumber The UHF Radio RF Mode to use
       */
      void
      setMRfModeNumber (
        RF_Mode::RF_ModeNumber mRfModeNumber)
      {
        m_rfModeNumber = mRfModeNumber;
      }

    private:

      static MAC* m_instance;

      QueueHandle_t xSendQueue = NULL;
      QueueHandle_t xRecvQueue = NULL;

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

      void processReceivedCSP(csp_packet_t *packet);
      void processReceivedUARTPPacket(std::vector<uint8_t> &packet);

      ErrorCorrection *m_errorCorrection;

      RF_Mode::RF_ModeNumber m_rfModeNumber;

      uint16_t m_numCodewordFragments;
      uint16_t m_messageLength;

    };

  } // namespace sdr
} // namespace ex2

#endif /* EX2_SDR_MAC_LAYER_MAC_H_ */

