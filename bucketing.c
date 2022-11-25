#include "bucketing.h"

// WASM Binary Data Include -- See scripts/get-bucketing-lib.sh
#include "lib/bucketing-lib.release.wasm.h"

bool initialize() {
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
    //wasm_byte_vec_delete(&wasm_file);
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


    // Create all the imported functions
    wasm_functype_t *env_abort_t = wasm_functype_new_4_0(wasm_valtype_new_i32(), wasm_valtype_new_i32(),
                                                         wasm_valtype_new_i32(), wasm_valtype_new_i32());
    wasmtime_func_t env_abort;
    wasmtime_func_new(wasm_context, env_abort_t, env_abort_callback, NULL, NULL, &env_abort);
    wasm_functype_delete(env_abort_t);

    wasm_functype_t *env_date_now_t = wasm_functype_new_0_1(wasm_valtype_new_f64());
    wasmtime_func_t env_date_now;
    wasmtime_func_new(wasm_context, env_date_now_t, env_date_now_callback, NULL, NULL, &env_date_now);
    wasm_functype_delete(env_date_now_t);

    wasm_functype_t *env_console_log_t = wasm_functype_new_1_0(wasm_valtype_new_i32());
    wasmtime_func_t env_console_log;
    wasmtime_func_new(wasm_context, env_console_log_t, env_console_log_callback, NULL, NULL, &env_console_log);
    wasm_functype_delete(env_console_log_t);

    wasm_functype_t *env_seed_t = wasm_functype_new_0_1(wasm_valtype_new_f64());
    wasmtime_func_t env_seed;
    wasmtime_func_new(wasm_context, env_seed_t, env_seed_callback, NULL, NULL, &env_seed);
    wasm_functype_delete(env_seed_t);


    wasmtime_extern_t imports[4] = {
            {.kind = WASMTIME_EXTERN_FUNC, .of.func = env_abort},
            {.kind = WASMTIME_EXTERN_FUNC, .of.func = env_date_now},
            {.kind = WASMTIME_EXTERN_FUNC, .of.func = env_console_log},
            {.kind = WASMTIME_EXTERN_FUNC, .of.func = env_seed}};

    error = wasmtime_instance_new(wasm_context, wasm_module, imports, 4, wasm_instance, &trap);
    if (error != NULL || trap != NULL)
        exit_with_error("failed to instantiate", error, trap);


    // Get the WASM Memory to access later.
    wasmtime_extern_t memory;
    bool ok;
    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, "memory", strlen("memory"), &memory);
    assert(ok && memory.kind == WASMTIME_EXTERN_MEMORY);
    wasm_memory = memory.of.memory;
    if (wasmtime_memory_size(wasm_context, &wasm_memory) != DVCBUCKETING_WASM_PAGES) {
        exit_with_error("WASM Memory was not allocated to the expected size in pages.", NULL, NULL);
    }

    // Find and return all functions exported from the WASM file.
    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, DVCBUCKETING_ASC_NEW,
                                      strlen(DVCBUCKETING_ASC_NEW), w__new);
    assert(ok);
    assert(w__new->kind == WASMTIME_EXTERN_FUNC);

    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, DVCBUCKETING_SETPLATFORMDATA,
                                      strlen(DVCBUCKETING_SETPLATFORMDATA), w_set_platform_data);
    assert(ok);
    assert(w_set_platform_data->kind == WASMTIME_EXTERN_FUNC);

    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, DVCBUCKETING_SETCONFIGDATA,
                                      strlen(DVCBUCKETING_SETCONFIGDATA), w_store_config);
    assert(ok);
    assert(w_store_config->kind == WASMTIME_EXTERN_FUNC);

    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, DVCBUCKETING_QUEUEAGGREGATEEVENT,
                                      strlen(DVCBUCKETING_QUEUEAGGREGATEEVENT), w_queue_aggregate_event);
    assert(ok);
    assert(w_queue_aggregate_event->kind == WASMTIME_EXTERN_FUNC);

    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, DVCBUCKETING_QUEUEEVENT,
                                      strlen(DVCBUCKETING_QUEUEEVENT), w_queue_event);
    assert(ok);
    assert(w_queue_event->kind == WASMTIME_EXTERN_FUNC);

    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, DVCBUCKETING_ONPAYLOADFAILURE,
                                      strlen(DVCBUCKETING_ONPAYLOADFAILURE), w_on_payload_failure);
    assert(ok);
    assert(w_on_payload_failure->kind == WASMTIME_EXTERN_FUNC);

    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, DVCBUCKETING_EVENTQUEUESIZE,
                                      strlen(DVCBUCKETING_EVENTQUEUESIZE), w_event_queue_size);
    assert(ok);
    assert(w_event_queue_size->kind == WASMTIME_EXTERN_FUNC);

    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, DVCBUCKETING_GENERATEBUCKETEDCONFIGFORUSER,
                                      strlen(DVCBUCKETING_GENERATEBUCKETEDCONFIGFORUSER), w_generate_bucketed_config);
    assert(ok);
    assert(w_generate_bucketed_config->kind == WASMTIME_EXTERN_FUNC);

    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, DVCBUCKETING_ONPAYLOADSUCCESS,
                                      strlen(DVCBUCKETING_ONPAYLOADSUCCESS), w_on_payload_success);
    assert(ok);
    assert(w_on_payload_success->kind == WASMTIME_EXTERN_FUNC);

    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, DVCBUCKETING_FLUSHEVENTQUEUE,
                                      strlen(DVCBUCKETING_FLUSHEVENTQUEUE), w_flush_event_queue);
    assert(ok);
    assert(w_flush_event_queue->kind == WASMTIME_EXTERN_FUNC);

    ok = wasmtime_instance_export_get(wasm_context, wasm_instance, DVCBUCKETING_INITEVENTQUEUE,
                                      strlen(DVCBUCKETING_INITEVENTQUEUE), w_init_event_queue);
    assert(ok);
    assert(w_init_event_queue->kind == WASMTIME_EXTERN_FUNC);

    if(trap)
        wasm_trap_delete(trap);
    if(error)
        wasmtime_error_delete(error);
    return ok;
}

