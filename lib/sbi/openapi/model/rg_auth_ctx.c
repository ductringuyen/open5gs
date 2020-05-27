
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "rg_auth_ctx.h"

OpenAPI_rg_auth_ctx_t *OpenAPI_rg_auth_ctx_create(
    int auth_ind,
    char *supi,
    char *supported_features
    )
{
    OpenAPI_rg_auth_ctx_t *rg_auth_ctx_local_var = OpenAPI_malloc(sizeof(OpenAPI_rg_auth_ctx_t));
    if (!rg_auth_ctx_local_var) {
        return NULL;
    }
    rg_auth_ctx_local_var->auth_ind = auth_ind;
    rg_auth_ctx_local_var->supi = supi;
    rg_auth_ctx_local_var->supported_features = supported_features;

    return rg_auth_ctx_local_var;
}

void OpenAPI_rg_auth_ctx_free(OpenAPI_rg_auth_ctx_t *rg_auth_ctx)
{
    if (NULL == rg_auth_ctx) {
        return;
    }
    OpenAPI_lnode_t *node;
    ogs_free(rg_auth_ctx->supi);
    ogs_free(rg_auth_ctx->supported_features);
    ogs_free(rg_auth_ctx);
}

cJSON *OpenAPI_rg_auth_ctx_convertToJSON(OpenAPI_rg_auth_ctx_t *rg_auth_ctx)
{
    cJSON *item = NULL;

    if (rg_auth_ctx == NULL) {
        ogs_error("OpenAPI_rg_auth_ctx_convertToJSON() failed [RgAuthCtx]");
        return NULL;
    }

    item = cJSON_CreateObject();
    if (!rg_auth_ctx->auth_ind) {
        ogs_error("OpenAPI_rg_auth_ctx_convertToJSON() failed [auth_ind]");
        goto end;
    }
    if (cJSON_AddBoolToObject(item, "authInd", rg_auth_ctx->auth_ind) == NULL) {
        ogs_error("OpenAPI_rg_auth_ctx_convertToJSON() failed [auth_ind]");
        goto end;
    }

    if (rg_auth_ctx->supi) {
        if (cJSON_AddStringToObject(item, "supi", rg_auth_ctx->supi) == NULL) {
            ogs_error("OpenAPI_rg_auth_ctx_convertToJSON() failed [supi]");
            goto end;
        }
    }

    if (rg_auth_ctx->supported_features) {
        if (cJSON_AddStringToObject(item, "supportedFeatures", rg_auth_ctx->supported_features) == NULL) {
            ogs_error("OpenAPI_rg_auth_ctx_convertToJSON() failed [supported_features]");
            goto end;
        }
    }

end:
    return item;
}

OpenAPI_rg_auth_ctx_t *OpenAPI_rg_auth_ctx_parseFromJSON(cJSON *rg_auth_ctxJSON)
{
    OpenAPI_rg_auth_ctx_t *rg_auth_ctx_local_var = NULL;
    cJSON *auth_ind = cJSON_GetObjectItemCaseSensitive(rg_auth_ctxJSON, "authInd");
    if (!auth_ind) {
        ogs_error("OpenAPI_rg_auth_ctx_parseFromJSON() failed [auth_ind]");
        goto end;
    }


    if (!cJSON_IsBool(auth_ind)) {
        ogs_error("OpenAPI_rg_auth_ctx_parseFromJSON() failed [auth_ind]");
        goto end;
    }

    cJSON *supi = cJSON_GetObjectItemCaseSensitive(rg_auth_ctxJSON, "supi");

    if (supi) {
        if (!cJSON_IsString(supi)) {
            ogs_error("OpenAPI_rg_auth_ctx_parseFromJSON() failed [supi]");
            goto end;
        }
    }

    cJSON *supported_features = cJSON_GetObjectItemCaseSensitive(rg_auth_ctxJSON, "supportedFeatures");

    if (supported_features) {
        if (!cJSON_IsString(supported_features)) {
            ogs_error("OpenAPI_rg_auth_ctx_parseFromJSON() failed [supported_features]");
            goto end;
        }
    }

    rg_auth_ctx_local_var = OpenAPI_rg_auth_ctx_create (
        auth_ind->valueint,
        supi ? ogs_strdup(supi->valuestring) : NULL,
        supported_features ? ogs_strdup(supported_features->valuestring) : NULL
        );

    return rg_auth_ctx_local_var;
end:
    return NULL;
}

