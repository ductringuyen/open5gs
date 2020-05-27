/*
 * auth_type.h
 *
 *
 */

#ifndef _OpenAPI_auth_type_H_
#define _OpenAPI_auth_type_H_

#include <string.h>
#include "../external/cJSON.h"
#include "../include/list.h"
#include "../include/keyValuePair.h"
#include "../include/binary.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OpenAPI_auth_type_s OpenAPI_auth_type_t;
typedef struct OpenAPI_auth_type_s {
} OpenAPI_auth_type_t;

OpenAPI_auth_type_t *OpenAPI_auth_type_create(
    );
void OpenAPI_auth_type_free(OpenAPI_auth_type_t *auth_type);
OpenAPI_auth_type_t *OpenAPI_auth_type_parseFromJSON(cJSON *auth_typeJSON);
cJSON *OpenAPI_auth_type_convertToJSON(OpenAPI_auth_type_t *auth_type);

#ifdef __cplusplus
}
#endif

#endif /* _OpenAPI_auth_type_H_ */

