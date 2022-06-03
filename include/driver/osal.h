#ifndef PORTING_H_
#define PORTING_H_

#ifdef __cplusplus
extern "C" {
#endif

void* os_malloc(size_t size);
void os_free(void *ptr);

void os_sleep_ms(uint32_t time_ms);

typedef void* os_queue_handle_t;

os_queue_handle_t os_queue_create(int length, size_t item_size);

int os_queue_enqueue(os_queue_handle_t handle, const void * value);

int os_queue_dequeue(os_queue_handle_t handle, void* buf);

typedef void* os_task_handle_t;
typedef void os_task_return_t;

typedef os_task_return_t (*os_task_func_t)(void* parameter);

#define OS_RX_TASK_STACK_SIZE 512

int os_task_create(os_task_func_t func, const char *const name, unsigned int stack_size, void *parameter, unsigned int priority, os_task_handle_t *handle);

#ifdef __cplusplus
}
#endif
#endif // OSAL_H_
