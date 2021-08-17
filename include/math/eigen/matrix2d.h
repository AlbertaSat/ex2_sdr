/*!
 * @file matrix2d.h
 * @author Steven Knudsen
 * @date June 18, 2019
 *
 * @details
 *
 * @copyright University of Alberta 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#ifndef EX2_SDR_MATH_EIGEN_MATRIX2D_H_
#define EX2_SDR_MATH_EIGEN_MATRIX2D_H_

#include <eigen3/Eigen/Dense>

/*!
 * @brief Perform a circular rotation of first rows and then columns.
 *
 * @param[in] source The source matrix.
 * @param[in] rowsToRotate The number of rows positions to rotate.
 * @param[in] down If true, rotate rows down, else rotate up.
 * @param[in] colsToRotate The number of column positions to rotate.
 * @param[in] left If true, rotate columns to the left, else rotate right.
 * @return[inout] A copy of the circularly rotated source.
 */
template<typename M>
  M
  rotate (
    const M & source,
    unsigned int rowsToRotate = 0,
    bool down = true,
    unsigned int colsToRotate = 0,
    bool left = true)
  {
    // check for trivial shift
    if (rowsToRotate == 0 and colsToRotate == 0) return source;

    M out (source.rows (), source.cols ());

    // Handle a non-zero row rotate
    if (rowsToRotate > 0)
    {
      rowsToRotate = rowsToRotate % source.rows ();
      if (not down) rowsToRotate = source.rows () - rowsToRotate;
      unsigned int remaining_rows = source.rows () - rowsToRotate;
      out.topRows (rowsToRotate) = source.bottomRows (rowsToRotate);
      out.bottomRows (remaining_rows) = source.topRows (remaining_rows);
    }
    else
      out = source;

    if (colsToRotate > 0)
    {
      colsToRotate = colsToRotate % source.cols ();
      if (not left) colsToRotate = source.cols () - colsToRotate;
      unsigned int remaining_cols = source.cols () - colsToRotate;

      M temp (out.rows (), out.cols ());
      temp = out;

      out.rightCols (colsToRotate) = temp.leftCols (colsToRotate);
      out.leftCols (remaining_cols) = temp.rightCols (remaining_cols);
    }

    return out;
  }

#endif /* EX2_SDR_MATH_EIGEN_MATRIX2D_H_ */
