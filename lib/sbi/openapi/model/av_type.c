
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "av_type.h"

OpenAPI_av_type_t *OpenAPI_av_type_create(
    )
{
    OpenAPI_av_type_t *av_type_local_var = OpenAPI_malloc(sizeof(OpenAPI_av_type_t));
    if (!av_type_local_var) {
        return NULL;
    }

    return av_type_local_var;
}

void OpenAPI_av_type_free(OpenAPI_av_type_t *av_type)
{
    if (NULL == av_type) {
        return;
    }
    OpenAPI_lnode_t *node;
    ogs_free(av_type);
}

cJSON *OpenAPI_av_type_convertToJSON(OpenAPI_av_type_t *av_type)
{
    cJSON *item = NULL;

    if (av_type == NULL) {
        ogs_error("OpenAPI_av_type_convertToJSON() failed [AvType]");
        return NULL;
    }

    item = cJSON_CreateObject();
end:
    return item;
}

OpenAPI_av_type_t *OpenAPI_av_type_parseFromJSON(cJSON *av_typeJSON)
{
    OpenAPI_av_type_t *av_type_local_var = NULL;
    av_type_local_var = OpenAPI_av_type_create (
        );

    return av_type_local_var;
end:
    return NULL;
}

