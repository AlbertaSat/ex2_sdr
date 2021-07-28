/*!
 * @file convCodeR_1_2.cpp
 * @author Arash Yazdani
 * @date July 21, 2021
 *
 * @details 
 *
 * @copyright 
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#include "convCodeR_1_2.hpp"
#include "pdu.hpp"


namespace ex2 {
  namespace sdr {

    convCodeR_1_2::~convCodeR_1_2() {  }

    PPDU_u8
    convCodeR_1_2::encode(PPDU_u8 &payload) {
      
      PPDU_u8 encodedPayload;
      
      encodedPayload.resize((1/rate) * sizeof(payload)); //what if not integer?
      // hardcoded for k = 7 and rate = 1/2
      std::vector<uint8_t> g1 = [0, 1, 2, 3, 6];
      std::vector<uint8_t> g2 = [0, 2, 3, 5, 6];
      
      payload_t * PayloadData = payload.getPayload();
      // append 0 at beginning for k-1
      payload_t AppendedPayloadData (constraint_length - 1, 0);
      AppendedPayloadData.insert(AppendedPayloadData.end(), PayloadData.begin(), PayloadData.end());
      
      payload_t encodedPayloadData;
      // for loop of encoding 
      // hardcoded for rate = 1/2
      for(uint8_t i = 0; i<payload.payloadLength(); i++){
        encodedPayloadData[i] = adder(AppendedPayloadData[i + constraint_length - 1],g1);
        encodedPayloadData[i + 1] = adder(AppendedPayloadData[i + constraint_length - 1],g2);
      }
      encodedPayload.PDU(encodedPayloadData);
      
      return encodedPayload;
      
    }

    uint32_t
    convCodeR_1_2::decode(const PPDU_u8::payload_t& encodedPayload, float snrEstimate,
      PPDU_u8::payload_t& decodedPayload) {

      (void) snrEstimate; // Not used in this mmethod

      decodedPayload.resize(0); // Resize in all FEC decode methods

      
      //decodedPayload = encodedPayload;

      return 0;
    }

    uint8_t adder(uint8_t * payload_sym , std::vector<uint8_t> g){
        uint8_t sum = * payload_sym;

        for (int i =0; i < g.size(); i++){
          if (g[i] < constraint_length && g[i]>0) {
            sum += *(payload_sym - g[i]);
          }
          // else throw sth?
        }
        // Should it necessarily be modulo-2? PDU looks like general
        return sum;

    }


  } /* namespace sdr */
} /* namespace ex2 */
