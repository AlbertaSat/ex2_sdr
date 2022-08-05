/*!
 * @file mpdu.cpp
 * @author Steven Knudsen
 * @date May 25, 2021
 *
 * @details The MPDU class.
 *
 * @copyright University of Alberta, 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#include "mpdu.hpp"

namespace ex2
{
  namespace sdr
  {

    MPDUException::MPDUException(const std::string& message) :
               runtime_error(message) { }


    MPDU::MPDU (
      MPDUHeader& header,
      std::vector<uint8_t>& codeword)
    {
      try {
      // Make a copy
      m_mpduHeader = new MPDUHeader(header);
      // TODO Not sure we need to keep the codeword for this constructor since
      // the MPDU object will be used to define a transparent mode payload
      // comprising Data Field 1 and 2, that is, the header and the codeword
      // together.
      m_payload = std::vector<uint8_t>(codeword);
      m_rawMPDU.resize(0);
      std::vector<uint8_t> temp = header.getHeaderPayload();
      m_rawMPDU.insert(m_rawMPDU.end(), temp.begin(), temp.end());
      m_rawMPDU.insert(m_rawMPDU.end(), codeword.begin(), codeword.end());
      }
      catch (MPDUHeaderException& e) {
        // @todo should log this
        printf("MPDUHeader exception : %s\n", e.what());
        throw MPDUException("MPDU: Bad MPDUHeader.");
      }
    }

    MPDU::MPDU (
      const ErrorCorrection &currentErrorCorrection,
      std::vector<uint8_t>& rawMPDU) {

      // There are several possibilities for received @p rawMPDU:
      //     1. Shorter than expected
      //     2. As long as expected
      //     3. Longer than expected
      //
      // For 1., as long as there are enough bytes to make an MPDUHeader, it's
      // worth processing because it might be the first packet fragment. The
      // MPDUHeader may decode alright and then if it's the first packet
      // fragment, we can choose to substitute dummy data for the missing,
      // but expected message. Maybe subsequent payload will be okay...
      //
      // For 2., just process to make the MPDU
      //
      // For 3., process only the expected number of bytes to make the MPDU

      // the @p rawPayload is assumed to contain Data Field 1, the raw header,
      // and the codeword. Data Field 1 is not encoded, so we take our chances;
      // assume that the radio receives only as many Data Field 2 bytes as
      // indicated by Data Field 1 regardless of how much was actually transmitted
      // by the other end. Unless Data Field 1 is 129, the raw MPDU is no good

      // @todo Maybe we should allow for Data Field 1 to be bad? For example, as
      // long as we get enough bytes for the MPDU header and can check it, we
      // can still have a partial packet. In that case, we check for rawMPDU.size()
      // >

      try {
        m_mpduHeader = new MPDUHeader(currentErrorCorrection, rawMPDU);

        // Header seems okay, so make codeword based on how many remaining bytes
        // in rawMPDU
        uint32_t minMPDULength = MPDUHeader::MACHeaderLength() + MPDU::maxMTU();
        if (rawMPDU.size() >= (minMPDULength)) {
          m_payload.assign(rawMPDU.begin()+MPDUHeader::MACHeaderLength(), rawMPDU.begin()+minMPDULength);
        }
        else {
          // insert what we have and then pad to correct length
          m_payload.assign(rawMPDU.begin()+MPDUHeader::MACHeaderLength(), rawMPDU.end());
          m_payload.resize(minMPDULength);
        }
      }
      catch (MPDUHeaderException& e) {
        // @todo should log this
        printf("MPDUHeader exception : %s\n", e.what());
        throw MPDUException("MPDU: Bad raw MPDUHeader.");
      }

      // Might as well copy the input as the member raw MPDU
      m_rawMPDU = std::vector<uint8_t>(rawMPDU);
    }

    MPDU::~MPDU ()
    {
      if (m_mpduHeader != NULL) {
        delete m_mpduHeader;
      }
    }

    const std::vector<uint8_t>&
    MPDU::getRawMPDU() const {
      return m_rawMPDU;
    }

    uint16_t
    MPDU::mpdusInNBytes(uint32_t byteCount, ErrorCorrection &errorCorrection) {

      // First find how many messages are needed for @p byteCount bytes
      uint32_t msgLen = errorCorrection.getMessageLen() / 8;
      uint32_t numMsgsPerPacket;
      if (byteCount > 0) {
        numMsgsPerPacket = byteCount / msgLen;
        if (byteCount % msgLen != 0) {
          numMsgsPerPacket++;
        }
      }
      else {
        numMsgsPerPacket = 1;
      }

      // Codewords are packed into consecutive MPDUs, so find out how many
      // are needed, being sure to round up.
      uint32_t numCodewordBytesPerPacket = numMsgsPerPacket * errorCorrection.getCodewordLen() / 8;
      uint32_t numMPDUsPerPacket = numCodewordBytesPerPacket / maxMTU();
      if (numCodewordBytesPerPacket % maxMTU() != 0) {
        numMPDUsPerPacket++;
      }

      return numMPDUsPerPacket;
    }

  } /* namespace sdr */
} /* namespace ex2 */

