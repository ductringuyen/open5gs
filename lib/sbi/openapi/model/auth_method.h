/*
 * auth_method.h
 *
 *
 */

#ifndef _OpenAPI_auth_method_H_
#define _OpenAPI_auth_method_H_

#include <string.h>
#include "../external/cJSON.h"
#include "../include/list.h"
#include "../include/keyValuePair.h"
#include "../include/binary.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OpenAPI_auth_method_s OpenAPI_auth_method_t;
typedef struct OpenAPI_auth_method_s {
} OpenAPI_auth_method_t;

OpenAPI_auth_method_t *OpenAPI_auth_method_create(
    );
void OpenAPI_auth_method_free(OpenAPI_auth_method_t *auth_method);
OpenAPI_auth_method_t *OpenAPI_auth_method_parseFromJSON(cJSON *auth_methodJSON);
cJSON *OpenAPI_auth_method_convertToJSON(OpenAPI_auth_method_t *auth_method);

#ifdef __cplusplus
}
#endif

#endif /* _OpenAPI_auth_method_H_ */

