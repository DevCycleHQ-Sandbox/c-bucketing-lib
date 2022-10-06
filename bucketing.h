#ifndef C_BUCKETING_LIB_BUCKETING_H
#define C_BUCKETING_LIB_BUCKETING_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wasm.h>
#include <wasmtime.h>
#include <stdbool.h>
#include <time.h>


#include "lib/bucketing-lib.release.wasm.h"

static void initialize();

static void cleanup();

static void init_event_queue(char *envKey, char *options);

static char *flush_event_queue(char *envKey);

static void on_payload_success(char *envKey, char *payloadId);

static void on_payload_failure(char *envKey, char *payloadId, bool retryable);

static char *generate_bucketed_config(char *token, char *user);

static int event_queue_size(char *envKey);

static void queue_event(char *envKey, char *user, char *eventString);

static void queue_aggregate_event(char *envKey, char *user, char *eventString);

static void store_config(char *token, char *config);

static void set_platform_data(char *platformData);

static void *asc_malloc(int length);

static int new_asc_string(const char *data, int len);

static unsigned char *read_asc_string(int addr);

    static wasm_trap_t *env__abort(void *env,
                               wasmtime_caller_t *caller,
                               const wasmtime_val_t *args,
                               size_t nargs,
                               wasmtime_val_t *results,
                               size_t nresults);

static wasm_trap_t *env__console_log(void *env,
                                     wasmtime_caller_t *caller,
                                     const wasmtime_val_t *args,
                                     size_t nargs,
                                     wasmtime_val_t *results,
                                     size_t nresults);

static wasm_trap_t *env__date_now(void *env,
                                  wasmtime_caller_t *caller,
                                  const wasmtime_val_t *args,
                                  size_t nargs,
                                  wasmtime_val_t *results,
                                  size_t nresults);

static wasm_trap_t *env__seed(void *env,
                              wasmtime_caller_t *caller,
                              const wasmtime_val_t *args,
                              size_t nargs,
                              wasmtime_val_t *results,
                              size_t nresults);

static void exit_with_error(const char *message, wasmtime_error_t *error, wasm_trap_t *trap);

#endif //C_BUCKETING_LIB_BUCKETING_H
