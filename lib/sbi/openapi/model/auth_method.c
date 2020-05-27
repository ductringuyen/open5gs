
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "auth_method.h"

OpenAPI_auth_method_t *OpenAPI_auth_method_create(
    )
{
    OpenAPI_auth_method_t *auth_method_local_var = OpenAPI_malloc(sizeof(OpenAPI_auth_method_t));
    if (!auth_method_local_var) {
        return NULL;
    }

    return auth_method_local_var;
}

void OpenAPI_auth_method_free(OpenAPI_auth_method_t *auth_method)
{
    if (NULL == auth_method) {
        return;
    }
    OpenAPI_lnode_t *node;
    ogs_free(auth_method);
}

cJSON *OpenAPI_auth_method_convertToJSON(OpenAPI_auth_method_t *auth_method)
{
    cJSON *item = NULL;

    if (auth_method == NULL) {
        ogs_error("OpenAPI_auth_method_convertToJSON() failed [AuthMethod]");
        return NULL;
    }

    item = cJSON_CreateObject();
end:
    return item;
}

OpenAPI_auth_method_t *OpenAPI_auth_method_parseFromJSON(cJSON *auth_methodJSON)
{
    OpenAPI_auth_method_t *auth_method_local_var = NULL;
    auth_method_local_var = OpenAPI_auth_method_create (
        );

    return auth_method_local_var;
end:
    return NULL;
}