void cleanup() {
    wasmtime_store_delete(wasm_store);
    wasmtime_module_delete(wasm_module);
    wasm_engine_delete(wasm_engine);
}

void init_event_queue(char *envKey, char *options) {
    wasm_trap_t *trap = NULL;
    wasmtime_error_t * error = NULL;
    wasmtime_val_t results[1];
    wasmtime_val_t params[2];

    wasmtime_val_t envKeyParam = new_asc_string_param(envKey);
    wasmtime_val_t optionsJSONParam = new_asc_string_param(options);
    params[0] = envKeyParam;
    params[1] = optionsJSONParam;
    error = wasmtime_func_call(wasm_context, &(*w_init_event_queue).of.func, params, 2, results,
                               0, &trap);
    if (error != NULL)
        exit_with_error("failed to call w_init_event_queue.", error, trap);


    wasm_trap_delete(trap);
    wasmtime_error_delete(error);
}

unsigned char *flush_event_queue(char *envKey) {
    wasmtime_val_t envKeyParam = new_asc_string_param(envKey);

    wasm_trap_t *trap = NULL;
    wasmtime_error_t * error = NULL;
    wasmtime_val_t results[1];
    wasmtime_val_t params[1];
    params[0] = envKeyParam;
    error = wasmtime_func_call(wasm_context, &(*w_flush_event_queue).of.func, params, 1, results,
                               1, &trap);
    if (error != NULL) {
        exit_with_error("failed to call w_event_queue_size.", error, trap);
    }
    assert(results[0].kind == WASMTIME_I32);

    wasm_trap_delete(trap);
    wasmtime_error_delete(error);
    wasmtime_val_delete(&envKeyParam);
    return read_asc_string(results[0].of.i32);
}

