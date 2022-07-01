#include <stddef.h>
#include <stdint.h>
#include "osal.h"

#ifdef OS_POSIX

#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include "pthread_queue.h"

void* os_malloc(size_t size) {
	return malloc(size);
}

void os_free(void *ptr) {
    if (ptr) free(ptr);
}

void os_sleep_ms(uint32_t time_ms) {
	struct timespec req, rem;
	req.tv_sec = (time_ms / 1000U);
	req.tv_nsec = ((time_ms % 1000U) * 1000000U);

	while ((nanosleep(&req, &rem) < 0) && (errno == EINTR)) {
		req = rem;
	}
}

os_queue_handle_t os_queue_create(int length, size_t item_size) {
	return pthread_queue_create(length, item_size);
}

int os_queue_enqueue(os_queue_handle_t handle, const void *value) {
	return pthread_queue_enqueue(handle, value, 0);
}

int os_queue_dequeue(os_queue_handle_t handle, void *buf) {
  return pthread_queue_dequeue(handle, buf, (uint32_t) -1);
}

int os_task_create(os_task_func_t routine, const char * const task_name, unsigned int stack_size, void * parameters, unsigned int priority, os_task_handle_t * return_handle) {

	pthread_attr_t attributes;
	if (pthread_attr_init(&attributes) != 0) {
		return ENOMEM;
	}
	// if stack size is 0, use default stack size
	if (stack_size) {
		unsigned int min_stack_size = PTHREAD_STACK_MIN;// use at least one memory
		while (min_stack_size < stack_size) { // must reach at least the provided size
			min_stack_size += PTHREAD_STACK_MIN;// keep memory page boundary (some systems may fail otherwise))
		}
		pthread_attr_setstacksize(&attributes, min_stack_size);
	}
	pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);// no need to join with thread to free its resources

	pthread_t handle;
	int return_code = pthread_create(&handle, &attributes, routine, parameters);
	pthread_attr_destroy(&attributes);

	if (return_code != 0) {
		return ENOMEM;
	}
	if (return_handle) {
		*return_handle = (os_task_handle_t *)handle;
	}

	return 0;
}

#elif defined(OS_FREERTOS)

#include <FreeRTOS.h>
#include <os_queue.h>
#include <os_task.h>
#include "osal.h"

void* os_malloc(size_t size) {
	return pvPortMalloc(size);
}

void os_free(void *ptr) {
    if (ptr) vPortFree(ptr);
}

void os_sleep_ms(uint32_t time_ms) {
	vTaskDelay(time_ms / portTICK_RATE_MS);
}

#define QUEUE_NO_WAIT 0

os_queue_handle_t os_queue_create(int len, size_t item_size) {
	return xQueueCreate(len, item_size);
}

int os_queue_enqueue(os_queue_handle_t handle, const void* value) {
	return xQueueSendToBack(handle, value, QUEUE_NO_WAIT);
}

int os_queue_dequeue(os_queue_handle_t handle, void* buf) {
	return xQueueReceive(handle, buf, (uint32_t) -1);
}

int os_task_create(os_task_func_t routine, const char * const task_name, unsigned int stack_size, void * parameters, unsigned int priority, os_task_handle_t * return_handle) {
    os_task_handle_t handle;
	int ret = xTaskCreate(routine, task_name, stack_size, parameters, priority, &handle);

	if (return_handle) {
		*return_handle = handle;
	}
	return (ret == 1)? 0 : ret;
}

uint32_t os_get_ms() {
	return (uint32_t)(xTaskGetTickCount() * (1000/configTICK_RATE_HZ));
}

#endif // OS_FREERTOS
