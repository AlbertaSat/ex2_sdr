/*!
 * @file qa_maxtrix2d.cpp
 * @author Steven Knudsen
 * @date Dec 17, 2019
 *
 * @details Unit test for the matrix2d util.
 *
 * @copyright University of Alberta 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#include <cstdio>
#include <cstdint>
#include <iostream>

#include "math/eigen/matrix2d.h"

#include "gtest/gtest.h"

using namespace std;

/*!
 * @brief Test column right rotation of matrices
 */
TEST(Matrix2DTest, RotateMatrixColRight )
{
  uint32_t const matrixSize = 10;

  // create the test matrix
  Eigen::MatrixXi A = Eigen::MatrixXi::Identity(matrixSize, matrixSize);

  // create the refernce matrix for a rotation of 1
  Eigen::MatrixXi refA(matrixSize, matrixSize);
  refA <<
      0,1,0,0,0,0,0,0,0,0,
      0,0,1,0,0,0,0,0,0,0,
      0,0,0,1,0,0,0,0,0,0,
      0,0,0,0,1,0,0,0,0,0,
      0,0,0,0,0,1,0,0,0,0,
      0,0,0,0,0,0,1,0,0,0,
      0,0,0,0,0,0,0,1,0,0,
      0,0,0,0,0,0,0,0,1,0,
      0,0,0,0,0,0,0,0,0,1,
      1,0,0,0,0,0,0,0,0,0;

  //  cout << refA << endl;
  // create the reference matrix for a rotation of 9 more
  Eigen::MatrixXi fullRotation = Eigen::MatrixXi::Identity(matrixSize, matrixSize);

  // Rotate right by 1 column
  Eigen::MatrixXi A1 = rotate<Eigen::MatrixXi> (A,0,false,1,false);

  bool same = true;
  for (uint32_t i = 0; i < matrixSize; i++) {
    for (uint32_t j = 0; j < matrixSize; j++) {
      same = same & (A1(i,j) == refA(i,j));
    }
  }
  ASSERT_TRUE(same) << "Column rotation of 1 right failed";

  // Rotate right by 9 columns
  Eigen::MatrixXi A10 = rotate<Eigen::MatrixXi> (A1,0,false,9,false);

  same = true;
  for (uint32_t i = 0; i < matrixSize; i++) {
    for (uint32_t j = 0; j < matrixSize; j++) {
      same = same & (A10(i,j) == fullRotation(i,j));
    }
  }
  ASSERT_TRUE(same) << "Column rotation of 9 right failed";
}

/*!
 * @brief Test column right rotation of matrices
 */
TEST(Matrix2DTest, RotateMatrixColLeft )
{
  uint32_t const matrixSize = 10;

  // create the test matrix
  Eigen::MatrixXi A = Eigen::MatrixXi::Identity(matrixSize, matrixSize);

  // create the refernce matrix for a rotation of 1
  Eigen::MatrixXi refA(matrixSize, matrixSize);
  refA <<
      0,0,0,0,0,0,0,0,0,1,
      1,0,0,0,0,0,0,0,0,0,
      0,1,0,0,0,0,0,0,0,0,
      0,0,1,0,0,0,0,0,0,0,
      0,0,0,1,0,0,0,0,0,0,
      0,0,0,0,1,0,0,0,0,0,
      0,0,0,0,0,1,0,0,0,0,
      0,0,0,0,0,0,1,0,0,0,
      0,0,0,0,0,0,0,1,0,0,
      0,0,0,0,0,0,0,0,1,0;

  //  cout << refA << endl;
  // create the reference matrix for a rotation of 9 more
  Eigen::MatrixXi fullRotation = Eigen::MatrixXi::Identity(matrixSize, matrixSize);

  // Rotate right by 1 column
  Eigen::MatrixXi A1 = rotate<Eigen::MatrixXi> (A,0,false,1,true);

  bool same = true;
  for (uint32_t i = 0; i < matrixSize; i++) {
    for (uint32_t j = 0; j < matrixSize; j++) {
      same = same & (A1(i,j) == refA(i,j));
    }
  }
  ASSERT_TRUE(same) << "Column rotation of 1 left failed";

  // Rotate right by 9 columns
  Eigen::MatrixXi A10 = rotate<Eigen::MatrixXi> (A1,0,false,9,true);

  same = true;
  for (uint32_t i = 0; i < matrixSize; i++) {
    for (uint32_t j = 0; j < matrixSize; j++) {
      same = same & (A10(i,j) == fullRotation(i,j));
    }
  }
  ASSERT_TRUE(same) << "Column rotation of 9 left failed";
}