void on_payload_success(char *envKey, char *payloadId) {
    wasmtime_val_t envKeyParam = new_asc_string_param(envKey);

    wasm_trap_t *trap = NULL;
    wasmtime_error_t * error = NULL;
    wasmtime_val_t results[0];
    wasmtime_val_t params[2];

    wasmtime_val_t payloadIdParam = new_asc_string_param(payloadId);
    params[0] = envKeyParam;
    params[1] = payloadIdParam;
    error = wasmtime_func_call(wasm_context, &(*w_on_payload_success).of.func, params, 2, results,
                               0, &trap);
    if (error != NULL) {
        exit_with_error("failed to call w_on_payload_success.", error, trap);
    }

    wasm_trap_delete(trap);
    wasmtime_error_delete(error);
    wasmtime_val_delete(&envKeyParam);

}

void on_payload_failure(char *envKey, char *payloadId, bool retryable) {
    wasmtime_val_t envKeyParam = new_asc_string_param(envKey);
    wasmtime_val_t payloadIdParam = new_asc_string_param(payloadId);
    wasm_trap_t *trap = NULL;
    wasmtime_error_t * error = NULL;
    wasmtime_val_t results[0];
    wasmtime_val_t params[3];

    params[0] = envKeyParam;
    params[1] = payloadIdParam;
    params[2].kind = WASMTIME_I32;
    params[2].of.i32 = retryable ? 1 : 0;
    error = wasmtime_func_call(wasm_context, &(*w_on_payload_failure).of.func, params, 3, results,
                               0, &trap);
    if (error != NULL) {
        exit_with_error("failed to call w_on_payload_failure.", error, trap);
    }

    wasm_trap_delete(trap);

    wasmtime_error_delete(error);
    wasmtime_val_delete(&envKeyParam);
    wasmtime_val_delete(&payloadIdParam);


}

unsigned char *generate_bucketed_config(char *envKey, char *user) {
    wasmtime_val_t envKeyParam = new_asc_string_param(envKey);
    wasmtime_val_t userParam = new_asc_string_param(user);

    wasm_trap_t *trap = NULL;
    wasmtime_error_t * error = NULL;
    wasmtime_val_t results[1];
    wasmtime_val_t params[2];

    params[0] = envKeyParam;
    params[1] = userParam;
    error = wasmtime_func_call(wasm_context, &(*w_generate_bucketed_config).of.func, params, 2, results,
                               1, &trap);
    if (error != NULL) {
        exit_with_error("failed to call w_generate_bucketed_config.", error, trap);
    }
    assert(results[0].kind == WASMTIME_I32);

    wasm_trap_delete(trap);
    wasmtime_error_delete(error);
    wasmtime_val_delete(&envKeyParam);
    wasmtime_val_delete(&userParam);


    return read_asc_string(results[0].of.i32);
}

int event_queue_size(char *envKey) {
    wasmtime_val_t envKeyParam = new_asc_string_param(envKey);
    wasm_trap_t *trap = NULL;
    wasmtime_error_t * error = NULL;
    wasmtime_val_t results[1];
    wasmtime_val_t params[1];

    params[0] = envKeyParam;
    error = wasmtime_func_call(wasm_context, &(*w_event_queue_size).of.func, params, 1, results,
                               1, &trap);
    if (error != NULL) {
        exit_with_error("failed to call w_event_queue_size.", error, trap);
    }
    assert(results[0].kind == WASMTIME_I32);

    wasm_trap_delete(trap);
    wasmtime_error_delete(error);
    wasmtime_val_delete(&envKeyParam);

    return results[0].of.i32;
}

void queue_event(char *envKey, char *user, char *eventString) {
    wasmtime_val_t envKeyParam = new_asc_string_param(envKey);
    wasmtime_val_t userParam = new_asc_string_param(user);

    wasmtime_val_t eventStringParam = new_asc_string_param(eventString);
    wasm_trap_t *trap = NULL;
    wasmtime_error_t * error = NULL;
    wasmtime_val_t results[0];
    wasmtime_val_t params[3];

    params[0] = envKeyParam;
    params[1] = userParam;
    params[2] = eventStringParam;
    error = wasmtime_func_call(wasm_context, &(*w_queue_event).of.func, params, 3, results,
                               0, &trap);
    if (error != NULL) {
        exit_with_error("failed to call w_queue_event.", error, trap);
    }

    wasm_trap_delete(trap);
    wasmtime_error_delete(error);
    wasmtime_val_delete(&envKeyParam);
    wasmtime_val_delete(&userParam);
    wasmtime_val_delete(&eventStringParam);
}

