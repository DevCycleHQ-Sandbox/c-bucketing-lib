//
// Created by Jamie Sinn on 2022-11-24.
//

#include <stdio.h>
#include "testing.h"
#include "../bucketing.h"

int main() {
    printf("testing binary\n");
    initialize();
    char *VariableKey = "test";
    char *configString = "{\"project\":{\"settings\":{\"edgeDB\":{\"enabled\":false}},\"_id\":\"6216420c2ea68943c8833c09\",\"key\":\"default\",\"a0_organization\":\"org_NszUFyWBFy7cr95J\"},\"environment\":{\"_id\":\"6216420c2ea68943c8833c0b\",\"key\":\"development\"},\"features\":[{\"_id\":\"6216422850294da359385e8b\",\"key\":\"test\",\"type\":\"release\",\"variations\":[{\"variables\":[{\"_var\":\"6216422850294da359385e8d\",\"value\":true}],\"name\":\"Variation On\",\"key\":\"variation-on\",\"_id\":\"6216422850294da359385e8f\"},{\"variables\":[{\"_var\":\"6216422850294da359385e8d\",\"value\":false}],\"name\":\"Variation Off\",\"key\":\"variation-off\",\"_id\":\"6216422850294da359385e90\"}],\"configuration\":{\"_id\":\"621642332ea68943c8833c4a\",\"targets\":[{\"distribution\":[{\"percentage\":0.5,\"_variation\":\"6216422850294da359385e8f\"},{\"percentage\":0.5,\"_variation\":\"6216422850294da359385e90\"}],\"_audience\":{\"_id\":\"621642332ea68943c8833c4b\",\"filters\":{\"operator\":\"and\",\"filters\":[{\"values\":[],\"type\":\"all\",\"filters\":[]}]}},\"_id\":\"621642332ea68943c8833c4d\"}],\"forcedUsers\":{}}}],\"variables\":[{\"_id\":\"6216422850294da359385e8d\",\"key\":\"test\",\"type\":\"Boolean\"}],\"featureVariationMap\":{\"6216422850294da359385e8b\":\"6216422850294da359385e8f\"},\"variableVariationMap\":{\"test\":\"6216422850294da359385e8f\"},\"variableHashes\":{\"test\":2447239932}}";


    init_event_queue("dvc_server_blahblahlbah_hash", "{}");

    store_config("dvc_server_blahblahlbah_hash", configString);
    set_platform_data("{\"platform\":\"c\",\"platformVersion\":\"1\", \"sdkType\":\"c-wrapper\", \"sdkVersion\":\"testing\"}");
    unsigned char *bucketedConfig = generate_bucketed_config("dvc_server_blahblahlbah_hash", "{\"user_id\":\"j_test\"}");
    printf("%s\n", bucketedConfig);
    return 0;
}