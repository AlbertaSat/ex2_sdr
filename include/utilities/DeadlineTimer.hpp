/*!
 * @file DeadlineTimer.hpp
 * @author Steven Knudsen
 * @date Aug. 19, 2022
 *
 * @details A general deadline timer with callback.
 *
 * @copyright University of Alberta, 2022
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */
#ifndef EX2_SDR_UTILITIES_DEADLINETIMER_HPP_
#define EX2_SDR_UTILITIES_DEADLINETIMER_HPP_

#include <stdint.h>
#include "utilities/os_support.h"
#include <boost/asio.hpp>

//#ifdef OS_POSIX
//#include <boost/asio.hpp>
//#else
//
//#ifdef OS_FREERTOS
//  void start(uint16_t timeout_in_ms, void (*timer_callback_f)(void));
//#else
//#error No OS defined
//#endif
//
//#endif

class DeadlineTimer
{
public:
  DeadlineTimer ();
  virtual
  ~DeadlineTimer ();

  /*!
   * @brief A type definition for the timer callback function based on OS
   */
#ifdef OS_POSIX
  typedef std::function< void(boost::system::error_code) > callback_function_t;
#else
#ifdef OS_FREERTOS
  typedef std::function< void(void) > callback_function_t;
#endif
#endif

  /*!
   * @brief Accessor to set the callback function
   *
   * @param[in] cfunc The callback function with signature matching @p callback_function_t
   */
  void setCallback(callback_function_t func);

  /*!
   * @brief Start the timer
   *
   * @param[in] timeout_in_ms The number of milliseconds in [0,65535]
   */
  void start(uint16_t timeout_in_ms);

  /*!
   * @brief Stop the timer
   *
   * @details It is always possible there will be a race between the invocation
   * of this method and actually stopping the timer. That is, call @p stop too
   * close to the timer expiry and it may just expire. For that reason, this
   * method also sets the state of the deadline timer to "was stopped" so that
   * the if the callback is invoked by mistake, it can check to see if it was
   * stopped and act accordingly
   */
  void stop();

  /*!
   * @brief Returnt the stopped status of the timer
   * @return true if @p stop was invoked after @p start was invoked
   */
  bool isStopped();

#ifdef OS_POSIX
  /*!
   * @brief The signed difference in microseconds between two times.
   * @param current_time
   * @param previous_time
   * @return current_time - previous_time in microseconds
   */
  static int32_t time_diff(
    struct timeval *current_time,
    struct timeval *previous_time);
#else
#ifdef OS_FREERTOS
  // @TODO add whatever is needed here
#endif
#endif

private:

#ifdef OS_POSIX
  // Need an io_service object
  boost::asio::io_service m_io;

  // Timer object
  boost::asio::deadline_timer *m_deadline_timer;
#else
#ifdef OS_FREERTOS
  // @TODO add whatever is needed here
#endif
#endif

  // Function pointer for callback
  callback_function_t m_timer_callback_f;

  bool m_was_stopped;
};

#endif /* EX2_SDR_UTILITIES_DEADLINETIMER_HPP_ */
