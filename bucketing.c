#include "bucketing.h"

static wasmtime_extern_t *_init_event_queue;
static wasmtime_extern_t *_flush_event_queue;
static wasmtime_extern_t *_on_payload_success;
static wasmtime_extern_t *_on_payload_failure;
static wasmtime_extern_t *_generate_bucketed_config;
static wasmtime_extern_t *_event_queue_size;
static wasmtime_extern_t *_queue_event;
static wasmtime_extern_t *_queue_aggregate_event;
static wasmtime_extern_t *_store_config;
static wasmtime_extern_t *_set_platform_data;
static wasmtime_extern_t *__new;


static wasmtime_module_t *wasm_module = NULL;
static wasmtime_memory_t wasm_memory;
static wasmtime_linker_t *wasm_linker;
static wasmtime_store_t *wasm_store;
static wasm_engine_t *wasm_engine;
static wasmtime_context_t *wasm_context;
static wasmtime_instance_t *wasm_instance;


void initialize() {
    srand(time(NULL));
    // We're storing the wasm compiled file in a constant via XXD.
    // This constant is set prior to compilation and is embedded inside the lib.
    wasmtime_error_t * error;
    wasm_byte_vec_t wasm_file;
    wasm_file.data = (wasm_byte_t *) &___lib_bucketing_lib_release_wasm;
    wasm_file.size = ___lib_bucketing_lib_release_wasm_len;

    // Initializing the WASM Engine, Store, and Context. Then cleaning up the wasm vector.
    wasm_engine = wasm_engine_new();
    assert(wasm_engine != NULL);
    wasm_store = wasmtime_store_new(wasm_engine, NULL, NULL);
    assert(wasm_store != NULL);
    wasm_context = wasmtime_store_context(wasm_store);
    error = wasmtime_module_new(wasm_engine, (uint8_t *) wasm_file.data, wasm_file.size,
                                &wasm_module);
    wasm_byte_vec_delete(&wasm_file);
    if (error != NULL) {
        exit_with_error("Could not instantiate WASM module.", error, NULL);
    }

    // We only use WASI for IO handling
    wasi_config_t *wasi_config = wasi_config_new();
    assert(wasi_config);
    wasi_config_inherit_stderr(wasi_config);
    wasi_config_inherit_stdout(wasi_config);
    wasm_trap_t *trap = NULL;
    error = wasmtime_context_set_wasi(wasm_context, wasi_config);
    if (error != NULL)
        exit_with_error("Failed to initialize WASI configuration.", error, trap);

    // Set up the linker - we'll need to bind our externals here later.
    error = wasmtime_linker_instantiate(wasm_linker, wasm_context, wasm_module,
                                        wasm_instance, &trap);
    if (error != NULL || trap != NULL)
        exit_with_error("failed to instantiate linking1", error, trap);


    // Get the WASM Memory to access later.;
    wasmtime_extern_t item;
    bool ok;
    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, "memory", strlen("memory"), &item);
    assert(ok && item.kind == WASMTIME_EXTERN_MEMORY);
    wasm_memory = item.of.memory;
    if (wasmtime_memory_size(wasm_context, &wasm_memory) != 0x10000) {
        exit_with_error("WASM Memory was not allocated to the expected size in pages.", NULL, NULL);
    }

    // Find and return all functions exported from the WASM file.
    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, "__new", 5, __new);
    assert(ok);
    assert(__new->kind == WASMTIME_EXTERN_FUNC);

    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, "setPlatformData", 15, _set_platform_data);
    assert(ok);
    assert(__new->kind == WASMTIME_EXTERN_FUNC);

    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, "setConfigData", 13, _store_config);
    assert(ok);
    assert(__new->kind == WASMTIME_EXTERN_FUNC);

    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, "queueAggregateEvent", 19, _queue_aggregate_event);
    assert(ok);
    assert(__new->kind == WASMTIME_EXTERN_FUNC);

    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, "queueEvent", 10, _queue_event);
    assert(ok);
    assert(__new->kind == WASMTIME_EXTERN_FUNC);

    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, "onPayloadFailure", 16, _on_payload_failure);
    assert(ok);
    assert(__new->kind == WASMTIME_EXTERN_FUNC);

    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, "eventQueueSize", 14, _event_queue_size);
    assert(ok);
    assert(__new->kind == WASMTIME_EXTERN_FUNC);

    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, "generateBucketedConfigForUser", 29,
                                      _generate_bucketed_config);
    assert(ok);
    assert(__new->kind == WASMTIME_EXTERN_FUNC);

    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, "onPayloadSuccess", 16, _on_payload_success);
    assert(ok);
    assert(__new->kind == WASMTIME_EXTERN_FUNC);

    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, "flushEventQueue", 15, _flush_event_queue);
    assert(ok);
    assert(__new->kind == WASMTIME_EXTERN_FUNC);

    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, "initEventQueue", 14, _init_event_queue);
    assert(ok);
    assert(__new->kind == WASMTIME_EXTERN_FUNC);


}

void cleanup() {
    wasmtime_store_delete(wasm_store);
    wasmtime_module_delete(wasm_module);
    wasm_engine_delete(wasm_engine);
}

