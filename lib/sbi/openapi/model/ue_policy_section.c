
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ue_policy_section.h"

OpenAPI_ue_policy_section_t *OpenAPI_ue_policy_section_create(
    char ue_policy_section_info,
    char *upsi
    )
{
    OpenAPI_ue_policy_section_t *ue_policy_section_local_var = OpenAPI_malloc(sizeof(OpenAPI_ue_policy_section_t));
    if (!ue_policy_section_local_var) {
        return NULL;
    }
    ue_policy_section_local_var->ue_policy_section_info = ue_policy_section_info;
    ue_policy_section_local_var->upsi = upsi;

    return ue_policy_section_local_var;
}

void OpenAPI_ue_policy_section_free(OpenAPI_ue_policy_section_t *ue_policy_section)
{
    if (NULL == ue_policy_section) {
        return;
    }
    OpenAPI_lnode_t *node;
    ogs_free(ue_policy_section->upsi);
    ogs_free(ue_policy_section);
}

cJSON *OpenAPI_ue_policy_section_convertToJSON(OpenAPI_ue_policy_section_t *ue_policy_section)
{
    cJSON *item = NULL;

    if (ue_policy_section == NULL) {
        ogs_error("OpenAPI_ue_policy_section_convertToJSON() failed [UePolicySection]");
        return NULL;
    }

    item = cJSON_CreateObject();
    if (!ue_policy_section->ue_policy_section_info) {
        ogs_error("OpenAPI_ue_policy_section_convertToJSON() failed [ue_policy_section_info]");
        goto end;
    }
    if (cJSON_AddNumberToObject(item, "uePolicySectionInfo", ue_policy_section->ue_policy_section_info) == NULL) {
        ogs_error("OpenAPI_ue_policy_section_convertToJSON() failed [ue_policy_section_info]");
        goto end;
    }

    if (!ue_policy_section->upsi) {
        ogs_error("OpenAPI_ue_policy_section_convertToJSON() failed [upsi]");
        goto end;
    }
    if (cJSON_AddStringToObject(item, "upsi", ue_policy_section->upsi) == NULL) {
        ogs_error("OpenAPI_ue_policy_section_convertToJSON() failed [upsi]");
        goto end;
    }

end:
    return item;
}

OpenAPI_ue_policy_section_t *OpenAPI_ue_policy_section_parseFromJSON(cJSON *ue_policy_sectionJSON)
{
    OpenAPI_ue_policy_section_t *ue_policy_section_local_var = NULL;
    cJSON *ue_policy_section_info = cJSON_GetObjectItemCaseSensitive(ue_policy_sectionJSON, "uePolicySectionInfo");
    if (!ue_policy_section_info) {
        ogs_error("OpenAPI_ue_policy_section_parseFromJSON() failed [ue_policy_section_info]");
        goto end;
    }


    if (!cJSON_IsNumber(ue_policy_section_info)) {
        ogs_error("OpenAPI_ue_policy_section_parseFromJSON() failed [ue_policy_section_info]");
        goto end;
    }

    cJSON *upsi = cJSON_GetObjectItemCaseSensitive(ue_policy_sectionJSON, "upsi");
    if (!upsi) {
        ogs_error("OpenAPI_ue_policy_section_parseFromJSON() failed [upsi]");
        goto end;
    }


    if (!cJSON_IsString(upsi)) {
        ogs_error("OpenAPI_ue_policy_section_parseFromJSON() failed [upsi]");
        goto end;
    }

    ue_policy_section_local_var = OpenAPI_ue_policy_section_create (
        ue_policy_section_info->valueint,
        ogs_strdup(upsi->valuestring)
        );

    return ue_policy_section_local_var;
end:
    return NULL;
}

