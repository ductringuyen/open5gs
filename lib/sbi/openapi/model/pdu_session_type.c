
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "pdu_session_type.h"

OpenAPI_pdu_session_type_t *OpenAPI_pdu_session_type_create(
    )
{
    OpenAPI_pdu_session_type_t *pdu_session_type_local_var = OpenAPI_malloc(sizeof(OpenAPI_pdu_session_type_t));
    if (!pdu_session_type_local_var) {
        return NULL;
    }

    return pdu_session_type_local_var;
}

void OpenAPI_pdu_session_type_free(OpenAPI_pdu_session_type_t *pdu_session_type)
{
    if (NULL == pdu_session_type) {
        return;
    }
    OpenAPI_lnode_t *node;
    ogs_free(pdu_session_type);
}

cJSON *OpenAPI_pdu_session_type_convertToJSON(OpenAPI_pdu_session_type_t *pdu_session_type)
{
    cJSON *item = NULL;

    if (pdu_session_type == NULL) {
        ogs_error("OpenAPI_pdu_session_type_convertToJSON() failed [PduSessionType]");
        return NULL;
    }

    item = cJSON_CreateObject();
end:
    return item;
}

OpenAPI_pdu_session_type_t *OpenAPI_pdu_session_type_parseFromJSON(cJSON *pdu_session_typeJSON)
{
    OpenAPI_pdu_session_type_t *pdu_session_type_local_var = NULL;
    pdu_session_type_local_var = OpenAPI_pdu_session_type_create (
        );

    return pdu_session_type_local_var;
end:
    return NULL;
}

OpenAPI_pdu_session_type_t *OpenAPI_pdu_session_type_copy(OpenAPI_pdu_session_type_t *dst, OpenAPI_pdu_session_type_t *src)
{
    cJSON *item = NULL;
    char *content = NULL;

    ogs_assert(src);
    item = OpenAPI_pdu_session_type_convertToJSON(src);
    if (!item) {
        ogs_error("OpenAPI_pdu_session_type_convertToJSON() failed");
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

    OpenAPI_pdu_session_type_free(dst);
    dst = OpenAPI_pdu_session_type_parseFromJSON(item);
    cJSON_Delete(item);

    return dst;
}

