/*!
 * @file mpdu.hpp
 * @author Steven Knudsen
 * @date May 25, 2021
 *
 * @details The MPDU class.
 *
 * @todo change the user packet fragment index to be a CRC8 field
 *
 * @copyright University of Alberta, 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#ifndef EX2_SDR_MAC_LAYER_PDU_FRAME_H_
#define EX2_SDR_MAC_LAYER_PDU_FRAME_H_

#include <cstdint>
#include <stdexcept>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

#include "csp_types.h"

#ifdef __cplusplus
}
#endif

#include "pdu.hpp"
#include "mpduHeader.hpp"
#include "radio.h"

namespace ex2
{
  namespace sdr
  {

    class MPDUException: public std::runtime_error {

    public:
      MPDUException(const std::string& message);
    };

    /*!
     * @brief Defines the MPDU.
     *
     * @details The MAC Protocol Data Unit (MPDU) comprises the MAC header and
     * codeword. The codeword comprises the message and parity bits.
     *
     * The MAC header and codeword are contained in the Data Field 2 of the
     * transparent mode packet. The fields and their length are shown in the
     * two figures below.
     *
     * @dot
     * digraph html {
     *   packet [shape=none, margin=0, label=<
     *     <TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0" CELLPADDING="4">
     *       <TR >
     *       <TD bgcolor="#aaaaaa" COLSPAN="5" Align = "Center">Data Field 2</TD>
     *       </TR>
     *       <TR >
     *       <TD bgcolor="#ffffff" COLSPAN="5" Align = "Center"></TD>
     *       </TR>
     *       <TR>
     *       <TD bgcolor="#ff7777" COLSPAN="1" Align = "Center">MAC Header (9 bytes)</TD>
     *       <TD bgcolor="#77ff77" COLSPAN="4" Align = "Center">Codeword (Message + Parity) (0 - 119 bytes)</TD>
     *       </TR>
     *     </TABLE>
     *   >];
     *  }
     * @enddot
     * The MAC header comprises
     * * the Modulation/FEC scheme (9 bits)
     *   * the Modulation (3 bits)
     *   * the FEC Scheme (6 bits)
     * * the Codeword Fragment Index (7 bits), an index used to order split codewords when a FEC scheme codeword is longer than 119 bytes
     * * the User Packet Length, the length of the user packet provided to the MAC, which should be a CSP packet
     * * the User Packet Fragement Index, which of the User Packet fragments this is
     * *
     * @dot
     * digraph html {
     *   packet [shape=none, margin=0, label=<
     *     <TABLE BORDER="0" CELLBORDER="1" CELLSPACING="0" CELLPADDING="4">
     *       <TR >
     *       <TD bgcolor="#ff7777" COLSPAN="25" Align = "Center">MAC Header (9 bytes)</TD>
     *       </TR>
     *       <TR>
     *       <TD bgcolor="#ffffff" COLSPAN="25"></TD>
     *       </TR>
     *       <TR>
     *       <TD bgcolor="#ff7777" COLSPAN="3" Align = "Center">Modulation/FEC Scheme (9 bits)</TD>
     *       <TD bgcolor="#ff7777" COLSPAN="4" Align = "Center">Codeword Fragment Index (7 bits)</TD>
     *       <TD bgcolor="#ff7777" COLSPAN="5" Align = "Center">User Packet Payload Length (12 bits)</TD>
     *       <TD bgcolor="#ff7777" COLSPAN="6" Align = "Center">User Packet Fragment Index (8 bits)</TD>
     *       <TD bgcolor="#ff7777" COLSPAN="7" Align = "Center">Golay parity bits[Note 1] (36 bits)</TD>
     *       </TR>
     *       <TR>
     *       <TD bgcolor="#ffffff" COLSPAN="3"></TD>
     *       </TR>
     *       <TR>
     *       <TD bgcolor="#ff7777" COLSPAN="1">Modulation (4 bits)</TD>
     *       <TD bgcolor="#ff7777" COLSPAN="2">FEC Scheme (6 bits)</TD>
     *       </TR>
     *     </TABLE>
     *   >];
     *  }
     * @enddot
     *
     */
    class MPDU {
    public:

      /*!
       * @brief Constructor
       *
       * @details The @p header should be constructed new for each MPDU created
       * so that the correct indices are set. See @p MPDUHeader class. The
       * @p codeword is assumed to be an FEC encoded message and must be MPDU_MTU
       * bytes long.
       *
       * @param[in] header The header corresponding to this MPDU
       * @param[in] payload MAC service data unit
       */
      MPDU (
        MPDUHeader& header,
        std::vector<uint8_t>& payload);

      /*!
       * @brief Constructor
       *
       * @details Used when reconstructing an MPDU based on a received
       * transparent mode packet
       *
       * @note The @p rawMPDU can be any length since the UHF radio will "receive"
       * as many bytes as it thinks are in the Data Field 1, which may be
       * corrupted since there is no FEC encoding on it. As long as the rawMPDU
       * is long enough to make an MPDUHeader and that header appears to be
       * valid, an MPDU will be made. If there is not enough data past the
       * MPDUHeader in @p rawMPDU, bytes will be added to the codeword
       *
       * @param[in] rawMPDU The received transparent mode Data Field 2 as a byte vector
       */
      MPDU (
        std::vector<uint8_t>& rawMPDU);

      ~MPDU ();

      /*!
       * @brief Accessor for the MPDU as a byte vector.
       *
       * @return The MPDU as a byte vector, including the header.
       */
      const std::vector<uint8_t>& getRawMPDU() const;

      /*!
       * @brief The length of the raw MPDU comprising the header and the payload
       *
       * @return Length of the raw MPDU comprising the header and the payload
       */
      static uint32_t rawMPDULength() {
        return UHF_TRANSPARENT_MODE_DATA_FIELD_2_MAX_LENGTH;
      }

      /*!
       * @brief Accessor for payload
       *
       * @return The payload
       */
      const std::vector<uint8_t>& getPayload() const {
        return m_payload;
      }

      /*!
       * @brief Accessor for MPDU header
       * @return The header.
       */
      MPDUHeader* getMpduHeader() const {
        return m_mpduHeader;
      }

      /*!
       * @brief Return the maximum transmission unit in bytes
       *
       * @return The MTU in bytes
       */
      static uint16_t maxMTU() {
        return UHF_TRANSPARENT_MODE_DATA_FIELD_2_MAX_LENGTH - MPDUHeader::MACHeaderLength();
      }

      /*!
       * @brief Return the number of MPDUs in N bytes for the given FEC scheme
       *
       * @param[in] byteCount The number of bytes we assume need to be turned
       * into MPDUs
       *
       * @return errorCorrection Reference to the current FEC scheme
       */
      static uint16_t mpdusInNBytes(uint32_t byteCount, ErrorCorrection &errorCorrection);

    private:
      MPDUHeader *m_mpduHeader;
      std::vector<uint8_t> m_payload;
      std::vector<uint8_t> m_rawMPDU;
    };

  } // namespace sdr
} // namespace ex2

#endif /* EX2_SDR_MAC_LAYER_PDU_FRAME_H_ */

