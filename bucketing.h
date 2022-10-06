#ifndef C_BUCKETING_LIB_BUCKETING_H
#define C_BUCKETING_LIB_BUCKETING_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wasm.h>
#include <wasmtime.h>
#include <stdbool.h>


#include "lib/bucketing-lib.release.wasm.h"

void initialize();

void cleanup();

void init_event_queue(char *envKey, char *options);

char *flush_event_queue(char *envKey);

void on_payload_success(char *envKey, char *payloadId);

void on_payload_failure(char *envKey, char *payloadId, bool retryable);

char *generate_bucketed_config(char *token, char *user);

int event_queue_size(char *envKey);

void queue_event(char *envKey, char *user, char *eventString);

void queue_aggregate_event(char *envKey, char *user, char *eventString);

void store_config(char *token, char *config);

void set_platform_data(char *platformData);

static void exit_with_error(const char *message, wasmtime_error_t *error, wasm_trap_t *trap);

static wasm_trap_t *hello_callback(
        void *env,
        wasmtime_caller_t *caller,
        const wasmtime_val_t *args,
        size_t nargs,
        wasmtime_val_t *results,
        size_t nresults
) {
    printf("Calling back...\n");
    printf("> Hello World!\n");
    return NULL;
}


static wasm_trap_t *env__abort(void *env,
                               wasmtime_caller_t *caller,
                               const wasmtime_val_t *args,
                               size_t nargs,
                               wasmtime_val_t *results,
                               size_t nresults) {

    return NULL;
}

static wasm_trap_t *env__console_log(void *env,
                                     wasmtime_caller_t *caller,
                                     const wasmtime_val_t *args,
                                     size_t nargs,
                                     wasmtime_val_t *results,
                                     size_t nresults) {

    return NULL;
}

static wasm_trap_t *env__date_now(void *env,
                                  wasmtime_caller_t *caller,
                                  const wasmtime_val_t *args,
                                  size_t nargs,
                                  wasmtime_val_t *results,
                                  size_t nresults) {

    return NULL;
}

static wasm_trap_t *env__seed(void *env,
                              wasmtime_caller_t *caller,
                              const wasmtime_val_t *args,
                              size_t nargs,
                              wasmtime_val_t *results,
                              size_t nresults) {

    return NULL;
}

#endif //C_BUCKETING_LIB_BUCKETING_H
