#ifndef DVCBUCKETING_BUCKETING_H
#define DVCBUCKETING_BUCKETING_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wasm.h>
#include <wasmtime.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
// Values that can be overridden by the user.
#ifndef DVCBUCKETING_WASM_PAGES
#define DVCBUCKETING_WASM_PAGES 10000
#endif

// Exposed Functions

/**
 * initialize the underlying bucketing library. This function must be called before using any of the other calls.
 * @return success
 */
bool initialize();

/**
 * cleanup the WASM resources. Only use on destruction of the application.
 */
void cleanup();

/**
 * init_event_queue initializes the event queue that handles batching and aggregation of events before posting to the
 * DevCycle events api.
 * @param envKey environment sdk key
 * @param options event queue options. JSON
 */
void init_event_queue(const char *envKey, const char *options);

/**
 * flush_event_queue returns the request body for a given environment key's event queue to be submitted to
 * the DevCycle events api.
 * @param envKey environment sdk key
 * @return
 */
unsigned char *flush_event_queue(const char *envKey);

/**
 * on_payload_sucess is the callback to tell WASM that the given payload can be removed from the event queue and was
 * successfully sent to the DevCycle events api.
 * @param envKey environment sdk key
 * @param payloadId payload GUID
 */
void on_payload_success(const char *envKey, const char *payloadId);

/**
 * on_payload_failure is the callback to tell WASM that the given payload failed, and needs to be re-queued into the
 * next flush payload if it's retryable, and if not - to remove from the queue.
 * @param envKey environment sdk key
 * @param payloadId payload GUID
 * @param retryable whether the error was retryable or not
 */
void on_payload_failure(const char *envKey, const char *payloadId, bool retryable);

/**
 * generate_bucketed_config returns a bucketed user configuration for a given user.
 * @param envKey environment sdk key
 * @param user user to bucket
 * @return bucketed user config string
 */
unsigned char *generate_bucketed_config(const char *envKey, const char *user);

/**
 * event_queue_size returns the current size of a given SDK key's event queue.
 * @param envKey environment sdk key
 * @return size of event queue
 */
int event_queue_size(const char *envKey);

/**
 * queue_event queue's a single event to the WASM event queue
 * @param envKey environment sdk key
 * @param user bucketed user config associated with the event
 * @param eventString event data
 */
void queue_event(const char *envKey, const char *user, const char *eventString);

/**
 * queue_aggregate_event queue's an aggregated event to the WASM event queue
 * @param envKey environment sdk key
 * @param user bucketed user config associated with the event
 * @param eventString event data
 */
void queue_aggregate_event(const char *envKey, const char *user, const char *eventString);

/**
 * store_config updates the stored DevCycle environment project configuration in WASM used for bucketing rule evaluation
 * @param envKey environment sdk key
 * @param config environment project configuration
 */
void store_config(const char *envKey, const char *config);

/**
 * set_platform_data sets the platformdata in WASM used for bucketing evaluation rules
 * @param platformData platformdata
 */
void set_platform_data(const char *platformData);


#endif //DVCBUCKETING_BUCKETING_H
