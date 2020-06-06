
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "amf_non3_gpp_access_registration_modification.h"

OpenAPI_amf_non3_gpp_access_registration_modification_t *OpenAPI_amf_non3_gpp_access_registration_modification_create(
    OpenAPI_guami_t *guami,
    int purge_flag,
    char *pei,
    OpenAPI_ims_vo_ps_t *ims_vo_ps,
    OpenAPI_list_t *backup_amf_info
    )
{
    OpenAPI_amf_non3_gpp_access_registration_modification_t *amf_non3_gpp_access_registration_modification_local_var = OpenAPI_malloc(sizeof(OpenAPI_amf_non3_gpp_access_registration_modification_t));
    if (!amf_non3_gpp_access_registration_modification_local_var) {
        return NULL;
    }
    amf_non3_gpp_access_registration_modification_local_var->guami = guami;
    amf_non3_gpp_access_registration_modification_local_var->purge_flag = purge_flag;
    amf_non3_gpp_access_registration_modification_local_var->pei = pei;
    amf_non3_gpp_access_registration_modification_local_var->ims_vo_ps = ims_vo_ps;
    amf_non3_gpp_access_registration_modification_local_var->backup_amf_info = backup_amf_info;

    return amf_non3_gpp_access_registration_modification_local_var;
}

void OpenAPI_amf_non3_gpp_access_registration_modification_free(OpenAPI_amf_non3_gpp_access_registration_modification_t *amf_non3_gpp_access_registration_modification)
{
    if (NULL == amf_non3_gpp_access_registration_modification) {
        return;
    }
    OpenAPI_lnode_t *node;
    OpenAPI_guami_free(amf_non3_gpp_access_registration_modification->guami);
    ogs_free(amf_non3_gpp_access_registration_modification->pei);
    OpenAPI_ims_vo_ps_free(amf_non3_gpp_access_registration_modification->ims_vo_ps);
    OpenAPI_list_for_each(amf_non3_gpp_access_registration_modification->backup_amf_info, node) {
        OpenAPI_backup_amf_info_free(node->data);
    }
    OpenAPI_list_free(amf_non3_gpp_access_registration_modification->backup_amf_info);
    ogs_free(amf_non3_gpp_access_registration_modification);
}

cJSON *OpenAPI_amf_non3_gpp_access_registration_modification_convertToJSON(OpenAPI_amf_non3_gpp_access_registration_modification_t *amf_non3_gpp_access_registration_modification)
{
    cJSON *item = NULL;

    if (amf_non3_gpp_access_registration_modification == NULL) {
        ogs_error("OpenAPI_amf_non3_gpp_access_registration_modification_convertToJSON() failed [AmfNon3GppAccessRegistrationModification]");
        return NULL;
    }

    item = cJSON_CreateObject();
    if (!amf_non3_gpp_access_registration_modification->guami) {
        ogs_error("OpenAPI_amf_non3_gpp_access_registration_modification_convertToJSON() failed [guami]");
        goto end;
    }
    cJSON *guami_local_JSON = OpenAPI_guami_convertToJSON(amf_non3_gpp_access_registration_modification->guami);
    if (guami_local_JSON == NULL) {
        ogs_error("OpenAPI_amf_non3_gpp_access_registration_modification_convertToJSON() failed [guami]");
        goto end;
    }
    cJSON_AddItemToObject(item, "guami", guami_local_JSON);
    if (item->child == NULL) {
        ogs_error("OpenAPI_amf_non3_gpp_access_registration_modification_convertToJSON() failed [guami]");
        goto end;
    }

    if (amf_non3_gpp_access_registration_modification->purge_flag >= 0) {
        if (cJSON_AddBoolToObject(item, "purgeFlag", amf_non3_gpp_access_registration_modification->purge_flag) == NULL) {
            ogs_error("OpenAPI_amf_non3_gpp_access_registration_modification_convertToJSON() failed [purge_flag]");
            goto end;
        }
    }

    if (amf_non3_gpp_access_registration_modification->pei) {
        if (cJSON_AddStringToObject(item, "pei", amf_non3_gpp_access_registration_modification->pei) == NULL) {
            ogs_error("OpenAPI_amf_non3_gpp_access_registration_modification_convertToJSON() failed [pei]");
            goto end;
        }
    }

    if (amf_non3_gpp_access_registration_modification->ims_vo_ps) {
        cJSON *ims_vo_ps_local_JSON = OpenAPI_ims_vo_ps_convertToJSON(amf_non3_gpp_access_registration_modification->ims_vo_ps);
        if (ims_vo_ps_local_JSON == NULL) {
            ogs_error("OpenAPI_amf_non3_gpp_access_registration_modification_convertToJSON() failed [ims_vo_ps]");
            goto end;
        }
        cJSON_AddItemToObject(item, "imsVoPs", ims_vo_ps_local_JSON);
        if (item->child == NULL) {
            ogs_error("OpenAPI_amf_non3_gpp_access_registration_modification_convertToJSON() failed [ims_vo_ps]");
            goto end;
        }
    }

    if (amf_non3_gpp_access_registration_modification->backup_amf_info) {
        cJSON *backup_amf_infoList = cJSON_AddArrayToObject(item, "backupAmfInfo");
        if (backup_amf_infoList == NULL) {
            ogs_error("OpenAPI_amf_non3_gpp_access_registration_modification_convertToJSON() failed [backup_amf_info]");
            goto end;
        }

        OpenAPI_lnode_t *backup_amf_info_node;
        if (amf_non3_gpp_access_registration_modification->backup_amf_info) {
            OpenAPI_list_for_each(amf_non3_gpp_access_registration_modification->backup_amf_info, backup_amf_info_node) {
                cJSON *itemLocal = OpenAPI_backup_amf_info_convertToJSON(backup_amf_info_node->data);
                if (itemLocal == NULL) {
                    ogs_error("OpenAPI_amf_non3_gpp_access_registration_modification_convertToJSON() failed [backup_amf_info]");
                    goto end;
                }
                cJSON_AddItemToArray(backup_amf_infoList, itemLocal);
            }
        }
    }

end:
    return item;
}

