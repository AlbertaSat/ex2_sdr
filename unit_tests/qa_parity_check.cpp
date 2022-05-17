/*!
 * @file qa_parity_check.cpp
 * @author Steven Knudsen
 * @date August 8, 2019
 *
 * @details Unit test for the LDPC parity check matrix.
 *
 * This unit tests the ParityCheck class.
 *
 * @copyright Xiphos Systems Corp. 2019
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#include <cstdio>
#include <cstdint>
#include <iostream>
#include <random>
#include <time.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <eigen3/Eigen/Dense>

#include "error_correction.h"
#include "parity_check.h"
#include "ppdu_u8.h"

using namespace std;
using namespace xiphos::darkstar;
namespace fs = boost::filesystem;

#include "gtest/gtest.h"

#define PARITY_CHECK_REF_H648_12_FILENAME  "H648_12.txt"
#define PARITY_CHECK_REF_H648_23_FILENAME  "H648_23.txt"
#define PARITY_CHECK_REF_H648_34_FILENAME  "H648_34.txt"
#define PARITY_CHECK_REF_H648_56_FILENAME  "H648_56.txt"
#define PARITY_CHECK_REF_H1296_12_FILENAME "H1296_12.txt"
#define PARITY_CHECK_REF_H1296_23_FILENAME "H1296_23.txt"
#define PARITY_CHECK_REF_H1296_34_FILENAME "H1296_34.txt"
#define PARITY_CHECK_REF_H1296_56_FILENAME "H1296_56.txt"
#define PARITY_CHECK_REF_H1944_12_FILENAME "H1944_12.txt"
#define PARITY_CHECK_REF_H1944_23_FILENAME "H1944_23.txt"
#define PARITY_CHECK_REF_H1944_34_FILENAME "H1944_34.txt"
#define PARITY_CHECK_REF_H1944_56_FILENAME "H1944_56.txt"


static inline std::string
parity_check_data_path(const char *local_path)
{
  // get the current directory
  std::string s_cwd (getcwd (NULL, 0));
  // make into a path.
  fs::path b_cwd (s_cwd);

  // form the local path
  fs::path lp (local_path);

  // Form the absolute path to the unit tests directory. We assume this
  // test executes in the "build" directory, so go up one level before
  // forming the path.
  fs::path uhd_jitc_unittests_path = b_cwd.parent_path() / fs::path("unit_tests");
  fs::path parity_check_data_path = uhd_jitc_unittests_path / fs::path("parity_check_matrices");

  // form the absolute path and return
  fs::path abs_path = parity_check_data_path / lp;
  return abs_path.string ();
}
bool
compare_parity_check_matrix (
  Eigen::MatrixXi & H,
  const char * ref_H_filename)
{
  bool okay = false;

  fs::path h_path = parity_check_data_path (ref_H_filename);

  Eigen::MatrixXi ref_H;
  unsigned int rows = H.rows ();
  unsigned int cols = H.cols ();
  ref_H = Eigen::MatrixXi (rows, cols);

  // read in the reference matrix
  if (h_path.is_complete () and fs::is_regular_file (h_path)
  and fs::exists (h_path))
  {
    std::string line;
    fs::fstream infile (h_path);

    unsigned int row_count = 0;
    bool col_count_okay = true;
    while (std::getline (infile, line))  // this does the checking!
    {
      if (row_count >= rows)
        throw std::runtime_error (
          (boost::format ("qa_parity_check: %s has wrong number of rows.")
      % h_path.c_str ()).str ());

      std::vector<std::string> dataLine;
      boost::trim_if (line, boost::is_any_of ("\t "));
      boost::split (dataLine, line, boost::is_any_of ("\t "),
        boost::token_compress_on);

      col_count_okay = col_count_okay and (dataLine.size () == cols);
      if (!col_count_okay)
        throw std::runtime_error (
          (boost::format ("qa_parity_check: %s has wrong number of cols.")
      % h_path.c_str ()).str ());
      int n, j = 0;
      BOOST_FOREACH( std::string & s, dataLine )
      {
        std::istringstream (s) >> n;
        ref_H (row_count, j++) = n;
      }
      row_count++;
    }
    infile.close ();
    if (row_count != rows or !col_count_okay)
      throw std::runtime_error (
        (boost::format (
          "qa_parity_check: %s has bad contents or is wrong for this test.")
    % h_path.c_str ()).str ());

    // Finished reading in reference matrix, time to compare.
    okay = (ref_H.rows () == rows and ref_H.cols () == cols);
    if (okay)
    {
      okay = ((H - ref_H).norm () == 0);

    }
  }
  else
  {
    throw std::runtime_error (
      (boost::format ("qa_parity_check: Unable to find %s") % h_path).str ());
  }

  return okay;
}

void
check_parity_matrix (
  Eigen::MatrixXi & H,
  ErrorCorrection::ErrorCorrectionCoding errorCorrectionCoding,
  ErrorCorrection::CodingRate codingRate,
  const char *ref_parity_check_filename)
{
  bool okay = false;
  ErrorCorrection ec(errorCorrectionCoding, codingRate);
  try
  {
    okay = compare_parity_check_matrix (H, ref_parity_check_filename);
    ASSERT_TRUE(okay) << (boost::format (
      "Parity-check matrix does not match expected for n=%d r=%s.")
    % ec.getCodewordLen() % ErrorCorrection::CodingRateName(codingRate)).str () << std::endl;
  }
  catch (std::runtime_error& re)
  {
    cout << re.what() << endl;
    GTEST_FAIL() << (boost::format ("Runtime exception for n=%d r=%s.")
    % ec.getCodewordLen() % ErrorCorrection::CodingRateName(codingRate)).str () << std::endl;
  }
}

/*!
 * @brief Test constructor for one LDPC configuratin
 */
