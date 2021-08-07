/*!
 * @file convCode27.cpp
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

#include "convCode27.hpp"
#include "pdu.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#include "viterbi.h"

#ifdef __cplusplus
}
#endif


namespace ex2 {
  namespace sdr {

    convCode27::~convCode27() {
      //rate = 1/2;
      //constraint_length = 7;
    }

    PPDU_u8
    convCode27::encode(PPDU_u8 &payload) {
      
      // hardcoded for k = 7 and rate = 1/2
      std::vector<uint8_t> g1 = {0, 1, 2, 3, 6};
      std::vector<uint8_t> g2 = {0, 2, 3, 5, 6};
      
      PPDU_u8::payload_t PayloadData = payload.getPayload();

      // append 0 at beginning for k-1
      //PPDU_u8::payload_t AppendedPayloadData (constraint_length - 1, 0);
      //AppendedPayloadData.insert(AppendedPayloadData.end(), PayloadData.begin(), PayloadData.end());
      
      PPDU_u8::payload_t encodedPayloadData;
      //encodedPayloadData.resize(2 * sizeof(PayloadData));
      // for loop of encoding 
      // hardcoded for rate = 1/2
      uint8_t shiftreg = 0;
      uint8_t sum = 0;
      for(int i = 0; i<8*payload.payloadLength() + constraint_length - 1; i++){       
        shiftreg = (shiftreg << 1) | ((PayloadData[i/8] >> (i%8)) & 1);
        //c3 -> e8....so it's not a stream
        sum = sum | (parity(shiftreg & V27POLYA) << (2 * (i%4)));
        sum = sum | (parity(shiftreg & V27POLYB) << (2 * (i%4) + 1));
        if (i%4 == 3){
            //sum = shiftreg;
            encodedPayloadData.push_back(sum);
            sum = 0;
            //shiftreg = 0;
        }
        //encodedPayloadData.push_back(adder(&AppendedPayloadData[i + constraint_length - 1],g1));
        //encodedPayloadData.push_back(adder(&AppendedPayloadData[i + constraint_length - 1],g2));
      }
      PPDU_u8 encodedPayload(encodedPayloadData);
      
      return encodedPayload;
      
    }

    uint32_t
    convCode27::decode(const PPDU_u8::payload_t& encodedPayload, float snrEstimate,
      PPDU_u8::payload_t& decodedPayload) {

      (void) snrEstimate; // Not used in this mmethod

      decodedPayload.resize(0); // Resize in all FEC decode methods

      //printf("encodedpayload[0] = %0x%02x\n", encodedPayload[0]);

      /* Init Vitrbi */
      void *vp;
      int framebits = 8*encodedPayload.size();
      if((vp = create_viterbi27_port(framebits)) == NULL){
        // Init failed.
      }

      // TODO: Use snrEstimate here.

      /* Decode it and make sure we get the right answer */
      /* Initialize Viterbi decoder */
      init_viterbi27_port(vp,0);
      
      /* Decode block */
      uint8_t encodedArr[8*framebits+constraint_length-1];
      for (int i; i<sizeof(encodedArr); i++){
        encodedArr[i] = 82 + 90*((encodedPayload[i/8] >> (i%8)) & 1);//offset needed
      }
      //std::copy(encodedPayload.begin(), encodedPayload.end(), encodedArr);
      //uint8_t * encodedPtr = &encodedPayload[0];
      update_viterbi27_blk_port(vp,encodedArr,framebits+constraint_length-1);
      
      /* Do Viterbi chainback */
      //uint8_t * decodedPtr = &decodedPayload[0];
      uint8_t decodedArr[framebits/2];
      //std::copy(decodedPayload.begin(), decodedPayload.end(), decodedArr);
      chainback_viterbi27_port(vp,decodedArr,framebits,0);
      
      //decodedPayload.insert(decodedPayload.end(), &decodedArr[0], &decodedArr[framebits]);
      for (int i; i<sizeof(decodedArr); i++){
        unsigned char revertbit = 0;
        for (int j=0; j<8; j++){
          revertbit = revertbit | (((decodedArr[i] >> j) & 1)<<(7-j));
        }
        decodedPayload.push_back(revertbit);
      }

      return 0;
    }

    uint8_t
    convCode27::adder(uint8_t * payload_sym , std::vector<uint8_t> g){
        uint8_t sum = * payload_sym;

        for (int i =0; i < g.size(); i++){
            sum |= *(payload_sym - g[i]);
        }
        // Should it necessarily be modulo-2? PDU looks like general
        return sum;

    }


  } /* namespace sdr */
} /* namespace ex2 */
