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

static wasmtime_module_t *wasm_module;
static wasmtime_memory_t *wasm_memory;
static wasmtime_linker_t *wasm_linker;
static wasmtime_store_t  *wasm_store;
static wasm_engine_t *wasm_engine;
static wasmtime_context_t *wasm_context;

void initialize() {
    wasm_byte_vec_t wasm_file;
    wasm_file.data = (wasm_byte_t *) &___lib_bucketing_lib_release_wasm;
    wasm_file.size = ___lib_bucketing_lib_release_wasm_len;

    wasm_engine = wasm_engine_new();
    assert(wasm_engine != NULL);
    wasm_store = wasmtime_store_new(wasm_engine, NULL, NULL);
    assert(wasm_store != NULL);
    wasm_context = wasmtime_store_context(wasm_store);

}

void cleanup() {

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


int main() {
    int ret = 0;
    // Set up our compilation context. Note that we could also work with a
    // `wasm_config_t` here to configure what feature are enabled and various
    // compilation settings.
    printf("Initializing...\n");
    wasm_engine_t *engine = wasm_engine_new();
    assert(engine != NULL);

    // With an engine we can create a *store* which is a long-lived group of wasm
    // modules. Note that we allocate some custom data here to live in the store,
    // but here we skip that and specify NULL.
    wasmtime_store_t *store = wasmtime_store_new(engine, NULL, NULL);
    assert(store != NULL);
    wasmtime_context_t *context = wasmtime_store_context(store);

    // Read our input file, which in this case is a wasm text file.
    FILE *file = fopen("hello.wat", "r");
    assert(file != NULL);
    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    wasm_byte_vec_t wat;
    wasm_byte_vec_new_uninitialized(&wat, file_size);
    assert(fread(wat.data, file_size, 1, file) == 1);
    fclose(file);

    // Parse the wat into the binary wasm format
    wasm_byte_vec_t wasm;
    wasmtime_error_t * error = wasmtime_wat2wasm(wat.data, wat.size, &wasm);
    if (error != NULL)
        exit_with_error("failed to parse wat", error, NULL);
    wasm_byte_vec_delete(&wat);

    // Now that we've got our binary webassembly we can compile our module.
    printf("Compiling module...\n");
    wasmtime_module_t *module = NULL;
    error = wasmtime_module_new(engine, (uint8_t *) wasm.data, wasm.size, &module);
    wasm_byte_vec_delete(&wasm);
    if (error != NULL)
        exit_with_error("failed to compile module", error, NULL);

    // Next up we need to create the function that the wasm module imports. Here
    // we'll be hooking up a thunk function to the `hello_callback` native
    // function above. Note that we can assign custom data, but we just use NULL
    // for now).
    printf("Creating callback...\n");
    wasm_functype_t *hello_ty = wasm_functype_new_0_0();
    wasmtime_func_t hello;
    wasmtime_func_new(context, hello_ty, hello_callback, NULL, NULL, &hello);

    // With our callback function we can now instantiate the compiled module,
    // giving us an instance we can then execute exports from. Note that
    // instantiation can trap due to execution of the `start` function, so we need
    // to handle that here too.
    printf("Instantiating module...\n");
    wasm_trap_t *trap = NULL;
    wasmtime_instance_t instance;
    wasmtime_extern_t import;
    import.kind = WASMTIME_EXTERN_FUNC;
    import.of.func = hello;
    error = wasmtime_instance_new(context, module, &import, 1, &instance, &trap);
    if (error != NULL || trap != NULL)
        exit_with_error("failed to instantiate", error, trap);

    // Lookup our `run` export function
    printf("Extracting export...\n");
    wasmtime_extern_t run;
    bool ok = wasmtime_instance_export_get(context, &instance, "run", 3, &run);
    assert(ok);
    assert(run.kind == WASMTIME_EXTERN_FUNC);

    // And call it!
    printf("Calling export...\n");
    error = wasmtime_func_call(context, &run.of.func, NULL, 0, NULL, 0, &trap);
    if (error != NULL || trap != NULL)
        exit_with_error("failed to call function", error, trap);

    // Clean up after ourselves at this point
    printf("All finished!\n");
    ret = 0;

    wasmtime_module_delete(module);
    wasmtime_store_delete(store);
    wasm_engine_delete(engine);
    return ret;
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