/*!
 * @file MPDUHeader.h
 * @author Steven Knudsen
 * @date April 30, 2021
 *
 * @details Represents the MPDU header, which contains information about the
 * radio RF Mode, Error Correction scheme, which codeword fragment and packet
 * fragment this is, and the original payload length.
 *
 * @copyright AlbertaSat 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#ifndef EX2_SDR_MAC_LAYER_PDU_MPDU_HEADER_H_
#define EX2_SDR_MAC_LAYER_PDU_MPDU_HEADER_H_

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "error_correction.hpp"
#include "pdu.hpp"
#include "rfMode.hpp"

namespace ex2 {
  namespace sdr {

    class MPDUHeaderException: public std::runtime_error {

    public:
      MPDUHeaderException(const std::string& message);
    };

    class MPDUHeader {
    public:

      /*!
       * @brief Constructor
       *
       * @param[in] uhfPacketLength The UHF radio packet length (as from Data Field 1) **deprecated
       * @param[in] modulation The UHF radio modulation (RF mode)
       * @param[in] errorCorrection The error correction for this MPDU header
       * @param[in] codewordFragmentIndex The index of the codeword fragment
       * @param[in] userPacketPayloadLength The length of the original user packet payload
       * @param[in] userPacketFragmentIndex The current fragment of the user packet
       */
      MPDUHeader(/*const uint8_t uhfPacketLength,*/
        const RF_Mode::RF_ModeNumber modulation,
        const ErrorCorrection &errorCorrection,
        const uint8_t codewordFragmentIndex,
        const uint16_t userPacketPayloadLength,
        const uint8_t userPacketFragmentIndex);

      /*!
       * @brief Constructor
       *
       * @details Reconstitute a header object from raw (received, we assume)
       * bytes. Check the data for correctness and throw an exepction if bad.
       *
       * @param[in] rawHeader The first @p k_MACHeaderLength / 8 bytes of this
       * vector are assumed to contain the MACHeader information.
       * @throws MPDUHeaderException
       */
      MPDUHeader (std::vector<uint8_t> &rawHeader);

      /*!
       * @brief Copy Constructor
       *
       * @param[in] header
       */
      MPDUHeader (MPDUHeader& header);

      virtual ~MPDUHeader();

      /*!
       * @brief Accessor
       * @return MAC header length in bytes
       */
      static uint16_t
      MACHeaderLength ()
      {
        return k_MACHeaderLength / 8;
      }

      /*!
       * @brief Return the FEC scheme
       *
       * @return The FEC aka Error Correction scheme
       */
      ErrorCorrection::ErrorCorrectionScheme
      getErrorCorrectionScheme() const
      {
        return m_errorCorrection->getErrorCorrectionScheme();
      }

      uint8_t
      getCodewordFragmentIndex () const
      {
        return m_codewordFragmentIndex;
      }

      /*!
       * @brief Return the FEC scheme codeword length
       *
       * @return FEC scheme codeword length in bits
       */
      uint32_t
      getCodewordLength () const
      {
        return m_errorCorrection->getCodewordLen();
      }

      /*!
       * @brief Return the FEC scheme message length
       *
       * @return FEC scheme message length in bits
       */
      uint32_t
      getMessageLength () const
      {
        return m_errorCorrection->getMessageLen();
      }

      const std::vector<uint8_t>&
      getHeaderPayload () const
      {
        return m_headerPayload;
      }


      RF_Mode::RF_ModeNumber
      getRfModeNumber () const
      {
        return m_rfModeNumber;
      }

      uint16_t
      getUserPacketFragmentIndex () const
      {
        return m_userPacketFragmentIndex;
      }

      uint16_t
      getUserPacketPayloadLength () const
      {
        return m_userPacketPayloadLength;
      }

      bool
      isMHeaderValid () const
      {
        return m_headerValid;
      }
    private:

      /*!
       * @details These constants are a function of the number of bits allocated
       * for their information in the header, not a function of the underlying
       * type size.
       *
       * @todo The parity bits should be based on the number of Golay codewords
       */
      static const uint16_t k_modulation              = 3; // bits
      static const uint16_t k_FECScheme               = 6; // bits
      static const uint16_t k_modulationFECScheme     = k_modulation + k_FECScheme;
      static const uint16_t k_codewordFragmentIndex   = 7; // bits
      static const uint16_t k_userPacketPayloadLength = 12; // bits
      static const uint16_t k_userPacketFragmentIndex = 8; // bits
      static const uint16_t k_parityBits              = 36; // bits
      static const uint16_t k_MACHeaderLength =
          k_modulationFECScheme +
          k_codewordFragmentIndex +
          k_userPacketPayloadLength +
          k_userPacketFragmentIndex +
          k_parityBits;

      /*uint8_t m_uhfPacketLength;*/
      RF_Mode::RF_ModeNumber m_rfModeNumber;
      ErrorCorrection *m_errorCorrection;
      uint8_t  m_codewordFragmentIndex;
      uint16_t m_userPacketPayloadLength;
      uint16_t m_userPacketFragmentIndex;
      std::vector<uint8_t> m_headerPayload;

      bool m_headerValid;

      /*!
       * @brief Used to decode a raw received packet to get the MAC header
       *
       * @param packet The received transparent mode packet
       * @return True if header decodes without errors, but could still bad because
       * if there are > 4 errors in a Golay codeword, they will not be detected
       */
      bool decodeMACHeader(std::vector<uint8_t> &packet);

      void encodeMACHeader();

    };

  } /* namespace sdr */
} /* namespace ex2 */

#endif /* EX2_SDR_MAC_LAYER_PDU_MPDU_HEADER_H_ */
