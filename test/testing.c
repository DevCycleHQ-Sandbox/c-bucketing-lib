//
// Created by Jamie Sinn on 2022-11-24.
//

#include <stdio.h>
#include "testing.h"
#include "../bucketing.h"
#include <sys/time.h>
#include <regex.h>
#include <ctype.h>

char *ltrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

char *rtrim(char *s)
{
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

char *trim(char *s)
{
    return rtrim(ltrim(s));
}

char *getPayloadId(char *flushed_queue) {

    char *payloadId;
    regex_t payloadIdRegex;
    regmatch_t rm[1];
    int reti;
    reti = regcomp(&payloadIdRegex, "[a-f0-9]{8}-[a-f0-9]{4}-4[a-f0-9]{3}-(:?8|9|[aA]|[bB])[a-f0-9]{3}-[a-f0-9]{12}", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile payloadIdRegex\n");
        exit(1);
    }

    /* Execute regular expression */
    reti = regexec(&payloadIdRegex, flushed_queue, 1, rm, 0);
    if (!reti) {
        puts("Match");
        payloadId = malloc((int)(rm[0].rm_eo - rm[0].rm_so));
        for (int i = 0; i < (int)(rm[0].rm_eo - rm[0].rm_so); i++)
        {
            payloadId[i] = flushed_queue[rm[0].rm_so+i];
        }
        printf("Payload Id: '%s'\n", payloadId);
    } else if (reti == REG_NOMATCH) {
        puts("No match");
        exit(1);
    } else {
        fprintf(stderr, "Regex match failed\n");
        exit(1);
    }
    regfree(&payloadIdRegex);
    return payloadId;
}

int main() {

    char *sdkKey = "dvc_server_blahblahlbah_hash";
    char *configString = "{\"project\":{\"settings\":{\"edgeDB\":{\"enabled\":false}},\"_id\":\"6216420c2ea68943c8833c09\",\"key\":\"default\",\"a0_organization\":\"org_NszUFyWBFy7cr95J\"},\"environment\":{\"_id\":\"6216420c2ea68943c8833c0b\",\"key\":\"development\"},\"features\":[{\"_id\":\"6216422850294da359385e8b\",\"key\":\"test\",\"type\":\"release\",\"variations\":[{\"variables\":[{\"_var\":\"6216422850294da359385e8d\",\"value\":true}],\"name\":\"Variation On\",\"key\":\"variation-on\",\"_id\":\"6216422850294da359385e8f\"},{\"variables\":[{\"_var\":\"6216422850294da359385e8d\",\"value\":false}],\"name\":\"Variation Off\",\"key\":\"variation-off\",\"_id\":\"6216422850294da359385e90\"}],\"configuration\":{\"_id\":\"621642332ea68943c8833c4a\",\"targets\":[{\"distribution\":[{\"percentage\":0.5,\"_variation\":\"6216422850294da359385e8f\"},{\"percentage\":0.5,\"_variation\":\"6216422850294da359385e90\"}],\"_audience\":{\"_id\":\"621642332ea68943c8833c4b\",\"filters\":{\"operator\":\"and\",\"filters\":[{\"values\":[],\"type\":\"all\",\"filters\":[]}]}},\"_id\":\"621642332ea68943c8833c4d\"}],\"forcedUsers\":{}}}],\"variables\":[{\"_id\":\"6216422850294da359385e8d\",\"key\":\"test\",\"type\":\"Boolean\"}],\"featureVariationMap\":{\"6216422850294da359385e8b\":\"6216422850294da359385e8f\"},\"variableVariationMap\":{\"test\":\"6216422850294da359385e8f\"},\"variableHashes\":{\"test\":2447239932}}";
    char *platformData = "{\"platform\":\"c\",\"platformVersion\":\"1\", \"sdkType\":\"c-wrapper\", \"sdkVersion\":\"testing\"}";
    struct timeval stop, start;


    printf("testing binary\n");
    initialize();


    init_event_queue(sdkKey, "{}");
    store_config(sdkKey, configString);
    set_platform_data(platformData);
    unsigned char *bucketedConfig = generate_bucketed_config(sdkKey, "{\"user_id\":\"j_test\"}");
    printf("%s\n", bucketedConfig);

    // how fast is it to generate a bucketed config?
    gettimeofday(&start, NULL);
    bucketedConfig = generate_bucketed_config(sdkKey, "{\"user_id\":\"j_test\"}");
    gettimeofday(&stop, NULL);
    printf("took %lu microseconds to generate a bucketed config\n",
           (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);

    queue_event(sdkKey, "{\"user_id\":\"j_test\"}", "{\"type\":\"testing\"}");
    queue_event(sdkKey, "{\"user_id\":\"j_test\"}", "{\"type\":\"testing2\"}");
    queue_event(sdkKey, "{\"user_id\":\"j_test\"}", "{\"type\":\"testing3\"}");

    unsigned char *flushed_queue = flush_event_queue(sdkKey);
    printf("%s\n", flushed_queue);

    char *payloadId = getPayloadId(flushed_queue);

    on_payload_failure(sdkKey, payloadId, true);
    assert(strcmp(flushed_queue, flush_event_queue(sdkKey)) == 0);

    on_payload_success(sdkKey, payloadId);
    flushed_queue = flush_event_queue(sdkKey);
    assert(strcmp("[]", flushed_queue) == 0);
    printf("%s\n", flushed_queue);

    return 0;
}
