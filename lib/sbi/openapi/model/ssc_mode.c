
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ssc_mode.h"

OpenAPI_ssc_mode_t *OpenAPI_ssc_mode_create(
    )
{
    OpenAPI_ssc_mode_t *ssc_mode_local_var = OpenAPI_malloc(sizeof(OpenAPI_ssc_mode_t));
    if (!ssc_mode_local_var) {
        return NULL;
    }

    return ssc_mode_local_var;
}

void OpenAPI_ssc_mode_free(OpenAPI_ssc_mode_t *ssc_mode)
{
    if (NULL == ssc_mode) {
        return;
    }
    OpenAPI_lnode_t *node;
    ogs_free(ssc_mode);
}

cJSON *OpenAPI_ssc_mode_convertToJSON(OpenAPI_ssc_mode_t *ssc_mode)
{
    cJSON *item = NULL;

    if (ssc_mode == NULL) {
        ogs_error("OpenAPI_ssc_mode_convertToJSON() failed [SscMode]");
        return NULL;
    }

    item = cJSON_CreateObject();
end:
    return item;
}

OpenAPI_ssc_mode_t *OpenAPI_ssc_mode_parseFromJSON(cJSON *ssc_modeJSON)
{
    OpenAPI_ssc_mode_t *ssc_mode_local_var = NULL;
    ssc_mode_local_var = OpenAPI_ssc_mode_create (
        );

    return ssc_mode_local_var;
end:
    return NULL;
}

OpenAPI_ssc_mode_t *OpenAPI_ssc_mode_copy(OpenAPI_ssc_mode_t *dst, OpenAPI_ssc_mode_t *src)
{
    cJSON *item = NULL;
    char *content = NULL;

    ogs_assert(src);
    item = OpenAPI_ssc_mode_convertToJSON(src);
    if (!item) {
        ogs_error("OpenAPI_ssc_mode_convertToJSON() failed");
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

    OpenAPI_ssc_mode_free(dst);
    dst = OpenAPI_ssc_mode_parseFromJSON(item);
    cJSON_Delete(item);

    return dst;
}

