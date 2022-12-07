//
// Created by Jamie Sinn on 2022-11-24.
//

#ifndef DVCBUCKETING_TESTING_H
#define DVCBUCKETING_TESTING_H
#include <sys/time.h>
#include <regex.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

char *getPayloadId(unsigned char *flushed_queue) {

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

#endif //DVCBUCKETING_TESTING_H