void queue_aggregate_event(char *envKey, char *user, char *eventString) {
    wasmtime_val_t envKeyParam = new_asc_string_param(envKey);
    wasmtime_val_t userParam = new_asc_string_param(user);

    wasmtime_val_t eventStringParam = new_asc_string_param(eventString);
    wasm_trap_t *trap = NULL;
    wasmtime_error_t * error = NULL;
    wasmtime_val_t results[0];
    wasmtime_val_t params[3];

    params[0] = envKeyParam;
    params[1] = userParam;
    params[2] = eventStringParam;
    error = wasmtime_func_call(wasm_context, &(*w_queue_aggregate_event).of.func, params, 3, results,
                               0, &trap);
    if (error != NULL) {
        exit_with_error("failed to call w_queue_aggregate_event.", error, trap);
    }

    wasm_trap_delete(trap);
    wasmtime_error_delete(error);
    wasmtime_val_delete(params);

}

void store_config(char *envKey, char *config) {
    wasmtime_val_t envKeyParam = new_asc_string_param(envKey);

    wasm_trap_t *trap = NULL;
    wasmtime_error_t * error = NULL;
    wasmtime_val_t results[0];
    wasmtime_val_t params[2];

    wasmtime_val_t configParam = new_asc_string_param(config);
    params[0] = envKeyParam;
    params[1] = configParam;
    error = wasmtime_func_call(wasm_context, &(*w_store_config).of.func, params, 2, results,
                               0, &trap);
    if (error != NULL) {
        exit_with_error("failed to call w_store_config.", error, trap);
    }

    wasm_trap_delete(trap);
    wasmtime_error_delete(error);
    wasmtime_val_delete(&envKeyParam);
    wasmtime_val_delete(&configParam);

}

void set_platform_data(char *platformData) {
    wasmtime_val_t platformDataParam = new_asc_string_param(platformData);

    wasm_trap_t *trap = NULL;
    wasmtime_error_t * error = NULL;
    wasmtime_val_t results[0];
    wasmtime_val_t params[1];
    params[0] = platformDataParam;
    error = wasmtime_func_call(wasm_context, &(*w_set_platform_data).of.func, params, 1, results,
                               0, &trap);
    if (error != NULL) {
        exit_with_error("failed to call w_set_platform_data.", error, trap);
    }
    wasm_trap_delete(trap);
    wasmtime_error_delete(error);
    wasmtime_val_delete(params);

}

static int asc_malloc(unsigned long length) {
    wasm_trap_t *trap = NULL;
    wasmtime_error_t * error = NULL;
    wasmtime_val_t results[1];
    wasmtime_val_t params[2];
    // Arg 0 is the size to allocate.
    params[0].kind = WASMTIME_I32;
    params[0].of.i32 = (int) length;
    // Arg 1 is the type of allocation. We're assuming we only care about strings - class id of 1
    params[1].kind = WASMTIME_I32;
    params[1].of.i32 = 1;

    error = wasmtime_func_call(wasm_context, &(*w__new).of.func, params, 2, results,
                               1, &trap);
    if (error != NULL)
        exit_with_error("failed to call w__new. Cannot allocate memory for parameter", error, trap);

    assert(results[0].kind == WASMTIME_I32);

    wasm_trap_delete(trap);
    wasmtime_error_delete(error);
    wasmtime_val_delete(params);
    return results[0].of.i32;
}

