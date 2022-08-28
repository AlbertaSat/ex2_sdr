/*!
 * @file os_support.h
 * @author Steven Knudsen
 * @date Aug. 19, 2022
 *
 * @details Some useful definitions to help support POSIX and FreeRTOS OSes
 *
 * @copyright University of Alberta, 2022
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */
#ifndef EX2_SDR_UTILITIES_OS_SUPPORT_H_
#define EX2_SDR_UTILITIES_OS_SUPPORT_H_

#if !defined(OS_FREERTOS) && !defined(OS_POSIX)
#define OS_POSIX
#endif


#endif /* EX2_SDR_UTILITIES_OS_SUPPORT_H_ */
