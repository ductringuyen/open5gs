
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "battery_indication.h"

OpenAPI_battery_indication_t *OpenAPI_battery_indication_create(
    int battery_ind,
    int replaceable_ind,
    int rechargeable_ind
    )
{
    OpenAPI_battery_indication_t *battery_indication_local_var = OpenAPI_malloc(sizeof(OpenAPI_battery_indication_t));
    if (!battery_indication_local_var) {
        return NULL;
    }
    battery_indication_local_var->battery_ind = battery_ind;
    battery_indication_local_var->replaceable_ind = replaceable_ind;
    battery_indication_local_var->rechargeable_ind = rechargeable_ind;

    return battery_indication_local_var;
}

void OpenAPI_battery_indication_free(OpenAPI_battery_indication_t *battery_indication)
{
    if (NULL == battery_indication) {
        return;
    }
    OpenAPI_lnode_t *node;
    ogs_free(battery_indication);
}

cJSON *OpenAPI_battery_indication_convertToJSON(OpenAPI_battery_indication_t *battery_indication)
{
    cJSON *item = NULL;

    if (battery_indication == NULL) {
        ogs_error("OpenAPI_battery_indication_convertToJSON() failed [BatteryIndication]");
        return NULL;
    }

    item = cJSON_CreateObject();
    if (battery_indication->battery_ind >= 0) {
        if (cJSON_AddBoolToObject(item, "batteryInd", battery_indication->battery_ind) == NULL) {
            ogs_error("OpenAPI_battery_indication_convertToJSON() failed [battery_ind]");
            goto end;
        }
    }

    if (battery_indication->replaceable_ind >= 0) {
        if (cJSON_AddBoolToObject(item, "replaceableInd", battery_indication->replaceable_ind) == NULL) {
            ogs_error("OpenAPI_battery_indication_convertToJSON() failed [replaceable_ind]");
            goto end;
        }
    }

    if (battery_indication->rechargeable_ind >= 0) {
        if (cJSON_AddBoolToObject(item, "rechargeableInd", battery_indication->rechargeable_ind) == NULL) {
            ogs_error("OpenAPI_battery_indication_convertToJSON() failed [rechargeable_ind]");
            goto end;
        }
    }

end:
    return item;
}

OpenAPI_battery_indication_t *OpenAPI_battery_indication_parseFromJSON(cJSON *battery_indicationJSON)
{
    OpenAPI_battery_indication_t *battery_indication_local_var = NULL;
    cJSON *battery_ind = cJSON_GetObjectItemCaseSensitive(battery_indicationJSON, "batteryInd");

    if (battery_ind) {
        if (!cJSON_IsBool(battery_ind)) {
            ogs_error("OpenAPI_battery_indication_parseFromJSON() failed [battery_ind]");
            goto end;
        }
    }

    cJSON *replaceable_ind = cJSON_GetObjectItemCaseSensitive(battery_indicationJSON, "replaceableInd");

    if (replaceable_ind) {
        if (!cJSON_IsBool(replaceable_ind)) {
            ogs_error("OpenAPI_battery_indication_parseFromJSON() failed [replaceable_ind]");
            goto end;
        }
    }

    cJSON *rechargeable_ind = cJSON_GetObjectItemCaseSensitive(battery_indicationJSON, "rechargeableInd");

    if (rechargeable_ind) {
        if (!cJSON_IsBool(rechargeable_ind)) {
            ogs_error("OpenAPI_battery_indication_parseFromJSON() failed [rechargeable_ind]");
            goto end;
        }
    }

    battery_indication_local_var = OpenAPI_battery_indication_create (
        battery_ind ? battery_ind->valueint : -1,
        replaceable_ind ? replaceable_ind->valueint : -1,
        rechargeable_ind ? rechargeable_ind->valueint : -1
        );

    return battery_indication_local_var;
end:
    return NULL;
}

