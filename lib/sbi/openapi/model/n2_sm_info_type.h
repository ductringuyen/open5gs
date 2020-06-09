/*
 * n2_sm_info_type.h
 *
 * Possible values are - PDU_RES_SETUP_REQ - PDU_RES_SETUP_RSP - PDU_RES_SETUP_FAIL - PDU_RES_REL_CMD - PDU_RES_REL_RSP - PDU_RES_MOD_REQ - PDU_RES_MOD_RSP - PDU_RES_MOD_FAIL - PDU_RES_NTY - PDU_RES_NTY_REL - PDU_RES_MOD_IND - PDU_RES_MOD_CFM - PATH_SWITCH_REQ - PATH_SWITCH_SETUP_FAIL - PATH_SWITCH_REQ_ACK - PATH_SWITCH_REQ_FAIL - HANDOVER_REQUIRED - HANDOVER_CMD - HANDOVER_PREP_FAIL - HANDOVER_REQ_ACK - HANDOVER_RES_ALLOC_FAIL - SECONDARY_RAT_USAGE
 */

#ifndef _OpenAPI_n2_sm_info_type_H_
#define _OpenAPI_n2_sm_info_type_H_

#include <string.h>
#include "../external/cJSON.h"
#include "../include/list.h"
#include "../include/keyValuePair.h"
#include "../include/binary.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OpenAPI_n2_sm_info_type_s OpenAPI_n2_sm_info_type_t;
typedef struct OpenAPI_n2_sm_info_type_s {
} OpenAPI_n2_sm_info_type_t;

OpenAPI_n2_sm_info_type_t *OpenAPI_n2_sm_info_type_create(
    );
void OpenAPI_n2_sm_info_type_free(OpenAPI_n2_sm_info_type_t *n2_sm_info_type);
OpenAPI_n2_sm_info_type_t *OpenAPI_n2_sm_info_type_parseFromJSON(cJSON *n2_sm_info_typeJSON);
cJSON *OpenAPI_n2_sm_info_type_convertToJSON(OpenAPI_n2_sm_info_type_t *n2_sm_info_type);
OpenAPI_n2_sm_info_type_t *OpenAPI_n2_sm_info_type_copy(OpenAPI_n2_sm_info_type_t *dst, OpenAPI_n2_sm_info_type_t *src);

#ifdef __cplusplus
}
#endif

#endif /* _OpenAPI_n2_sm_info_type_H_ */