static int new_asc_string(const char *data, unsigned long len) {
    int addr = asc_malloc(len * 2);
    for (int i = 0; i < len * 2; i += 2) {
        wasmtime_memory_data(wasm_context, &wasm_memory)[addr + i] = data[i / 2];
    }
    return addr;
}

// AssemblyScript's memory layout has the length of the data 4 bytes before the address.
static unsigned char *read_asc_string(int addr) {
    uint8_t bytes[] = {
            wasmtime_memory_data(wasm_context, &wasm_memory)[addr - 4],
            wasmtime_memory_data(wasm_context, &wasm_memory)[addr - 3],
            wasmtime_memory_data(wasm_context, &wasm_memory)[addr - 2],
            wasmtime_memory_data(wasm_context, &wasm_memory)[addr - 1],
    };

    uint32_t length = bytes[0] + (bytes[1] << 8) + (bytes[2] << 16) + (bytes[3] << 24);

    unsigned char *data = malloc(length);
    for (int i = 0; i < length; i += 2) {
        data[i / 2] = wasmtime_memory_data(wasm_context, &wasm_memory)[addr + i];
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

static wasm_trap_t *
env_abort_callback(__attribute__((unused)) __attribute__((unused)) void *env,
                   __attribute__((unused)) wasmtime_caller_t *caller, const wasmtime_val_t *args,
                   __attribute__((unused)) size_t nargs,
                   wasmtime_val_t *results,
                   size_t nresults) {

    if (nresults != 4) {
        char *message = "failed to match proper args length.";
        return wasmtime_trap_new(message, strlen(message));
    }
    // message address pointer
    unsigned char *message = read_asc_string(args[0].of.i32);
    // filename address pointer
    unsigned char *filename = read_asc_string(args[1].of.i32);
    // line number
    int line = args[2].of.i32;
    // column number
    int col = args[3].of.i32;
    printf("%s (%s) - %d:%d\n", message, filename, line, col);
    exit(1);
}

static wasm_trap_t *
env_console_log_callback(__attribute__((unused)) void *env, __attribute__((unused)) wasmtime_caller_t *caller,
                         const wasmtime_val_t *args, size_t nargs,
                         __attribute__((unused)) wasmtime_val_t *results, __attribute__((unused)) size_t nresults) {
    if (nargs >= 1) {
        printf("%s\n", read_asc_string(args[0].of.i32));
    }
    return NULL;
}

static wasm_trap_t *
env_date_now_callback(__attribute__((unused)) __attribute__((unused)) void *env,
                      __attribute__((unused)) wasmtime_caller_t *caller,
                      __attribute__((unused)) const wasmtime_val_t *args, __attribute__((unused)) size_t nargs,
                      wasmtime_val_t *results,
                      size_t nresults) {

    if (nresults != 1) {
        char *message = "failed to match proper return results length.";
        return wasmtime_trap_new(message, strlen(message));
    }

    wasmtime_val_t now;
    now.kind = WASMTIME_F64;
    // FIXME This returns seconds - we need millis. There's no standard implementation so this will have to be implementation specific
    now.of.f64 = time(NULL) * 1000;
    results[0] = now;
    return NULL;
}

static wasm_trap_t *
env_seed_callback(__attribute__((unused)) __attribute__((unused)) void *env,
                  __attribute__((unused)) wasmtime_caller_t *caller,
                  __attribute__((unused)) const wasmtime_val_t *args, size_t nargs,
                  wasmtime_val_t *results,
                  size_t nresults) {
    if (nresults != 1) {
        char *message = "failed to match proper return results length.";
        return wasmtime_trap_new(message, strlen(message));
    }

    wasmtime_val_t seed;
    seed.kind = WASMTIME_F64;
    // FIXME This returns seconds - we need millis. There's no standard implementation so this will have to be implementation specific
    seed.of.f64 = (float) (rand() * time(NULL) * 1000);
    results[0] = seed;

    return NULL;
}

static wasmtime_val_t new_asc_string_param(char *envKey) {
    wasmtime_val_t param = {.kind = WASMTIME_I32, .of.i32 = new_asc_string(envKey, strlen(envKey))};
    return param;
}