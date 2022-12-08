#ifndef DVCBUCKETING_BUCKETING_INTERNAL_H
#define DVCBUCKETING_BUCKETING_INTERNAL_H

#include <wasmtime.h>
#include <wasm.h>

#define DVCBUCKETING_ASC_NEW "__new"
#define DVCBUCKETING_SETPLATFORMDATA "setPlatformData"
#define DVCBUCKETING_SETCONFIGDATA "setConfigData"
#define DVCBUCKETING_QUEUEAGGREGATEEVENT "queueAggregateEvent"
#define DVCBUCKETING_QUEUEEVENT "queueEvent"
#define DVCBUCKETING_ONPAYLOADFAILURE "onPayloadFailure"
#define DVCBUCKETING_ONPAYLOADSUCCESS "onPayloadSuccess"
#define DVCBUCKETING_GENERATEBUCKETEDCONFIGFORUSER "generateBucketedConfigForUser"
#define DVCBUCKETING_FLUSHEVENTQUEUE "flushEventQueue"
#define DVCBUCKETING_INITEVENTQUEUE "initEventQueue"
#define DVCBUCKETING_EVENTQUEUESIZE "eventQueueSize"

// WASM Exported Functions
static wasmtime_extern_t w_init_event_queue;
static wasmtime_extern_t w_flush_event_queue;
static wasmtime_extern_t w_on_payload_success;
static wasmtime_extern_t w_on_payload_failure;
static wasmtime_extern_t w_generate_bucketed_config;
static wasmtime_extern_t w_event_queue_size;
static wasmtime_extern_t w_queue_event;
static wasmtime_extern_t w_queue_aggregate_event;
static wasmtime_extern_t w_store_config;
static wasmtime_extern_t w_set_platform_data;
static wasmtime_extern_t w__new;


// Core WASM Components
static wasmtime_module_t *wasm_module = NULL;
static wasmtime_memory_t wasm_memory;
static wasmtime_store_t *wasm_store;
static wasm_engine_t *wasm_engine;
static wasmtime_context_t *wasm_context;
static wasmtime_instance_t wasm_instance;


// Internal functions

/**
 * new_asc_string_param returns a parsed WASM parameter - after allocating the string into WASM memory
 * as well as sets the paramter type to I32 ready for calling functions.
 * @param string
 * @return
 */
static wasmtime_val_t new_asc_string_param(const char *string);

/**
 * asc_malloc allocates memory inside of the webassembly memory using the assemblyscript w__new function.
 * @param length length of the memory to be allocated
 * @return start address inside wasm memory
 */
static int asc_malloc(unsigned long length);

/**
 * new_asc_string allocates and returns the pointer to the given string inside WASM memory.
 * @param data string to allocate into WASM memory.
 * @param len string length
 * @return WASM memory address
 */
static int new_asc_string(const char *data, unsigned long len);

/**
 * read_asc_string pulls from WASM memory the string at a given address, and returns the converted UTF-8 string.
 * @param addr WASM memory address
 * @return resulting string
 */
static unsigned char *read_asc_string(int addr);

static wasm_trap_t *env_abort_callback(void *env,
                                       wasmtime_caller_t *caller,
                                       const wasmtime_val_t *args,
                                       size_t nargs,
                                       wasmtime_val_t *results,
                                       size_t nresults);

static wasm_trap_t *env_console_log_callback(void *env,
                                             wasmtime_caller_t *caller,
                                             const wasmtime_val_t *args,
                                             size_t nargs,
                                             wasmtime_val_t *results,
                                             size_t nresults);

static wasm_trap_t *env_date_now_callback(void *env,
                                          wasmtime_caller_t *caller,
                                          const wasmtime_val_t *args,
                                          size_t nargs,
                                          wasmtime_val_t *results,
                                          size_t nresults);

static wasm_trap_t *env_seed_callback(void *env,
                                      wasmtime_caller_t *caller,
                                      const wasmtime_val_t *args,
                                      size_t nargs,
                                      wasmtime_val_t *results,
                                      size_t nresults);

static void exit_with_error(const char *message, wasmtime_error_t *error,
                            wasm_trap_t *trap);


// We have to extend the builtins that wasm/wasmtime provides since abort (an assemblyscript import) is a 4:0 function.
static inline wasm_functype_t *wasm_functype_new_4_0(
        wasm_valtype_t *p1, wasm_valtype_t *p2,
        wasm_valtype_t *p3, wasm_valtype_t *p4
) {
    wasm_valtype_t *ps[4] = {p1, p2, p3, p4};
    wasm_valtype_vec_t params, results;
    wasm_valtype_vec_new(&params, 4, ps);
    wasm_valtype_vec_new_empty(&results);
    return wasm_functype_new(&params, &results);
}
#endif //DVCBUCKETING_BUCKETING_INTERNAL_H