OpenAPI_amf_non3_gpp_access_registration_modification_t *OpenAPI_amf_non3_gpp_access_registration_modification_parseFromJSON(cJSON *amf_non3_gpp_access_registration_modificationJSON)
{
    OpenAPI_amf_non3_gpp_access_registration_modification_t *amf_non3_gpp_access_registration_modification_local_var = NULL;
    cJSON *guami = cJSON_GetObjectItemCaseSensitive(amf_non3_gpp_access_registration_modificationJSON, "guami");
    if (!guami) {
        ogs_error("OpenAPI_amf_non3_gpp_access_registration_modification_parseFromJSON() failed [guami]");
        goto end;
    }

    OpenAPI_guami_t *guami_local_nonprim = NULL;

    guami_local_nonprim = OpenAPI_guami_parseFromJSON(guami);

    cJSON *purge_flag = cJSON_GetObjectItemCaseSensitive(amf_non3_gpp_access_registration_modificationJSON, "purgeFlag");

    if (purge_flag) {
        if (!cJSON_IsBool(purge_flag)) {
            ogs_error("OpenAPI_amf_non3_gpp_access_registration_modification_parseFromJSON() failed [purge_flag]");
            goto end;
        }
    }

    cJSON *pei = cJSON_GetObjectItemCaseSensitive(amf_non3_gpp_access_registration_modificationJSON, "pei");

    if (pei) {
        if (!cJSON_IsString(pei)) {
            ogs_error("OpenAPI_amf_non3_gpp_access_registration_modification_parseFromJSON() failed [pei]");
            goto end;
        }
    }

    cJSON *ims_vo_ps = cJSON_GetObjectItemCaseSensitive(amf_non3_gpp_access_registration_modificationJSON, "imsVoPs");

    OpenAPI_ims_vo_ps_t *ims_vo_ps_local_nonprim = NULL;
    if (ims_vo_ps) {
        ims_vo_ps_local_nonprim = OpenAPI_ims_vo_ps_parseFromJSON(ims_vo_ps);
    }

    cJSON *backup_amf_info = cJSON_GetObjectItemCaseSensitive(amf_non3_gpp_access_registration_modificationJSON, "backupAmfInfo");

    OpenAPI_list_t *backup_amf_infoList;
    if (backup_amf_info) {
        cJSON *backup_amf_info_local_nonprimitive;
        if (!cJSON_IsArray(backup_amf_info)) {
            ogs_error("OpenAPI_amf_non3_gpp_access_registration_modification_parseFromJSON() failed [backup_amf_info]");
            goto end;
        }

        backup_amf_infoList = OpenAPI_list_create();

        cJSON_ArrayForEach(backup_amf_info_local_nonprimitive, backup_amf_info ) {
            if (!cJSON_IsObject(backup_amf_info_local_nonprimitive)) {
                ogs_error("OpenAPI_amf_non3_gpp_access_registration_modification_parseFromJSON() failed [backup_amf_info]");
                goto end;
            }
            OpenAPI_backup_amf_info_t *backup_amf_infoItem = OpenAPI_backup_amf_info_parseFromJSON(backup_amf_info_local_nonprimitive);

            OpenAPI_list_add(backup_amf_infoList, backup_amf_infoItem);
        }
    }

    amf_non3_gpp_access_registration_modification_local_var = OpenAPI_amf_non3_gpp_access_registration_modification_create (
        guami_local_nonprim,
        purge_flag ? purge_flag->valueint : -1,
        pei ? ogs_strdup(pei->valuestring) : NULL,
        ims_vo_ps ? ims_vo_ps_local_nonprim : NULL,
        backup_amf_info ? backup_amf_infoList : NULL
        );

    return amf_non3_gpp_access_registration_modification_local_var;
end:
    return NULL;
}

