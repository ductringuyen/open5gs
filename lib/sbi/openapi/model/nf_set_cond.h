/*
 * nf_set_cond.h
 *
 * Subscription to a set of NFs based on their Set Id
 */

#ifndef _OpenAPI_nf_set_cond_H_
#define _OpenAPI_nf_set_cond_H_

#include <string.h>
#include "../external/cJSON.h"
#include "../include/list.h"
#include "../include/keyValuePair.h"
#include "../include/binary.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OpenAPI_nf_set_cond_s OpenAPI_nf_set_cond_t;
typedef struct OpenAPI_nf_set_cond_s {
    char *nf_set_id;
} OpenAPI_nf_set_cond_t;

OpenAPI_nf_set_cond_t *OpenAPI_nf_set_cond_create(
    char *nf_set_id
    );
void OpenAPI_nf_set_cond_free(OpenAPI_nf_set_cond_t *nf_set_cond);
OpenAPI_nf_set_cond_t *OpenAPI_nf_set_cond_parseFromJSON(cJSON *nf_set_condJSON);
cJSON *OpenAPI_nf_set_cond_convertToJSON(OpenAPI_nf_set_cond_t *nf_set_cond);

#ifdef __cplusplus
}
#endif

#endif /* _OpenAPI_nf_set_cond_H_ */

