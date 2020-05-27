/*
 * av_type.h
 *
 *
 */

#ifndef _OpenAPI_av_type_H_
#define _OpenAPI_av_type_H_

#include <string.h>
#include "../external/cJSON.h"
#include "../include/list.h"
#include "../include/keyValuePair.h"
#include "../include/binary.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OpenAPI_av_type_s OpenAPI_av_type_t;
typedef struct OpenAPI_av_type_s {
} OpenAPI_av_type_t;

OpenAPI_av_type_t *OpenAPI_av_type_create(
    );
void OpenAPI_av_type_free(OpenAPI_av_type_t *av_type);
OpenAPI_av_type_t *OpenAPI_av_type_parseFromJSON(cJSON *av_typeJSON);
cJSON *OpenAPI_av_type_convertToJSON(OpenAPI_av_type_t *av_type);

#ifdef __cplusplus
}
#endif

#endif /* _OpenAPI_av_type_H_ */

