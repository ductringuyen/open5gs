
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "auth_type.h"

OpenAPI_auth_type_t *OpenAPI_auth_type_create(
    )
{
    OpenAPI_auth_type_t *auth_type_local_var = OpenAPI_malloc(sizeof(OpenAPI_auth_type_t));
    if (!auth_type_local_var) {
        return NULL;
    }

    return auth_type_local_var;
}

void OpenAPI_auth_type_free(OpenAPI_auth_type_t *auth_type)
{
    if (NULL == auth_type) {
        return;
    }
    OpenAPI_lnode_t *node;
    ogs_free(auth_type);
}

cJSON *OpenAPI_auth_type_convertToJSON(OpenAPI_auth_type_t *auth_type)
{
    cJSON *item = NULL;

    if (auth_type == NULL) {
        ogs_error("OpenAPI_auth_type_convertToJSON() failed [AuthType]");
        return NULL;
    }

    item = cJSON_CreateObject();
end:
    return item;
}

OpenAPI_auth_type_t *OpenAPI_auth_type_parseFromJSON(cJSON *auth_typeJSON)
{
    OpenAPI_auth_type_t *auth_type_local_var = NULL;
    auth_type_local_var = OpenAPI_auth_type_create (
        );

    return auth_type_local_var;
end:
    return NULL;
}

