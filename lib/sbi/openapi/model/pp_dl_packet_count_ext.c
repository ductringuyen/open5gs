
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "pp_dl_packet_count_ext.h"

OpenAPI_pp_dl_packet_count_ext_t *OpenAPI_pp_dl_packet_count_ext_create(
    char *af_instance_id,
    int reference_id,
    char *validity_time
    )
{
    OpenAPI_pp_dl_packet_count_ext_t *pp_dl_packet_count_ext_local_var = OpenAPI_malloc(sizeof(OpenAPI_pp_dl_packet_count_ext_t));
    if (!pp_dl_packet_count_ext_local_var) {
        return NULL;
    }
    pp_dl_packet_count_ext_local_var->af_instance_id = af_instance_id;
    pp_dl_packet_count_ext_local_var->reference_id = reference_id;
    pp_dl_packet_count_ext_local_var->validity_time = validity_time;

    return pp_dl_packet_count_ext_local_var;
}

void OpenAPI_pp_dl_packet_count_ext_free(OpenAPI_pp_dl_packet_count_ext_t *pp_dl_packet_count_ext)
{
    if (NULL == pp_dl_packet_count_ext) {
        return;
    }
    OpenAPI_lnode_t *node;
    ogs_free(pp_dl_packet_count_ext->af_instance_id);
    ogs_free(pp_dl_packet_count_ext->validity_time);
    ogs_free(pp_dl_packet_count_ext);
}

cJSON *OpenAPI_pp_dl_packet_count_ext_convertToJSON(OpenAPI_pp_dl_packet_count_ext_t *pp_dl_packet_count_ext)
{
    cJSON *item = NULL;

    if (pp_dl_packet_count_ext == NULL) {
        ogs_error("OpenAPI_pp_dl_packet_count_ext_convertToJSON() failed [PpDlPacketCountExt]");
        return NULL;
    }

    item = cJSON_CreateObject();
    if (!pp_dl_packet_count_ext->af_instance_id) {
        ogs_error("OpenAPI_pp_dl_packet_count_ext_convertToJSON() failed [af_instance_id]");
        goto end;
    }
    if (cJSON_AddStringToObject(item, "afInstanceId", pp_dl_packet_count_ext->af_instance_id) == NULL) {
        ogs_error("OpenAPI_pp_dl_packet_count_ext_convertToJSON() failed [af_instance_id]");
        goto end;
    }

    if (!pp_dl_packet_count_ext->reference_id) {
        ogs_error("OpenAPI_pp_dl_packet_count_ext_convertToJSON() failed [reference_id]");
        goto end;
    }
    if (cJSON_AddNumberToObject(item, "referenceId", pp_dl_packet_count_ext->reference_id) == NULL) {
        ogs_error("OpenAPI_pp_dl_packet_count_ext_convertToJSON() failed [reference_id]");
        goto end;
    }

    if (pp_dl_packet_count_ext->validity_time) {
        if (cJSON_AddStringToObject(item, "validityTime", pp_dl_packet_count_ext->validity_time) == NULL) {
            ogs_error("OpenAPI_pp_dl_packet_count_ext_convertToJSON() failed [validity_time]");
            goto end;
        }
    }

end:
    return item;
}

OpenAPI_pp_dl_packet_count_ext_t *OpenAPI_pp_dl_packet_count_ext_parseFromJSON(cJSON *pp_dl_packet_count_extJSON)
{
    OpenAPI_pp_dl_packet_count_ext_t *pp_dl_packet_count_ext_local_var = NULL;
    cJSON *af_instance_id = cJSON_GetObjectItemCaseSensitive(pp_dl_packet_count_extJSON, "afInstanceId");
    if (!af_instance_id) {
        ogs_error("OpenAPI_pp_dl_packet_count_ext_parseFromJSON() failed [af_instance_id]");
        goto end;
    }


    if (!cJSON_IsString(af_instance_id)) {
        ogs_error("OpenAPI_pp_dl_packet_count_ext_parseFromJSON() failed [af_instance_id]");
        goto end;
    }

    cJSON *reference_id = cJSON_GetObjectItemCaseSensitive(pp_dl_packet_count_extJSON, "referenceId");
    if (!reference_id) {
        ogs_error("OpenAPI_pp_dl_packet_count_ext_parseFromJSON() failed [reference_id]");
        goto end;
    }


    if (!cJSON_IsNumber(reference_id)) {
        ogs_error("OpenAPI_pp_dl_packet_count_ext_parseFromJSON() failed [reference_id]");
        goto end;
    }

    cJSON *validity_time = cJSON_GetObjectItemCaseSensitive(pp_dl_packet_count_extJSON, "validityTime");

    if (validity_time) {
        if (!cJSON_IsString(validity_time)) {
            ogs_error("OpenAPI_pp_dl_packet_count_ext_parseFromJSON() failed [validity_time]");
            goto end;
        }
    }

    pp_dl_packet_count_ext_local_var = OpenAPI_pp_dl_packet_count_ext_create (
        ogs_strdup(af_instance_id->valuestring),
        reference_id->valuedouble,
        validity_time ? ogs_strdup(validity_time->valuestring) : NULL
        );

    return pp_dl_packet_count_ext_local_var;
end:
    return NULL;
}