void init_event_queue(char *envKey, char *options) {

}

char *flush_event_queue(char *envKey) {
    return "";
}

void on_payload_success(char *envKey, char *payloadId) {

}

void on_payload_failure(char *envKey, char *payloadId, bool retryable) {

}

char *generate_bucketed_config(char *token, char *user) {
    return "";
}

int event_queue_size(char *envKey) {
    return 0;
}

void queue_event(char *envKey, char *user, char *eventString) {

}

void queue_aggregate_event(char *envKey, char *user, char *eventString) {

}

void store_config(char *token, char *config) {

}

void set_platform_data(char *platformData) {

}

/**
 * asc_malloc allocates memory inside of the webassembly memory using the assemblyscript __new function.
 * @param length length of the memory to be allocated
 * @return start address inside wasm memory
 */
static void *_malloc_asc(int length) {
    wasm_trap_t *trap = NULL;
    wasmtime_error_t * error = NULL;
    wasmtime_val_t results[1];
    wasmtime_val_t params[2];
    // Arg 0 is the size to allocate.
    params[0].kind = WASMTIME_I32;
    params[0].of.i32 = length;
    // Arg 1 is the type of allocation. We're assuming we only care about strings - class id of 1
    params[1].kind = WASMTIME_I32;
    params[1].of.i32 = 1;

    error = wasmtime_func_call(wasm_context, &(*__new).of.func, params, 1, results,
                               1, &trap);
    if (error != NULL) {
        exit_with_error("failed to call __new. Cannot allocate memory for parameter", error, trap);
    }
    assert(results[0].kind == WASMTIME_EXTERNREF);
    return wasmtime_externref_data(results[0].of.externref);
}

static int new_asc_string(const char *data, int len) {
    int *addr = asc_malloc(len * 2);
    for (int i = 0; i < len * 2; i += 2) {
        wasmtime_memory_data(wasm_context, &wasm_memory)[*addr + i] = data[i / 2];
    }
    return *addr;
}

static unsigned char *read_asc_string(int addr) {
    int length = 0;
    uint8_t raw_length[] = {
            wasmtime_memory_data(wasm_context, &wasm_memory)[addr - 4],
            wasmtime_memory_data(wasm_context, &wasm_memory)[addr - 3],
            wasmtime_memory_data(wasm_context, &wasm_memory)[addr - 2],
            wasmtime_memory_data(wasm_context, &wasm_memory)[addr - 1],
    };

    // Convert the raw 4 bytes to an int
    for (int i = 0; i < 4; i++) {
        length = (length << 1) + raw_length[i];
        // some bitshift nonsense here
    }

    unsigned char *data = malloc(length);
    for (int i = 0; i < length; i += 2) {
        data[i/2] = wasmtime_memory_data(wasm_context, &wasm_memory)[addr + i];
    }
    return data;
}


static void exit_with_error(const char *message, wasmtime_error_t *error, wasm_trap_t *trap) {
    fprintf(stderr, "error: %s\n", message);
    wasm_byte_vec_t error_message;
    if (error != NULL) {
        wasmtime_error_message(error, &error_message);
        wasmtime_error_delete(error);
    } else {
        wasm_trap_message(trap, &error_message);
        wasm_trap_delete(trap);
    }
    fprintf(stderr, "%.*s\n", (int) error_message.size, error_message.data);
    wasm_byte_vec_delete(&error_message);
    exit(1);
}

wasm_trap_t *
env__abort(void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args, size_t nargs, wasmtime_val_t *results,
           size_t nresults) {

    // message address pointer
    unsigned char *message = read_asc_string(args[0].of.i32);
    // filename address pointer
    unsigned char *filename = read_asc_string(args[1].of.i32);
    // line number
    int line = args[2].of.i32;
    // column number
    int col = args[3].of.i32;
    printf("%s : %s - %d:%d\n",message, filename, line, col );
    exit(1);
    return NULL;
}

wasm_trap_t *env__console_log(void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args, size_t nargs,
                              wasmtime_val_t *results, size_t nresults) {
    printf("%s\n", read_asc_string(args[0].of.i32));
    return NULL;
}

wasm_trap_t *
env__date_now(void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args, size_t nargs, wasmtime_val_t *results,
              size_t nresults) {

    if (nresults != 1) {
        char *message = "failed to match proper return results length.";
        return wasmtime_trap_new(message, strlen(message));
    }

    wasmtime_val_t now;
    now.kind = WASMTIME_F64;
    // This returns seconds - we need millis.
    now.of.f64 = time(NULL) * 1000;
    results[0] = now;
    return NULL;
}

wasm_trap_t *
env__seed(void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args, size_t nargs, wasmtime_val_t *results,
          size_t nresults) {
    if (nresults != 1) {
        char *message = "failed to match proper return results length.";
        return wasmtime_trap_new(message, strlen(message));
    }

    wasmtime_val_t seed;
    seed.kind = WASMTIME_F64;
    // This returns seconds - we need millis.
    seed.of.f64 = rand() * time(NULL) * 1000;
    results[0] = seed;

    return NULL;
}
