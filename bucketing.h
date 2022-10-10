#ifndef DVCBUCKETING_BUCKETING_H
#define DVCBUCKETING_BUCKETING_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wasm.h>
#include <wasmtime.h>
#include <stdbool.h>
#include <time.h>

// WASM Binary Data Include -- See scripts/get-bucketing-lib.sh
#include "lib/bucketing-lib.release.wasm.h"

void initialize();

void cleanup();

void init_event_queue(char *envKey, char *options);

unsigned char *flush_event_queue(char *envKey);

void on_payload_success(char *envKey, char *payloadId);

void on_payload_failure(char *envKey, char *payloadId, bool retryable);

unsigned char *generate_bucketed_config(char *envKey, char *user);

int event_queue_size(char *envKey);

void queue_event(char *envKey, char *user, char *eventString);

void queue_aggregate_event(char *envKey, char *user, char *eventString);

void store_config(char *envKey, char *config);

void set_platform_data(char *platformData);

static wasmtime_val_t new_asc_string_param(char *string);

static int asc_malloc(unsigned long length);

static int new_asc_string(const char *data, unsigned long len);

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


// We have to extend the builtins that wasm provides since abort is a 4:0 function.
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

#endif //DVCBUCKETING_BUCKETING_H
