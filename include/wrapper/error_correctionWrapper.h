/*!
 * @file error_configurationWrapper.h
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
#ifndef EX2_SDR_WRAPPER_ERROR_CORRECTION_H_
#define EX2_SDR_WRAPPER_ERROR_CORRECTION_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  CCSDS_CONVOLUTIONAL_CODING_R_1_2          = 0, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_1_2, // CCSDS convolutional coding rate 1/2
  CCSDS_CONVOLUTIONAL_CODING_R_2_3          = 1, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_2_3, // CCSDS convolutional coding rate 2/3
  CCSDS_CONVOLUTIONAL_CODING_R_3_4          = 2, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_3_4, // CCSDS convolutional coding rate 3/4
  CCSDS_CONVOLUTIONAL_CODING_R_5_6          = 3, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_5_6, // CCSDS convolutional coding rate 5/6
  CCSDS_CONVOLUTIONAL_CODING_R_7_8          = 4, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_CONVOLUTIONAL_CODING_R_7_8, // CCSDS convolutional coding rate 7/8
  CCSDS_REED_SOLOMON_255_239_INTERLEAVING_1 = 5, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_REED_SOLOMON_255_239_INTERLEAVING_1, // Reed-Solomon (255,239) interleaving 1
  CCSDS_REED_SOLOMON_255_239_INTERLEAVING_2 = 6, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_REED_SOLOMON_255_239_INTERLEAVING_2, // Reed-Solomon (255,239) interleaving 2
  CCSDS_REED_SOLOMON_255_239_INTERLEAVING_3 = 7, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_REED_SOLOMON_255_239_INTERLEAVING_3, // Reed-Solomon (255,239) interleaving 3
  CCSDS_REED_SOLOMON_255_239_INTERLEAVING_4 = 8, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_REED_SOLOMON_255_239_INTERLEAVING_4, // Reed-Solomon (255,239) interleaving 4
  CCSDS_REED_SOLOMON_255_239_INTERLEAVING_5 = 9, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_REED_SOLOMON_255_239_INTERLEAVING_5, // Reed-Solomon (255,239) interleaving 5
  CCSDS_REED_SOLOMON_255_239_INTERLEAVING_8 = 10, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_REED_SOLOMON_255_239_INTERLEAVING_8, // Reed-Solomon (255,239) interleaving 8
  CCSDS_REED_SOLOMON_255_223_INTERLEAVING_1 = 11, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_REED_SOLOMON_255_223_INTERLEAVING_1, // Reed-Solomon (255,223) interleaving 1
  CCSDS_REED_SOLOMON_255_223_INTERLEAVING_2 = 12, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_REED_SOLOMON_255_223_INTERLEAVING_2, // Reed-Solomon (255,223) interleaving 2
  CCSDS_REED_SOLOMON_255_223_INTERLEAVING_3 = 13, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_REED_SOLOMON_255_223_INTERLEAVING_3, // Reed-Solomon (255,223) interleaving 3
  CCSDS_REED_SOLOMON_255_223_INTERLEAVING_4 = 14, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_REED_SOLOMON_255_223_INTERLEAVING_4, // Reed-Solomon (255,223) interleaving 4
  CCSDS_REED_SOLOMON_255_223_INTERLEAVING_5 = 15, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_REED_SOLOMON_255_223_INTERLEAVING_5, // Reed-Solomon (255,223) interleaving 5
  CCSDS_REED_SOLOMON_255_223_INTERLEAVING_8 = 16, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_REED_SOLOMON_255_223_INTERLEAVING_8, // Reed-Solomon (255,223) interleaving 8
  CCSDS_TURBO_1784_R_1_2                    = 17, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_TURBO_1784_R_1_2, // CCSDS 131.0-B-3 Turbo coding k=1784 rate 1/2
  CCSDS_TURBO_1784_R_1_3                    = 18, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_TURBO_1784_R_1_3, // CCSDS 131.0-B-3 Turbo coding k=1784 rate 1/3
  CCSDS_TURBO_1784_R_1_4                    = 19, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_TURBO_1784_R_1_4, // CCSDS 131.0-B-3 Turbo coding k=1784 rate 1/4
  CCSDS_TURBO_1784_R_1_6                    = 20, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_TURBO_1784_R_1_6, // CCSDS 131.0-B-3 Turbo coding k=1784 rate 1/6
  CCSDS_TURBO_3568_R_1_2                    = 21, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_TURBO_3568_R_1_2, // CCSDS 131.0-B-3 Turbo coding k=3568 rate 1/2
  CCSDS_TURBO_3568_R_1_3                    = 22, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_TURBO_3568_R_1_3, // CCSDS 131.0-B-3 Turbo coding k=3568 rate 1/3
  CCSDS_TURBO_3568_R_1_4                    = 23, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_TURBO_3568_R_1_4, // CCSDS 131.0-B-3 Turbo coding k=3568 rate 1/4
  CCSDS_TURBO_3568_R_1_6                    = 24, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_TURBO_3568_R_1_6, // CCSDS 131.0-B-3 Turbo coding k=3568 rate 1/6
  CCSDS_TURBO_7136_R_1_2                    = 25, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_TURBO_7136_R_1_2, // CCSDS 131.0-B-3 Turbo coding k=7136 rate 1/2
  CCSDS_TURBO_7136_R_1_3                    = 26, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_TURBO_7136_R_1_3, // CCSDS 131.0-B-3 Turbo coding k=7136 rate 1/3
  CCSDS_TURBO_7136_R_1_4                    = 27, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_TURBO_7136_R_1_4, // CCSDS 131.0-B-3 Turbo coding k=7136 rate 1/4
  CCSDS_TURBO_7136_R_1_6                    = 28, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_TURBO_7136_R_1_6, // CCSDS 131.0-B-3 Turbo coding k=7136 rate 1/6
  CCSDS_TURBO_8920_R_1_2                    = 29, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_TURBO_8920_R_1_2, // CCSDS 131.0-B-3 Turbo coding k=8920 rate 1/2
  CCSDS_TURBO_8920_R_1_3                    = 30, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_TURBO_8920_R_1_3, // CCSDS 131.0-B-3 Turbo coding k=8920 rate 1/3
  CCSDS_TURBO_8920_R_1_4                    = 31, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_TURBO_8920_R_1_4, // CCSDS 131.0-B-3 Turbo coding k=8920 rate 1/4
  CCSDS_TURBO_8920_R_1_6                    = 32, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_TURBO_8920_R_1_6, // CCSDS 131.0-B-3 Turbo coding k=8920 rate 1/6
  CCSDS_LDPC_ORANGE_BOOK_1280               = 33, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_LDPC_ORANGE_BOOK_1280, // CCSDS Orange Book 131.1-O-2 LDPC n=1280
  CCSDS_LDPC_ORANGE_BOOK_1536               = 34, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_LDPC_ORANGE_BOOK_1536, // CCSDS Orange Book 131.1-O-2 LDPC n=1536
  CCSDS_LDPC_ORANGE_BOOK_2048               = 35, //ErrorCorrection::ErrorCorrectionScheme::CCSDS_LDPC_ORANGE_BOOK_2048, // CCSDS Orange Book 131.1-O-2 LDPC n=2048
  IEEE_802_11N_QCLDPC_648_R_1_2             = 36, //ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_648_R_1_2, // IEEE 802.11n QC-LDPC n=648 rate 1/2
  IEEE_802_11N_QCLDPC_648_R_2_3             = 37, //ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_648_R_2_3, // IEEE 802.11n QC-LDPC n=648 rate 2/3
  IEEE_802_11N_QCLDPC_648_R_3_4             = 38, //ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_648_R_3_4, // IEEE 802.11n QC-LDPC n=648 rate 3/4
  IEEE_802_11N_QCLDPC_648_R_5_6             = 39, //ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_648_R_5_6, // IEEE 802.11n QC-LDPC n=648 rate 5/6
  IEEE_802_11N_QCLDPC_1296_R_1_2            = 40, //rrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1296_R_1_2, // IEEE 802.11n QC-LDPC n=1296 rate 1/2
  IEEE_802_11N_QCLDPC_1296_R_2_3            = 41, //ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1296_R_2_3, // IEEE 802.11n QC-LDPC n=1296 rate 2/3
  IEEE_802_11N_QCLDPC_1296_R_3_4            = 42, //ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1296_R_3_4, // IEEE 802.11n QC-LDPC n=1296 rate 3/4
  IEEE_802_11N_QCLDPC_1296_R_5_6            = 43, //ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1296_R_5_6, // IEEE 802.11n QC-LDPC n=1296 rate 5/6
  IEEE_802_11N_QCLDPC_1944_R_1_2            = 44, //ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1944_R_1_2, // IEEE 802.11n QC-LDPC n=1944 rate 1/2
  IEEE_802_11N_QCLDPC_1944_R_2_3            = 45, //ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1944_R_2_3, // IEEE 802.11n QC-LDPC n=1944 rate 2/3
  IEEE_802_11N_QCLDPC_1944_R_3_4            = 46, //ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1944_R_3_4, // IEEE 802.11n QC-LDPC n=1944 rate 3/4
  IEEE_802_11N_QCLDPC_1944_R_5_6            = 47, //ErrorCorrection::ErrorCorrectionScheme::IEEE_802_11N_QCLDPC_1944_R_5_6, // IEEE 802.11n QC-LDPC n=1944 rate 5/6

  NO_FEC                                    = 48, //ErrorCorrection::ErrorCorrectionScheme::NO_FEC, // No FEC

  LAST                                      = 49, //ErrorCorrection::ErrorCorrectionScheme::LAST // This gets changes if we add more schemes

  ERROR_CORRECTION_SCHEME_BAD_WRAPPER_CONTEXT = 100 // needed for wrapper existance checking

} error_correction_scheme_t;

#ifdef __cplusplus
}
#endif

#endif /* EX2_SDR_WRAPPER_ERROR_CORRECTION_H_ */
