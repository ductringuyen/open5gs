/*
 * rg_auth_ctx.h
 *
 *
 */

#ifndef _OpenAPI_rg_auth_ctx_H_
#define _OpenAPI_rg_auth_ctx_H_

#include <string.h>
#include "../external/cJSON.h"
#include "../include/list.h"
#include "../include/keyValuePair.h"
#include "../include/binary.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OpenAPI_rg_auth_ctx_s OpenAPI_rg_auth_ctx_t;
typedef struct OpenAPI_rg_auth_ctx_s {
    int auth_ind;
    char *supi;
    char *supported_features;
} OpenAPI_rg_auth_ctx_t;

OpenAPI_rg_auth_ctx_t *OpenAPI_rg_auth_ctx_create(
    int auth_ind,
    char *supi,
    char *supported_features
    );
void OpenAPI_rg_auth_ctx_free(OpenAPI_rg_auth_ctx_t *rg_auth_ctx);
OpenAPI_rg_auth_ctx_t *OpenAPI_rg_auth_ctx_parseFromJSON(cJSON *rg_auth_ctxJSON);
cJSON *OpenAPI_rg_auth_ctx_convertToJSON(OpenAPI_rg_auth_ctx_t *rg_auth_ctx);

#ifdef __cplusplus
}
#endif

#endif /* _OpenAPI_rg_auth_ctx_H_ */

