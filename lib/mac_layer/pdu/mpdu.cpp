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
      const std::vector<uint8_t>& codeword)
    {
      // Make a copy
      m_mpduHeader = new MPDUHeader(header);

      // TODO Not sure we need to keep the codeword for this constructor since
      // the MPDU object will be used to define a transparent mode payload
      // comprising Data Field 1 and 2, that is, the header and the codeword
      // together.
      m_codeword = std::vector<uint8_t>(codeword);
      m_rawMPDU.resize(0);
      std::vector<uint8_t> temp = header.getHeaderPayload();
      m_rawMPDU.insert(m_rawMPDU.begin(), temp.begin(), temp.end());
      m_rawMPDU.insert(m_rawMPDU.end(), codeword.begin(), codeword.end());
    }

    MPDU::MPDU (
      std::vector<uint8_t>& rawMPDU) {

      // the @p rawPayload is assumed to contain Data Field 1, the raw header,
      // and the codeword.
      if (rawMPDU.size() != (long unsigned int) MPDU_LENGTH) {
        throw MPDUException("The raw payload length is not 129");
      }

      // Might as well copy the input as the member raw MPDU
      m_rawMPDU = std::vector<uint8_t>(rawMPDU);

      // TODO should do better error checking and handling
      try {
        // TODO I think it's best to have the MPDU Header class determine if the
        // raw packet contains a valid header, and not to "peek" at the data
        // here. However, right now the MPDU Header class also checks the packet
        // length for validity; is that better done here, right after the next line?
        m_mpduHeader = new MPDUHeader(rawMPDU);

        // If we have a valid header, extract the payaload.
        // @todo replace magic number 10
        m_codeword.resize(0);
        m_codeword.insert(m_codeword.begin(), rawMPDU.begin()+10, rawMPDU.end());
      }
      catch (MPDUHeaderException &e) {
        throw MPDUException("Bad header in raw packet");
      }
    }

    MPDU::~MPDU ()
    {
      delete m_mpduHeader;
    }

    const std::vector<uint8_t>&
    MPDU::getRawMPDU() const {
      return m_rawMPDU;
    }

    uint32_t
    MPDU::mpdusPerCSPPacket(csp_packet_t * cspPacket, ErrorCorrection &errorCorrection) {
      // Get length of CSP packet in bytes. Make sure we are not fooled by
      // alignment, so add up the struct members
      uint32_t cspPacketSize = sizeof(csp_packet_t) + cspPacket->length;

//      printf("MPDU cspPacketSize = %d\n", cspPacketSize);

      // Get the FEC scheme message and codeword lengths in bytes
      uint32_t msgLen = errorCorrection.getMessageLen() / 8;
      if (errorCorrection.getMessageLen() % 8 != 0) {
        msgLen++;
      }
      uint32_t cwLen = errorCorrection.getCodewordLen() / 8;
      if (errorCorrection.getCodewordLen() % 8 != 0) {
        cwLen++;
      }

//      printf("msgLen = %db %dB cwLen = %db %dB\n",
//        errorCorrection.getMessageLen(), msgLen,
//        errorCorrection.getCodewordLen(), cwLen);

      // The CSP packet is split FEC codewords and the number depends on the
      // FEC scheme. Each codeword is split across 1 or more MPDU payloads
      // @TODO update MPDU doc to refer to payloads, not codewords

      uint32_t numCWsInCSPPacket = cspPacketSize / msgLen;
      if (cspPacketSize % msgLen != 0) {
        numCWsInCSPPacket++;
      }

      uint32_t numMPDUPayloadPerCW = cwLen / MPDU_MTU;
      if (cwLen % MPDU_MTU != 0) {
        numMPDUPayloadPerCW++;
      }

      printf ("numCWsInCSPPacket = %d numMPDUPayloadPerCW = %d\n", numCWsInCSPPacket, numMPDUPayloadPerCW);

      return (numMPDUPayloadPerCW * numCWsInCSPPacket);

    } // mpdusPerCSPPacket

    uint32_t
    MPDU::mpdusPerCodeword(ErrorCorrection &errorCorrection) {

      // Get the FEC scheme codeword lengths in bytes
      uint32_t cwLen = errorCorrection.getCodewordLen() / 8;
      if (errorCorrection.getCodewordLen() % 8 != 0) {
        cwLen++;
      }

      uint32_t numMPDUPayloadPerCW = cwLen / MPDU_MTU;
      if (cwLen % MPDU_MTU != 0) {
        numMPDUPayloadPerCW++;
      }

      return numMPDUPayloadPerCW;

    } // mpdusPerCodeword


  } /* namespace sdr */
} /* namespace ex2 */

