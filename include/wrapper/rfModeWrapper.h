/*!
 * @file rfModeWrapper.h
 * @author Steven Knudsen
 * @date Nov. 22, 2021
 *
 * @details 
 *
 * @copyright 
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */
#ifndef EX2_SDR_WRAPPER_RFMODE_H_
#define EX2_SDR_WRAPPER_RFMODE_H_

#include "rfMode.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  RF_MODE_0 = 0, //RF_Mode::RF_ModeNumber::RF_MODE_0,    // 2GFSK, 1200 bps,  Fdev 600Hz,   ModInd 1
  RF_MODE_1 = 1, //RF_Mode::RF_ModeNumber::RF_MODE_1,    // 2GFSK, 2400 bps,  Fdev 600Hz,   ModInd 0.5
  RF_MODE_2 = 2, //RF_Mode::RF_ModeNumber::RF_MODE_2,    // 2GFSK, 4800 bps,  Fdev 1200Hz,  ModInd 0.5
  RF_MODE_3 = 3, //RF_Mode::RF_ModeNumber::RF_MODE_3,    // 2GFSK, 9600 bps,  Fdev 2400Hz,  ModInd 0.5
  RF_MODE_4 = 4, //RF_Mode::RF_ModeNumber::RF_MODE_4,    // 2GFSK, 9600 bps,  Fdev 4800Hz,  ModInd 1
  RF_MODE_5 = 5, //RF_Mode::RF_ModeNumber::RF_MODE_5,    // 2GFSK, 19200 bps, Fdev 4800Hz,  ModInd 0.5
  RF_MODE_6 = 6, //RF_Mode::RF_ModeNumber::RF_MODE_6,    // 2GFSK, 19200 bps, Fdev 9600Hz,  ModInd 1
  RF_MODE_7 = 7, //RF_Mode::RF_ModeNumber::RF_MODE_7,    // 2GFSK, 19200 bps, Fdev 19200Hz, ModInd 2

  RF_MODE_BAD_WRAPPER_CONTEXT = 102 // needed for wrapper existance checking

} rf_mode_number_t;

#ifdef __cplusplus
}
#endif


#endif /* EX2_SDR_WRAPPER_RFMODE_H_ */
