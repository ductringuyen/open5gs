/*
 * sqn_scheme.h
 *
 *
 */

#ifndef _OpenAPI_sqn_scheme_H_
#define _OpenAPI_sqn_scheme_H_

#include <string.h>
#include "../external/cJSON.h"
#include "../include/list.h"
#include "../include/keyValuePair.h"
#include "../include/binary.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OpenAPI_sqn_scheme_s OpenAPI_sqn_scheme_t;
typedef struct OpenAPI_sqn_scheme_s {
} OpenAPI_sqn_scheme_t;

OpenAPI_sqn_scheme_t *OpenAPI_sqn_scheme_create(
    );
void OpenAPI_sqn_scheme_free(OpenAPI_sqn_scheme_t *sqn_scheme);
OpenAPI_sqn_scheme_t *OpenAPI_sqn_scheme_parseFromJSON(cJSON *sqn_schemeJSON);
cJSON *OpenAPI_sqn_scheme_convertToJSON(OpenAPI_sqn_scheme_t *sqn_scheme);

#ifdef __cplusplus
}
#endif

#endif /* _OpenAPI_sqn_scheme_H_ */

