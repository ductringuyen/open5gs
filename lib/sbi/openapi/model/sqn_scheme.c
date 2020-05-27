
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sqn_scheme.h"

OpenAPI_sqn_scheme_t *OpenAPI_sqn_scheme_create(
    )
{
    OpenAPI_sqn_scheme_t *sqn_scheme_local_var = OpenAPI_malloc(sizeof(OpenAPI_sqn_scheme_t));
    if (!sqn_scheme_local_var) {
        return NULL;
    }

    return sqn_scheme_local_var;
}

void OpenAPI_sqn_scheme_free(OpenAPI_sqn_scheme_t *sqn_scheme)
{
    if (NULL == sqn_scheme) {
        return;
    }
    OpenAPI_lnode_t *node;
    ogs_free(sqn_scheme);
}

cJSON *OpenAPI_sqn_scheme_convertToJSON(OpenAPI_sqn_scheme_t *sqn_scheme)
{
    cJSON *item = NULL;

    if (sqn_scheme == NULL) {
        ogs_error("OpenAPI_sqn_scheme_convertToJSON() failed [SqnScheme]");
        return NULL;
    }

    item = cJSON_CreateObject();
end:
    return item;
}

OpenAPI_sqn_scheme_t *OpenAPI_sqn_scheme_parseFromJSON(cJSON *sqn_schemeJSON)
{
    OpenAPI_sqn_scheme_t *sqn_scheme_local_var = NULL;
    sqn_scheme_local_var = OpenAPI_sqn_scheme_create (
        );

    return sqn_scheme_local_var;
end:
    return NULL;
}

