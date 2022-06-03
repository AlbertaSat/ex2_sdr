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
