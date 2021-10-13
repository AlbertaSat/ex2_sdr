/*!
 * @file error_correction.hpp
 * @author StevenKnudsen
 * @date April 30, 2021
 *
 * @details Define the Error Correction schemes that may be supported by
 * the UHF transparent mode.
 *
 * @copyright AlbertaSat 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#ifndef EX2_SDR_ERROR_CONTROL_ERROR_CORRECTION_H_
#define EX2_SDR_ERROR_CONTROL_ERROR_CORRECTION_H_

#include <cstdint>
#include <string>
#include <vector>

//#include "mpdu.hpp"

#define CCSDS_CONVOLUTIONAL_CODING_K ( 7 )

namespace ex2 {
  namespace sdr {

    /*!
     * @brief Define a forward error correction scheme.
     */
    class ErrorCorrection {
    public:

      /*!
       * @brief The possible error correction block codes
       *
       * @details These must be enumerated sequetially with integers so that
       * iterating will work. If schemes are added, @p LAST will need updating
       */
      enum class ErrorCorrectionScheme : uint16_t {
        CCSDS_CONVOLUTIONAL_CODING_R_1_2          = 0x0000, // CCSDS convolutional coding rate 1/2
        CCSDS_CONVOLUTIONAL_CODING_R_2_3          = 0x0001, // CCSDS convolutional coding rate 2/3
        CCSDS_CONVOLUTIONAL_CODING_R_3_4          = 0x0002, // CCSDS convolutional coding rate 3/4
        CCSDS_CONVOLUTIONAL_CODING_R_5_6          = 0x0003, // CCSDS convolutional coding rate 5/6
        CCSDS_CONVOLUTIONAL_CODING_R_7_8          = 0x0004, // CCSDS convolutional coding rate 7/8
        CCSDS_REED_SOLOMON_255_239_INTERLEAVING_1 = 0x0005, // Reed-Solomon (255,239) interleaving 1
        CCSDS_REED_SOLOMON_255_239_INTERLEAVING_2 = 0x0006, // Reed-Solomon (255,239) interleaving 2
        CCSDS_REED_SOLOMON_255_239_INTERLEAVING_3 = 0x0007, // Reed-Solomon (255,239) interleaving 3
        CCSDS_REED_SOLOMON_255_239_INTERLEAVING_4 = 0x0008, // Reed-Solomon (255,239) interleaving 4
        CCSDS_REED_SOLOMON_255_239_INTERLEAVING_5 = 0x0009, // Reed-Solomon (255,239) interleaving 5
        CCSDS_REED_SOLOMON_255_239_INTERLEAVING_8 = 0x000A, // Reed-Solomon (255,239) interleaving 8
        CCSDS_REED_SOLOMON_255_223_INTERLEAVING_1 = 0x000B, // Reed-Solomon (255,223) interleaving 1
        CCSDS_REED_SOLOMON_255_223_INTERLEAVING_2 = 0x000C, // Reed-Solomon (255,223) interleaving 2
        CCSDS_REED_SOLOMON_255_223_INTERLEAVING_3 = 0x000D, // Reed-Solomon (255,223) interleaving 3
        CCSDS_REED_SOLOMON_255_223_INTERLEAVING_4 = 0x000E, // Reed-Solomon (255,223) interleaving 4
        CCSDS_REED_SOLOMON_255_223_INTERLEAVING_5 = 0x000F, // Reed-Solomon (255,223) interleaving 5
        CCSDS_REED_SOLOMON_255_223_INTERLEAVING_8 = 0x0010, // Reed-Solomon (255,223) interleaving 8
        CCSDS_TURBO_1784_R_1_2                    = 0x0011, // CCSDS 131.0-B-3 Turbo coding k=1784 rate 1/2
        CCSDS_TURBO_1784_R_1_3                    = 0x0012, // CCSDS 131.0-B-3 Turbo coding k=1784 rate 1/3
        CCSDS_TURBO_1784_R_1_4                    = 0x0013, // CCSDS 131.0-B-3 Turbo coding k=1784 rate 1/4
        CCSDS_TURBO_1784_R_1_6                    = 0x0014, // CCSDS 131.0-B-3 Turbo coding k=1784 rate 1/6
        CCSDS_TURBO_3568_R_1_2                    = 0x0015, // CCSDS 131.0-B-3 Turbo coding k=3568 rate 1/2
        CCSDS_TURBO_3568_R_1_3                    = 0x0016, // CCSDS 131.0-B-3 Turbo coding k=3568 rate 1/3
        CCSDS_TURBO_3568_R_1_4                    = 0x0017, // CCSDS 131.0-B-3 Turbo coding k=3568 rate 1/4
        CCSDS_TURBO_3568_R_1_6                    = 0x0018, // CCSDS 131.0-B-3 Turbo coding k=3568 rate 1/6
        CCSDS_TURBO_7136_R_1_2                    = 0x0019, // CCSDS 131.0-B-3 Turbo coding k=7136 rate 1/2
        CCSDS_TURBO_7136_R_1_3                    = 0x001A, // CCSDS 131.0-B-3 Turbo coding k=7136 rate 1/3
        CCSDS_TURBO_7136_R_1_4                    = 0x001B, // CCSDS 131.0-B-3 Turbo coding k=7136 rate 1/4
        CCSDS_TURBO_7136_R_1_6                    = 0x001C, // CCSDS 131.0-B-3 Turbo coding k=7136 rate 1/6
        CCSDS_TURBO_8920_R_1_2                    = 0x001D, // CCSDS 131.0-B-3 Turbo coding k=8920 rate 1/2
        CCSDS_TURBO_8920_R_1_3                    = 0x001E, // CCSDS 131.0-B-3 Turbo coding k=8920 rate 1/3
        CCSDS_TURBO_8920_R_1_4                    = 0x001F, // CCSDS 131.0-B-3 Turbo coding k=8920 rate 1/4
        CCSDS_TURBO_8920_R_1_6                    = 0x0020, // CCSDS 131.0-B-3 Turbo coding k=8920 rate 1/6
        CCSDS_LDPC_ORANGE_BOOK_1280               = 0x0021, // CCSDS Orange Book 131.1-O-2 LDPC n=1280
        CCSDS_LDPC_ORANGE_BOOK_1536               = 0x0022, // CCSDS Orange Book 131.1-O-2 LDPC n=1536
        CCSDS_LDPC_ORANGE_BOOK_2048               = 0x0023, // CCSDS Orange Book 131.1-O-2 LDPC n=2048
        IEEE_802_11N_QCLDPC_648_R_1_2             = 0x0024, // IEEE 802.11n QC-LDPC n=648 rate 1/2
        IEEE_802_11N_QCLDPC_648_R_2_3             = 0x0025, // IEEE 802.11n QC-LDPC n=648 rate 2/3
        IEEE_802_11N_QCLDPC_648_R_3_4             = 0x0026, // IEEE 802.11n QC-LDPC n=648 rate 3/4
        IEEE_802_11N_QCLDPC_648_R_5_6             = 0x0027, // IEEE 802.11n QC-LDPC n=648 rate 5/6
        IEEE_802_11N_QCLDPC_1296_R_1_2            = 0x0028, // IEEE 802.11n QC-LDPC n=1296 rate 1/2
        IEEE_802_11N_QCLDPC_1296_R_2_3            = 0x0029, // IEEE 802.11n QC-LDPC n=1296 rate 2/3
        IEEE_802_11N_QCLDPC_1296_R_3_4            = 0x002A, // IEEE 802.11n QC-LDPC n=1296 rate 3/4
        IEEE_802_11N_QCLDPC_1296_R_5_6            = 0x002B, // IEEE 802.11n QC-LDPC n=1296 rate 5/6
        IEEE_802_11N_QCLDPC_1944_R_1_2            = 0x002C, // IEEE 802.11n QC-LDPC n=1944 rate 1/2
        IEEE_802_11N_QCLDPC_1944_R_2_3            = 0x002D, // IEEE 802.11n QC-LDPC n=1944 rate 2/3
        IEEE_802_11N_QCLDPC_1944_R_3_4            = 0x002E, // IEEE 802.11n QC-LDPC n=1944 rate 3/4
        IEEE_802_11N_QCLDPC_1944_R_5_6            = 0x002F, // IEEE 802.11n QC-LDPC n=1944 rate 5/6

        NO_FEC                                    = 0x0030, // No FEC

        LAST                                      = 0x0031 // This gets changes if we add more schemes
      };

      static const std::string ErrorCorrectionName(ErrorCorrectionScheme ecScheme);

      /*!
       * @brief The possible encoding rates.
       *
       * @details These must be enumerated sequetially with integers so that
       * iterating will work. If schemes are added, @p RATE_LAST will need updating
       *
       * @note Only some can be used with any one @p ErrorCorrectionScheme
       */
      enum class CodingRate : uint16_t {
        RATE_START = 0x0000,
        RATE_1_6   = 0x0001,  // RATE_1_6
        RATE_1_5   = 0x0002,  // RATE_1_5
        RATE_1_4   = 0x0003,  // RATE_1_4
        RATE_1_3   = 0x0004,  // RATE_1_3
        RATE_1_2   = 0x0005,  // RATE_1_2
        RATE_2_3   = 0x0006,  // RATE_2_3
        RATE_3_4   = 0x0007,  // RATE_3_4
        RATE_4_5   = 0x0008,  // RATE_4_5
        RATE_5_6   = 0x0009,  // RATE_5_6
        RATE_7_8   = 0x000A,  // RATE_7_8
        RATE_8_9   = 0x0010,  // RATE_8_9
        RATE_1     = 0x0011,  // RATE_1 used for no FEC
        RATE_NA    = 0x0012,
        RATE_BAD   = 0x0013,
        RATE_LAST  = 0x0014
      };

    public:

      /*!
       * @brief Constructor
       *
       * @param[in] ecScheme The error correction scheme, whcih is the coding
       *   method and rate combined
       * @param[in] continuousMaxCodewordLen For schemes that support continuous
       *   coding (i.e., not block coding) the maximum codeword length in bits.
       *   The default is set to the MPDU MTU
       */
      ErrorCorrection(ErrorCorrectionScheme ecScheme = ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2,
        uint32_t continuousMaxCodewordLen = 0);

      virtual
      ~ErrorCorrection();

      /*!
       * @brief Calculate the number of codeword fragments given a packet
       * payload length
       *
       * @details A utility function to find how many payloads are required to
       * contain a codeword. Useful to find how many packets are needed to
       * be transmitted for a given codeword.
       *
       * @param[in] payloadLength The packet payload length in bytes
       * @return The number of codeword fragments
       */
      uint32_t numCodewordFragments(uint32_t payloadLength);

//      /*!
//       * @brief Decode the codeword using the current ErrorCorrectionScheme
//       *
//       * @note Not all schemes are supported yet.
//       *
//       * @param [in out] codeword On invocation, the codeword to decode. If
//       * successful, the message replaces the contents. If not successful,
//       * the contents are indeterminate, invalid.
//       * @return If successful, the message is returned. If not, a zero-length
//       * vector is returned.
//       */
//      std::vector<uint8_t> & decode(std::vector<uint8_t> &codeword);
//
//      /*!
//       * @brief Encode the message using the current ErrorCorrectionScheme
//       *
//       * @param message
//       * @return If successful, the codeword is returned. If not, a zero-length
//       * vector is returned.
//       */
//      std::vector<uint8_t> & encode(std::vector<uint8_t> &codeword);

      /*!
       * @brief Accessor
       * @return The ErrorCorrectionScheme
       */
      ErrorCorrectionScheme
      getErrorCorrectionScheme () const
      {
        return m_errorCorrectionScheme;
      }

      /*!
       * @brief Accessor
       * @param mErrorCorrectionScheme The ErrorCorrectionScheme to use
       */
      void
      setErrorCorrectionScheme (
        ErrorCorrectionScheme errorCorrectionScheme);

      /*!
       * @brief Return the rate
       * @return The rate
       */
      CodingRate getCodingRate() const {
        return m_codingRate;
      }

      /*!
       * @brief Accessor
       * @return The length of the codeword in bits
       */
      uint32_t getCodewordLen() const {
        return m_codewordLen;
      }

      /*!
       * @brief Accessor
       * @return The length of the message in bits
       */
      uint32_t getMessageLen() const {
        return m_messageLen;
      }

      /*!
       * @brief Accessor
       * @return The coding rate
       */
      double getRate() const {
        return m_rate;
      }

      static bool isValid(ErrorCorrectionScheme scheme);

    private:
      ErrorCorrectionScheme m_errorCorrectionScheme;
      CodingRate m_codingRate;
      uint32_t m_continuousMaxCodewordLen;

      CodingRate m_getCodingRate(ErrorCorrectionScheme scheme);

      CodingRate m_bits2rate(uint16_t bits) const;

//      ErrorCorrectionScheme m_bits2errorCorrection(uint16_t bits) const;

      double m_rate;          // r, fractional rate
      uint32_t m_messageLen;  // k, bits
      uint32_t m_codewordLen; // n, bits

      double m_codingRateToFractionalRate();

      /*!
       * @brief The codeword length in bits for the current scheme
       *
       * @return The codeword length in bits
       */
      uint32_t m_codewordLength();

      /*!
       * @brief The message length in bits for the current scheme
       *
       * @return The message length in bits
       */
      uint32_t m_messageLength();

    };

  } /* namespace sdr */
} /* namespace ex2 */

#endif /* EX2_SDR_ERROR_CONTROL_ERROR_CORRECTION_H_ */
