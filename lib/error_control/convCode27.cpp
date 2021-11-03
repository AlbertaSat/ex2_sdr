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
#include "viterbi/sim.c"

#ifdef __cplusplus
}
#endif

#define QA_NOISY_TEST 0


namespace ex2 {
  namespace sdr {

    convCode27::~convCode27() {
    }

    PPDU_u8
    convCode27::encode(PPDU_u8 &payload) {
      
      PPDU_u8::payload_t PayloadData = payload.getPayload();
      
      PPDU_u8::payload_t encodedPayloadData;
      //encodedPayloadData.resize(2 * sizeof(PayloadData));
      uint8_t shiftreg = 0;
      uint8_t sum = 0;
      for(int i = 0; i<8*payload.payloadLength() + constraint_length - 1; i++){       
        shiftreg = (shiftreg << 1) | ((PayloadData[i/8] >> (i%8)) & 1);
        sum = sum | (parity(shiftreg & V27POLYA) << (2 * (i%4)));
        sum = sum | (parity(shiftreg & V27POLYB) << (2 * (i%4) + 1));
        if (i%4 == 3){
            encodedPayloadData.push_back(sum);
            sum = 0;
        }
      }
      PPDU_u8 encodedPayload(encodedPayloadData);
      
      return encodedPayload;
      
    }

    uint32_t
    convCode27::decode(const PPDU_u8::payload_t& encodedPayload, float snrEstimate,
      PPDU_u8::payload_t& decodedPayload) {

      double offset = 127.5;
      uint8_t amp = 32; //
      (void) snrEstimate; // Not used in this method

      decodedPayload.resize(0); // Resize in all FEC decode methods

      /* Init Vitrbi */
      void *vp;
      int framebits = 8*encodedPayload.size();
      if((vp = create_viterbi27_port(framebits)) == NULL){
        // Init failed.
        printf("CC27 initialization failed.\n");
        return UINT32_MAX;
      }

      /* Decode it and make sure we get the right answer */
      /* Initialize Viterbi decoder */
      init_viterbi27_port(vp,0);
      
      /* Decode block */
      uint8_t encodedArr[8*framebits+constraint_length-1];
      for (int i = 0; i<sizeof(encodedArr); i++){
        #if QA_NOISY_TEST
        encodedArr[i] = addnoise(((encodedPayload[i/8] >> (i%8)) & 1), 12, amp, offset, 255);
        #else
        encodedArr[i] = offset + (((encodedPayload[i/8] >> (i%8)) & 1) ? amp : -amp);//The viterbi decoder makes a decision based on the 127 threshold.
        #endif
//#if QA_NOISY_TEST // manually forcing errors but "255-" is not a way to go.
//        if (i%8 == 0){
//          encodedArr[i] = 255 - encodedArr[i];
//        }
//#endif
      }
      update_viterbi27_blk_port(vp,encodedArr,framebits+constraint_length-1);
      
      /* Do Viterbi chainback */
      uint8_t decodedArr[framebits/2];
      chainback_viterbi27_port(vp,decodedArr,framebits,0);
      printf("chainback_viterbi27_port, sizeof decodedArr %ld\n", sizeof(decodedArr));

      for (int i = 0; i<sizeof(decodedArr); i++){
        unsigned char revertbit = 0;
        for (int j=0; j<8; j++){
          revertbit = revertbit | (((decodedArr[i] >> j) & 1)<<(7-j));
        }
        decodedPayload.push_back(revertbit);
        printf("decodedPayload.size() = %ld\n",decodedPayload.size());
      }

      return 0;
    }

  } /* namespace sdr */
} /* namespace ex2 */
