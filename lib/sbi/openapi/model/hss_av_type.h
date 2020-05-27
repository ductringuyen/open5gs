/*
 * hss_av_type.h
 *
 *
 */

#ifndef _OpenAPI_hss_av_type_H_
#define _OpenAPI_hss_av_type_H_

#include <string.h>
#include "../external/cJSON.h"
#include "../include/list.h"
#include "../include/keyValuePair.h"
#include "../include/binary.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OpenAPI_hss_av_type_s OpenAPI_hss_av_type_t;
typedef struct OpenAPI_hss_av_type_s {
} OpenAPI_hss_av_type_t;

OpenAPI_hss_av_type_t *OpenAPI_hss_av_type_create(
    );
void OpenAPI_hss_av_type_free(OpenAPI_hss_av_type_t *hss_av_type);
OpenAPI_hss_av_type_t *OpenAPI_hss_av_type_parseFromJSON(cJSON *hss_av_typeJSON);
cJSON *OpenAPI_hss_av_type_convertToJSON(OpenAPI_hss_av_type_t *hss_av_type);

#ifdef __cplusplus
}
#endif

#endif /* _OpenAPI_hss_av_type_H_ */

