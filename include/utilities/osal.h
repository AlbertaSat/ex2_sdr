#ifndef OSAL_H_
#define OSAL_H_

#include <stdint.h>
#include <sdr_config.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(OS_FREERTOS) && !defined(OS_POSIX)
#define OS_POSIX
#endif

/**
   No error.
   @note Value is 1!
*/
#define OS_QUEUE_OK 1

/**
   Queue full.
   @note Value is 0!
*/
#define OS_QUEUE_FULL 0

/**
   Queue error.
   @note Value is 0!
*/
#define OS_QUEUE_ERROR 0

void* os_malloc(size_t size);
void os_free(void *ptr);

void os_sleep_ms(uint32_t time_ms);

typedef void* os_queue_handle_t;

os_queue_handle_t os_queue_create(int length, size_t item_size);

int os_queue_enqueue(os_queue_handle_t handle, const void * value);

int os_queue_dequeue(os_queue_handle_t handle, void* buf, uint32_t timeout);

typedef void* os_task_handle_t;

#if defined(OS_POSIX)
#  define OS_MAX_TIMEOUT (UINT32_MAX)
#  define OS_RX_TASK_STACK_SIZE 1024
#  define OS_TickType long
#  define ex2_log printf

typedef void* os_task_return_t;

#elif defined(OS_FREERTOS)
#include "FreeRTOS.h"
#  define OS_MAX_TIMEOUT portMAX_DELAY
#  define OS_RX_TASK_STACK_SIZE 1512/sizeof(int) // FreeRTOS allocates stack sizes based of words, not bytes
#  define OS_TickType TickType_t

typedef void os_task_return_t;

#else
#error "No OS specified"
#endif /* OS_FREERTOS */

typedef os_task_return_t (*os_task_func_t)(void* parameter);

int os_task_create(os_task_func_t func, const char *const name, unsigned int stack_size, void *parameter, unsigned int priority, os_task_handle_t *handle);

OS_TickType os_get_ms(void);
#ifdef __cplusplus
}
#endif
#endif // OSAL_H_
