
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sms_support.h"

OpenAPI_sms_support_t *OpenAPI_sms_support_create(
    )
{
    OpenAPI_sms_support_t *sms_support_local_var = OpenAPI_malloc(sizeof(OpenAPI_sms_support_t));
    if (!sms_support_local_var) {
        return NULL;
    }

    return sms_support_local_var;
}

void OpenAPI_sms_support_free(OpenAPI_sms_support_t *sms_support)
{
    if (NULL == sms_support) {
        return;
    }
    OpenAPI_lnode_t *node;
    ogs_free(sms_support);
}

cJSON *OpenAPI_sms_support_convertToJSON(OpenAPI_sms_support_t *sms_support)
{
    cJSON *item = NULL;

    if (sms_support == NULL) {
        ogs_error("OpenAPI_sms_support_convertToJSON() failed [SmsSupport]");
        return NULL;
    }

    item = cJSON_CreateObject();
end:
    return item;
}

OpenAPI_sms_support_t *OpenAPI_sms_support_parseFromJSON(cJSON *sms_supportJSON)
{
    OpenAPI_sms_support_t *sms_support_local_var = NULL;
    sms_support_local_var = OpenAPI_sms_support_create (
        );

    return sms_support_local_var;
end:
    return NULL;
}

OpenAPI_sms_support_t *OpenAPI_sms_support_copy(OpenAPI_sms_support_t *dst, OpenAPI_sms_support_t *src)
{
    cJSON *item = NULL;
    char *content = NULL;

    ogs_assert(src);
    item = OpenAPI_sms_support_convertToJSON(src);
    if (!item) {
        ogs_error("OpenAPI_sms_support_convertToJSON() failed");
        return NULL;
    }

    content = cJSON_Print(item);
    cJSON_Delete(item);

    if (!content) {
        ogs_error("cJSON_Print() failed");
        return NULL;
    }

    item = cJSON_Parse(content);
    ogs_free(content);
    if (!item) {
        ogs_error("cJSON_Parse() failed");
        return NULL;
    }

    OpenAPI_sms_support_free(dst);
    dst = OpenAPI_sms_support_parseFromJSON(item);
    cJSON_Delete(item);

    return dst;
}

