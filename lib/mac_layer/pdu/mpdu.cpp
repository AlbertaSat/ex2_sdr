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
      m_payload.resize(0);
      std::vector<uint8_t> temp = header.getHeaderPayload();
      m_payload.insert(m_payload.begin(), temp.begin(), temp.end());
      m_payload.insert(m_payload.end(), codeword.begin(), codeword.end());
    }

    MPDU::MPDU (
      std::vector<uint8_t>& rawPayload) {

      //      uint16_t rawPayloadLength = MPDU_DATA_FIELD_1_SIZE + MPDU_MTU + MPDUHeader::MACHeaderLength()/8;

      // the @p rawPayload is assumed to contain Data Field 1, the raw header,
      // and the codeword.
      if (rawPayload.size() != (long unsigned int) MPDU_LENGTH) {
        throw MPDUException("The raw payload length is not 129");
      }

      // TODO should do better error checking and handling
      try {
        // TODO I think it's best to have the MPDU Header class determine if the
        // raw packet contains a valid header, and not to "peek" at the data
        // here. However, right now the MPDU Header class also checks the packet
        // length for validity; is that better done here, right after the next line?
        m_mpduHeader = new MPDUHeader(rawPayload);

        // If we have a valid header,
        m_codeword.resize(0);
        m_codeword.insert(m_codeword.begin(), rawPayload.begin()+10, rawPayload.end());
        m_payload = std::vector<uint8_t>(rawPayload);
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
    MPDU::getMPDU() const {
      return m_payload;
    }


  } /* namespace sdr */
} /* namespace ex2 */

