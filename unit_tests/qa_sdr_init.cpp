/*!
 * @file qa_sdr_init.cpp
 * @author Steven Knudsen
 * @date January 18, 2023
 *
 * @details Unit test for sdr_init .
 *
 * @copyright AlbertaSat 2023
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#include "sdr_driver.h"

#include <gtest/gtest.h>

#define QA_SDR_INIT_DEBUG 0 // set to 1 for debugging output

//#define UHF_TRANSPARENT_MODE_PACKET_LENGTH 128  // UHF transparent mode packet is always 128 bytes


TEST(sdr_init, foo) {
  /* ---------------------------------------------------------------------
   * Make sure objects can be instantiated, then check accessors
   * ---------------------------------------------------------------------
   */

//   error_correction_scheme_t errorCorrectionScheme = NO_FEC;
//   rf_mode_number_t rfMode = RF_MODE_3;

//   mac_t *myMac1 = mac_create(rfMode, errorCorrectionScheme);

//   ASSERT_FALSE(myMac1 == NULL) << "Can't instantiate MAC 1";

//   mac_t *myMac2 = mac_create(rfMode, errorCorrectionScheme);

//   ASSERT_FALSE(myMac2 == NULL) << "Can't instantiate MAC 2";

//   //
//   // Check accessors
//   //
//   error_correction_scheme_t ec = get_error_correction_scheme(myMac1);

//   ASSERT_TRUE(ec == errorCorrectionScheme) << "Failed to get error correction scheme";

//   set_error_correction_scheme(myMac1, CCSDS_CONVOLUTIONAL_CODING_R_1_2);

//   ec = get_error_correction_scheme(myMac1);

//   ASSERT_TRUE(ec == CCSDS_CONVOLUTIONAL_CODING_R_1_2) << "Failed to set error correction scheme";

//   rf_mode_number_t m = get_rf_mode_number(myMac1);

//   ASSERT_TRUE(m == rfMode) << "Failed to get RF mode number aka modulation";

//   set_rf_mode_number(myMac1, RF_MODE_0);

//   m = get_rf_mode_number(myMac1);

//   ASSERT_TRUE(m == RF_MODE_0) << "Failed to set RF mode number aka modulation";

//   //
//   // Check destructor
//   //

//   mac_destroy(myMac1);

// // @todo there is no reliable way to tell if a C pointer has been freed. I did
// // check by calling one of the accessors again using myMac1 and that seg faulted,
// // so I am pretty sure the mac_destroy method works
// //  ASSERT_TRUE(myMac1 == NULL) << "Can't destroy MAC 1";

//   mac_destroy(myMac2);

// //  ASSERT_TRUE(myMac2 == NULL) << "Can't destroy MAC 2";

} // ConstructorsAndAccessors

