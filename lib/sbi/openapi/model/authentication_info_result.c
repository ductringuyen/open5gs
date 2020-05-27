
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "authentication_info_result.h"

OpenAPI_authentication_info_result_t *OpenAPI_authentication_info_result_create(
    OpenAPI_auth_type_t *auth_type,
    char *supported_features,
    OpenAPI_authentication_vector_t *authentication_vector,
    char *supi
    )
{
    OpenAPI_authentication_info_result_t *authentication_info_result_local_var = OpenAPI_malloc(sizeof(OpenAPI_authentication_info_result_t));
    if (!authentication_info_result_local_var) {
        return NULL;
    }
    authentication_info_result_local_var->auth_type = auth_type;
    authentication_info_result_local_var->supported_features = supported_features;
    authentication_info_result_local_var->authentication_vector = authentication_vector;
    authentication_info_result_local_var->supi = supi;

    return authentication_info_result_local_var;
}

void OpenAPI_authentication_info_result_free(OpenAPI_authentication_info_result_t *authentication_info_result)
{
    if (NULL == authentication_info_result) {
        return;
    }
    OpenAPI_lnode_t *node;
    OpenAPI_auth_type_free(authentication_info_result->auth_type);
    ogs_free(authentication_info_result->supported_features);
    OpenAPI_authentication_vector_free(authentication_info_result->authentication_vector);
    ogs_free(authentication_info_result->supi);
    ogs_free(authentication_info_result);
}

cJSON *OpenAPI_authentication_info_result_convertToJSON(OpenAPI_authentication_info_result_t *authentication_info_result)
{
    cJSON *item = NULL;

    if (authentication_info_result == NULL) {
        ogs_error("OpenAPI_authentication_info_result_convertToJSON() failed [AuthenticationInfoResult]");
        return NULL;
    }

    item = cJSON_CreateObject();
    if (!authentication_info_result->auth_type) {
        ogs_error("OpenAPI_authentication_info_result_convertToJSON() failed [auth_type]");
        goto end;
    }
    cJSON *auth_type_local_JSON = OpenAPI_auth_type_convertToJSON(authentication_info_result->auth_type);
    if (auth_type_local_JSON == NULL) {
        ogs_error("OpenAPI_authentication_info_result_convertToJSON() failed [auth_type]");
        goto end;
    }
    cJSON_AddItemToObject(item, "authType", auth_type_local_JSON);
    if (item->child == NULL) {
        ogs_error("OpenAPI_authentication_info_result_convertToJSON() failed [auth_type]");
        goto end;
    }

    if (authentication_info_result->supported_features) {
        if (cJSON_AddStringToObject(item, "supportedFeatures", authentication_info_result->supported_features) == NULL) {
            ogs_error("OpenAPI_authentication_info_result_convertToJSON() failed [supported_features]");
            goto end;
        }
    }

    if (authentication_info_result->authentication_vector) {
        cJSON *authentication_vector_local_JSON = OpenAPI_authentication_vector_convertToJSON(authentication_info_result->authentication_vector);
        if (authentication_vector_local_JSON == NULL) {
            ogs_error("OpenAPI_authentication_info_result_convertToJSON() failed [authentication_vector]");
            goto end;
        }
        cJSON_AddItemToObject(item, "authenticationVector", authentication_vector_local_JSON);
        if (item->child == NULL) {
            ogs_error("OpenAPI_authentication_info_result_convertToJSON() failed [authentication_vector]");
            goto end;
        }
    }

    if (authentication_info_result->supi) {
        if (cJSON_AddStringToObject(item, "supi", authentication_info_result->supi) == NULL) {
            ogs_error("OpenAPI_authentication_info_result_convertToJSON() failed [supi]");
            goto end;
        }
    }

end:
    return item;
}

OpenAPI_authentication_info_result_t *OpenAPI_authentication_info_result_parseFromJSON(cJSON *authentication_info_resultJSON)
{
    OpenAPI_authentication_info_result_t *authentication_info_result_local_var = NULL;
    cJSON *auth_type = cJSON_GetObjectItemCaseSensitive(authentication_info_resultJSON, "authType");
    if (!auth_type) {
        ogs_error("OpenAPI_authentication_info_result_parseFromJSON() failed [auth_type]");
        goto end;
    }

    OpenAPI_auth_type_t *auth_type_local_nonprim = NULL;

    auth_type_local_nonprim = OpenAPI_auth_type_parseFromJSON(auth_type);

    cJSON *supported_features = cJSON_GetObjectItemCaseSensitive(authentication_info_resultJSON, "supportedFeatures");

    if (supported_features) {
        if (!cJSON_IsString(supported_features)) {
            ogs_error("OpenAPI_authentication_info_result_parseFromJSON() failed [supported_features]");
            goto end;
        }
    }

    cJSON *authentication_vector = cJSON_GetObjectItemCaseSensitive(authentication_info_resultJSON, "authenticationVector");

    OpenAPI_authentication_vector_t *authentication_vector_local_nonprim = NULL;
    if (authentication_vector) {
        authentication_vector_local_nonprim = OpenAPI_authentication_vector_parseFromJSON(authentication_vector);
    }

    cJSON *supi = cJSON_GetObjectItemCaseSensitive(authentication_info_resultJSON, "supi");

    if (supi) {
        if (!cJSON_IsString(supi)) {
            ogs_error("OpenAPI_authentication_info_result_parseFromJSON() failed [supi]");
            goto end;
        }
    }

    authentication_info_result_local_var = OpenAPI_authentication_info_result_create (
        auth_type_local_nonprim,
        supported_features ? ogs_strdup(supported_features->valuestring) : NULL,
        authentication_vector ? authentication_vector_local_nonprim : NULL,
        supi ? ogs_strdup(supi->valuestring) : NULL
        );

    return authentication_info_result_local_var;
end:
    return NULL;
}

