/*!
 * @file qa_golay.cpp
 * @author Steven Knudsen
 * @date May 28, 2021
 *
 * @details Unit test for the golay codec.
 *
 * This unit test exercises the golay codec.
 *
 * @copyright AlbertaSat 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */

#include <cstdio>
#include <iostream>
#include <random>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "csp.h"
#include "csp_types.h"
#include "csp_buffer.h"
#include "src/drivers/sdr/circular_buffer.h"

#ifdef __cplusplus
}
#endif

using namespace std;

#include "gtest/gtest.h"



/*!
 * @brief Test creation, read, update, destroy
 */
TEST(circular_buffer, CRUD )
{

#endif
      // Check decoded is same as data
      ASSERT_TRUE(data == decoded) << "Oops, Golay failed!";
    } // num trials
  } // 1, 2, and 3 bits in error
}

