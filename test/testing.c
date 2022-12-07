//
// Created by Jamie Sinn on 2022-11-24.
//

#include "testing.h"
#include "../bucketing.h"

int main() {
    char *sdkKey = "dvc_server_blahblahlbah_hash";
    char *configString = "{\"project\":{\"settings\":{\"edgeDB\":{\"enabled\":false}},\"_id\":\"6216420c2ea68943c8833c09\",\"key\":\"default\",\"a0_organization\":\"org_NszUFyWBFy7cr95J\"},\"environment\":{\"_id\":\"6216420c2ea68943c8833c0b\",\"key\":\"development\"},\"features\":[{\"_id\":\"6216422850294da359385e8b\",\"key\":\"test\",\"type\":\"release\",\"variations\":[{\"variables\":[{\"_var\":\"6216422850294da359385e8d\",\"value\":true}],\"name\":\"Variation On\",\"key\":\"variation-on\",\"_id\":\"6216422850294da359385e8f\"},{\"variables\":[{\"_var\":\"6216422850294da359385e8d\",\"value\":false}],\"name\":\"Variation Off\",\"key\":\"variation-off\",\"_id\":\"6216422850294da359385e90\"}],\"configuration\":{\"_id\":\"621642332ea68943c8833c4a\",\"targets\":[{\"distribution\":[{\"percentage\":0.5,\"_variation\":\"6216422850294da359385e8f\"},{\"percentage\":0.5,\"_variation\":\"6216422850294da359385e90\"}],\"_audience\":{\"_id\":\"621642332ea68943c8833c4b\",\"filters\":{\"operator\":\"and\",\"filters\":[{\"values\":[],\"type\":\"all\",\"filters\":[]}]}},\"_id\":\"621642332ea68943c8833c4d\"}],\"forcedUsers\":{}}}],\"variables\":[{\"_id\":\"6216422850294da359385e8d\",\"key\":\"test\",\"type\":\"Boolean\"}],\"featureVariationMap\":{\"6216422850294da359385e8b\":\"6216422850294da359385e8f\"},\"variableVariationMap\":{\"test\":\"6216422850294da359385e8f\"},\"variableHashes\":{\"test\":2447239932}}";
    char *platformData = "{\"platform\":\"c\",\"platformVersion\":\"1\", \"sdkType\":\"c-wrapper\", \"sdkVersion\":\"testing\"}";
    struct timeval stop, start;


    printf("DevCycle Local Bucketing Wrapper - Testing WASM Functionality\n");

    printf("Initializing WASMTime engine... ");
    initialize();
    printf("Success\n");

    printf("Initializing Event Queue... ");
    init_event_queue(sdkKey, "{}");
    printf("Success\n");
    printf("Storing Environment Configuration... ");
    store_config(sdkKey, configString);
    printf("Success\n");
    printf("Storing Platform Data... ");
    set_platform_data(platformData);
    printf("Success\n");
    printf("Generating a bucketed config... ");
    gettimeofday(&start, NULL);
    unsigned char *bucketedConfig = generate_bucketed_config(sdkKey, "{\"user_id\":\"j_test\"}");
    gettimeofday(&stop, NULL);
    printf("Success\nBucketed User Config: %s\n", bucketedConfig);
    printf("took %lu microseconds to generate a bucketed config\n",
           (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);

    printf("Queuing multiple events... \n");
    queue_event(sdkKey, "{\"user_id\":\"j_test\"}", "{\"type\":\"testing\"}");
    printf("1/3\n");
    queue_event(sdkKey, "{\"user_id\":\"j_test\"}", "{\"type\":\"testing2\"}");
    printf("2/3\n");
    queue_event(sdkKey, "{\"user_id\":\"j_test\"}", "{\"type\":\"testing3\"}");
    printf("3/3\n");
    printf("Success\n");
    printf("Getting Event Queue Size... ");
    int equeue_size = event_queue_size(sdkKey);
    assert(equeue_size == 3);
    printf("Success\nEvent Queue Size: 3\n");

    unsigned char *flushed_queue = flush_event_queue(sdkKey);
    printf("Flushed Queue: '%s'\n", flushed_queue);

    char *payloadId = getPayloadId(flushed_queue);

    printf("Marking queue as failed. Expect next flush to equal previous\n");
    on_payload_failure(sdkKey, payloadId, true);
    unsigned char *secondFlush = flush_event_queue(sdkKey);
    assert(strcmp(flushed_queue, secondFlush)== 0);
    printf("Flushed Queue: %s\n", flushed_queue);

    printf("Marking queue as success. Expect next flush to equal empty array\n");
    on_payload_success(sdkKey, payloadId);
    flushed_queue = flush_event_queue(sdkKey);
    assert(strcmp("[]", flushed_queue) == 0);
    printf("Flushed Queue: %s\n", flushed_queue);

    return 0;
}