/*!
 * @brief Test row up rotation of matrices
 */
TEST(Matrix2DTest, RotateMatrixRowUp )
{
  uint32_t const matrixSize = 10;

  // create the test matrix
  Eigen::MatrixXi A = Eigen::MatrixXi::Identity(matrixSize, matrixSize);

  // create the refernce matrix for a rotation of 1
  Eigen::MatrixXi refA(matrixSize, matrixSize);
  refA <<
      0,0,1,0,0,0,0,0,0,0,
      0,0,0,1,0,0,0,0,0,0,
      0,0,0,0,1,0,0,0,0,0,
      0,0,0,0,0,1,0,0,0,0,
      0,0,0,0,0,0,1,0,0,0,
      0,0,0,0,0,0,0,1,0,0,
      0,0,0,0,0,0,0,0,1,0,
      0,0,0,0,0,0,0,0,0,1,
      1,0,0,0,0,0,0,0,0,0,
      0,1,0,0,0,0,0,0,0,0;

//  cout << refA << endl;
  // create the reference matrix for a rotation of 9 more
  Eigen::MatrixXi fullRotation = Eigen::MatrixXi::Identity(matrixSize, matrixSize);

  // Rotate up by 2 rows
  Eigen::MatrixXi A1 = rotate<Eigen::MatrixXi> (A,2,false,0,false);
//  cout << A1 << endl;

    bool same = true;
    for (uint32_t i = 0; i < matrixSize; i++) {
      for (uint32_t j = 0; j < matrixSize; j++) {
        same = same & (A1(i,j) == refA(i,j));
      }
    }
    ASSERT_TRUE(same) << "Row rotation of 2 up failed";

    // Rotate up by 8 columns
    Eigen::MatrixXi A10 = rotate<Eigen::MatrixXi> (A1,8,false,0,false);

    same = true;
    for (uint32_t i = 0; i < matrixSize; i++) {
      for (uint32_t j = 0; j < matrixSize; j++) {
        same = same & (A10(i,j) == fullRotation(i,j));
      }
    }
    ASSERT_TRUE(same) << "Row rotation of 9 up failed";
}

/*!
 * @brief Test row down rotation of matrices
 */
TEST(Matrix2DTest, RotateMatrixRowDown )
{
  uint32_t const matrixSize = 10;

  // create the test matrix
  Eigen::MatrixXi A = Eigen::MatrixXi::Identity(matrixSize, matrixSize);

  // create the refernce matrix for a rotation of 1
  Eigen::MatrixXi refA(matrixSize, matrixSize);
  refA <<
      0,0,0,0,0,0,0,0,1,0,
      0,0,0,0,0,0,0,0,0,1,
      1,0,0,0,0,0,0,0,0,0,
      0,1,0,0,0,0,0,0,0,0,
      0,0,1,0,0,0,0,0,0,0,
      0,0,0,1,0,0,0,0,0,0,
      0,0,0,0,1,0,0,0,0,0,
      0,0,0,0,0,1,0,0,0,0,
      0,0,0,0,0,0,1,0,0,0,
      0,0,0,0,0,0,0,1,0,0;

//  cout << refA << endl;
  // create the reference matrix for a rotation of 9 more
  Eigen::MatrixXi fullRotation = Eigen::MatrixXi::Identity(matrixSize, matrixSize);

  // Rotate down by 2 rows
  Eigen::MatrixXi A1 = rotate<Eigen::MatrixXi> (A,2,true,0,false);
//  cout << A1 << endl;

    bool same = true;
    for (uint32_t i = 0; i < matrixSize; i++) {
      for (uint32_t j = 0; j < matrixSize; j++) {
        same = same & (A1(i,j) == refA(i,j));
      }
    }
    ASSERT_TRUE(same) << "Row rotation of 2 down failed";

    // Rotate down by 8 columns
    Eigen::MatrixXi A10 = rotate<Eigen::MatrixXi> (A1,8,true,0,false);

    same = true;
    for (uint32_t i = 0; i < matrixSize; i++) {
      for (uint32_t j = 0; j < matrixSize; j++) {
        same = same & (A10(i,j) == fullRotation(i,j));
      }
    }
    ASSERT_TRUE(same) << "Row rotation of 9 down failed";
}
