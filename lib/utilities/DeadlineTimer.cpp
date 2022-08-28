/*!
 * @file DeadlineTimer.cpp
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
#include "DeadlineTimer.hpp"

#ifdef OS_POSIX
#include <functional>
#else
#ifdef OS_FREERTOS
  // @TODO Any includes needed for FreeRTOS
#endif
#endif

// Establish a templated class (struct) that we can use to cast our desired
// MPDUTimer callback to a C style function pointer
template <typename T>
struct Callback;

template <typename ReturnVal, typename... Params>
struct Callback<ReturnVal(Params...)> {
  template<typename... Args>
  static ReturnVal callback(Args... args) {
    return func(args...);
  }
  static std::function<ReturnVal(Params...)> func;
};

template <typename ReturnVal, typename... Params>
std::function<ReturnVal(Params...)> Callback<ReturnVal(Params...)>::func;

DeadlineTimer::DeadlineTimer ()
{
  m_deadline_timer = NULL;
  m_timer_callback_f = NULL;
  m_was_stopped = false;
}

DeadlineTimer::~DeadlineTimer ()
{
#ifdef OS_POSIX
    if (m_deadline_timer != NULL) {
      delete m_deadline_timer;
    }
#else
#ifdef OS_FREERTOS
  // @TODO clean up
#endif
#endif
}

void
DeadlineTimer::setCallback(callback_function_t cfunc) {
  m_timer_callback_f = cfunc;
}

void
DeadlineTimer::start(uint16_t timeout_in_ms)
{
  m_was_stopped = false;
#ifdef OS_POSIX
  m_deadline_timer = new boost::asio::deadline_timer(m_io, boost::posix_time::milliseconds(timeout_in_ms));
  m_deadline_timer->async_wait(m_timer_callback_f);
  m_io.run();
  if (m_deadline_timer) {
    delete m_deadline_timer;
    m_deadline_timer = NULL;
  }
#else
#ifdef OS_FREERTOS
  void start(uint16_t timeout_in_ms, void (*timer_callback_f)(void))
  {
    // @TODO add whatever is needed here
  }
#endif
#endif
}

void
DeadlineTimer::stop()
{
  m_was_stopped = true;
#ifdef OS_POSIX
  m_deadline_timer->cancel();
  if (m_deadline_timer) {
    delete m_deadline_timer;
    m_deadline_timer = NULL;
  }
#else
#ifdef OS_FREERTOS
  // @TODO add whatever is needed here
#endif
#endif
}

bool
DeadlineTimer::isStopped() {
  return m_was_stopped;
}

#ifdef OS_POSIX
int32_t
DeadlineTimer::time_diff (
  struct timeval *current_time,
  struct timeval *previous_time)
{
  uint64_t c = (uint64_t) current_time->tv_sec * 1000000
      + (uint64_t) current_time->tv_usec;
  uint64_t p = (uint64_t) previous_time->tv_sec * 1000000
      + (uint64_t) previous_time->tv_usec;
  return c - p;
}
#else
#ifdef OS_FREERTOS
// @TODO add whatever is needed here
#endif
#endif