TEST(ParityCheck, Constructor )
{
  ParityCheck *pchk = new ParityCheck (ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_648, ErrorCorrection::CodingRate::RATE_1_2);
  ASSERT_TRUE(pchk->isValid());
}

/*!
 * @brief Test validity of each IEEE codeword size and rate combination.
 *
 * @details This also tests the @p rank and @p parityCheckMatrix accessors
 *
 * @todo Add all other LDPC variants
 *
 */
TEST(ParityCheck, LDPC_IEEE_ALL )
{
  // n = 648 r = 1/2
  ParityCheck pc(ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_648, ErrorCorrection::CodingRate::RATE_1_2);
  ASSERT_FALSE(pc.rank() == 0) << "Rank is zero" << endl;
  check_parity_matrix (pc.parityCheckMatrix(), ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_648,
    ErrorCorrection::CodingRate::RATE_1_2, PARITY_CHECK_REF_H648_12_FILENAME);

  // n = 648 r = 2/3
  pc = ParityCheck(ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_648, ErrorCorrection::CodingRate::RATE_2_3);
  ASSERT_FALSE(pc.rank() == 0) << "Rank is zero" << endl;
  check_parity_matrix (pc.parityCheckMatrix(), ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_648,
    ErrorCorrection::CodingRate::RATE_2_3, PARITY_CHECK_REF_H648_23_FILENAME);

  // n = 648 r = 3/4
  pc = ParityCheck(ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_648, ErrorCorrection::CodingRate::RATE_3_4);
  ASSERT_FALSE(pc.rank() == 0) << "Rank is zero" << endl;
  check_parity_matrix (pc.parityCheckMatrix(), ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_648,
    ErrorCorrection::CodingRate::RATE_3_4, PARITY_CHECK_REF_H648_34_FILENAME);

  // n = 648 r = 5/6
  pc = ParityCheck(ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_648, ErrorCorrection::CodingRate::RATE_5_6);
  ASSERT_FALSE(pc.rank() == 0) << "Rank is zero" << endl;
  check_parity_matrix (pc.parityCheckMatrix(), ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_648,
    ErrorCorrection::CodingRate::RATE_5_6, PARITY_CHECK_REF_H648_56_FILENAME);

  // n = 1296 r = 1/2
  pc = ParityCheck(ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1296, ErrorCorrection::CodingRate::RATE_1_2);
  ASSERT_FALSE(pc.rank() == 0) << "Rank is zero" << endl;
  check_parity_matrix (pc.parityCheckMatrix(), ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1296,
    ErrorCorrection::CodingRate::RATE_1_2, PARITY_CHECK_REF_H1296_12_FILENAME);

  // n = 1296 r = 2/3
  pc = ParityCheck(ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1296, ErrorCorrection::CodingRate::RATE_2_3);
  ASSERT_FALSE(pc.rank() == 0) << "Rank is zero" << endl;
  check_parity_matrix (pc.parityCheckMatrix(), ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1296,
    ErrorCorrection::CodingRate::RATE_2_3, PARITY_CHECK_REF_H1296_23_FILENAME);

  // n = 1296 r = 3/4
  pc = ParityCheck(ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1296, ErrorCorrection::CodingRate::RATE_3_4);
  ASSERT_FALSE(pc.rank() == 0) << "Rank is zero" << endl;
  check_parity_matrix (pc.parityCheckMatrix(), ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1296,
    ErrorCorrection::CodingRate::RATE_3_4, PARITY_CHECK_REF_H1296_34_FILENAME);

  // n = 1296 r = 5/6
  pc = ParityCheck(ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1296, ErrorCorrection::CodingRate::RATE_5_6);
  ASSERT_FALSE(pc.rank() == 0) << "Rank is zero" << endl;
  check_parity_matrix (pc.parityCheckMatrix(), ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1296,
    ErrorCorrection::CodingRate::RATE_5_6, PARITY_CHECK_REF_H1296_56_FILENAME);

  // n = 1944 r = 1/2
  pc =ParityCheck(ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1944, ErrorCorrection::CodingRate::RATE_1_2);
  ASSERT_FALSE(pc.rank() == 0) << "Rank is zero" << endl;
  check_parity_matrix (pc.parityCheckMatrix(), ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1944,
    ErrorCorrection::CodingRate::RATE_1_2, PARITY_CHECK_REF_H1944_12_FILENAME);

  // n = 1944 r = 2/3
  pc = ParityCheck(ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1944, ErrorCorrection::CodingRate::RATE_2_3);
  ASSERT_FALSE(pc.rank() == 0) << "Rank is zero" << endl;
  check_parity_matrix (pc.parityCheckMatrix(), ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1944,
    ErrorCorrection::CodingRate::RATE_2_3, PARITY_CHECK_REF_H1944_23_FILENAME);

  // n = 1944 r = 3/4
  pc = ParityCheck(ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1944, ErrorCorrection::CodingRate::RATE_3_4);
  ASSERT_FALSE(pc.rank() == 0) << "Rank is zero" << endl;
  check_parity_matrix (pc.parityCheckMatrix(), ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1944,
    ErrorCorrection::CodingRate::RATE_3_4, PARITY_CHECK_REF_H1944_34_FILENAME);

  // n = 1944 r = 5/6
  pc = ParityCheck(ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1944, ErrorCorrection::CodingRate::RATE_5_6);
  ASSERT_FALSE(pc.rank() == 0) << "Rank is zero" << endl;
  check_parity_matrix (pc.parityCheckMatrix(), ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1944,
    ErrorCorrection::CodingRate::RATE_5_6, PARITY_CHECK_REF_H1944_56_FILENAME);
}

/*!
 * @brief Test the @ prototypeMatrixSize method.
 *
 * @todo This is currently only valid for IEEE LDPC
 */
TEST(ParityCheck, PrototypeMatrixSize )
{
  ParityCheck pc(ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_648, ErrorCorrection::CodingRate::RATE_1_2);
  ASSERT_TRUE(pc.prototypeMatrixSize() == 27);

  pc = ParityCheck(ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1296, ErrorCorrection::CodingRate::RATE_1_2);
  ASSERT_TRUE(pc.prototypeMatrixSize() == 54);

  pc = ParityCheck(ErrorCorrection::ErrorCorrectionCoding::LDPC_IEEE_1944, ErrorCorrection::CodingRate::RATE_1_2);
  ASSERT_TRUE(pc.prototypeMatrixSize() == 81);
}
