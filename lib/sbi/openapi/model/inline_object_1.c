
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "inline_object_1.h"

OpenAPI_inline_object_1_t *OpenAPI_inline_object_1_create(
    OpenAPI_send_mo_data_req_data_t *json_data,
    OpenAPI_binary_t* binary_mo_data
    )
{
    OpenAPI_inline_object_1_t *inline_object_1_local_var = OpenAPI_malloc(sizeof(OpenAPI_inline_object_1_t));
    if (!inline_object_1_local_var) {
        return NULL;
    }
    inline_object_1_local_var->json_data = json_data;
    inline_object_1_local_var->binary_mo_data = binary_mo_data;

    return inline_object_1_local_var;
}

void OpenAPI_inline_object_1_free(OpenAPI_inline_object_1_t *inline_object_1)
{
    if (NULL == inline_object_1) {
        return;
    }
    OpenAPI_lnode_t *node;
    OpenAPI_send_mo_data_req_data_free(inline_object_1->json_data);
    ogs_free(inline_object_1->binary_mo_data->data);
    ogs_free(inline_object_1);
}

cJSON *OpenAPI_inline_object_1_convertToJSON(OpenAPI_inline_object_1_t *inline_object_1)
{
    cJSON *item = NULL;

    if (inline_object_1 == NULL) {
        ogs_error("OpenAPI_inline_object_1_convertToJSON() failed [inline_object_1]");
        return NULL;
    }

    item = cJSON_CreateObject();
    if (inline_object_1->json_data) {
        cJSON *json_data_local_JSON = OpenAPI_send_mo_data_req_data_convertToJSON(inline_object_1->json_data);
        if (json_data_local_JSON == NULL) {
            ogs_error("OpenAPI_inline_object_1_convertToJSON() failed [json_data]");
            goto end;
        }
        cJSON_AddItemToObject(item, "jsonData", json_data_local_JSON);
        if (item->child == NULL) {
            ogs_error("OpenAPI_inline_object_1_convertToJSON() failed [json_data]");
            goto end;
        }
    }

    if (inline_object_1->binary_mo_data) {
        char* encoded_str_binary_mo_data = OpenAPI_base64encode(inline_object_1->binary_mo_data->data,inline_object_1->binary_mo_data->len);
        if (cJSON_AddStringToObject(item, "binaryMoData", encoded_str_binary_mo_data) == NULL) {
            ogs_error("OpenAPI_inline_object_1_convertToJSON() failed [binary_mo_data]");
            goto end;
        }
        ogs_free(encoded_str_binary_mo_data);
    }

end:
    return item;
}

OpenAPI_inline_object_1_t *OpenAPI_inline_object_1_parseFromJSON(cJSON *inline_object_1JSON)
{
    OpenAPI_inline_object_1_t *inline_object_1_local_var = NULL;
    cJSON *json_data = cJSON_GetObjectItemCaseSensitive(inline_object_1JSON, "jsonData");

    OpenAPI_send_mo_data_req_data_t *json_data_local_nonprim = NULL;
    if (json_data) {
        json_data_local_nonprim = OpenAPI_send_mo_data_req_data_parseFromJSON(json_data);
    }

    cJSON *binary_mo_data = cJSON_GetObjectItemCaseSensitive(inline_object_1JSON, "binaryMoData");

    OpenAPI_binary_t* decoded_str_binary_mo_data = OpenAPI_malloc(sizeof(OpenAPI_binary_t));
    if (binary_mo_data) {
        if (!cJSON_IsString(binary_mo_data)) {
            ogs_error("OpenAPI_inline_object_1_parseFromJSON() failed [binary_mo_data]");
            goto end;
        }
        decoded_str_binary_mo_data->data = OpenAPI_base64decode(binary_mo_data->valuestring, strlen(binary_mo_data->valuestring), &decoded_str_binary_mo_data->len);
        if (!decoded_str_binary_mo_data->data) {
            ogs_error("OpenAPI_inline_object_1_parseFromJSON() failed [binary_mo_data]");
            goto end;
        }
    }

    inline_object_1_local_var = OpenAPI_inline_object_1_create (
        json_data ? json_data_local_nonprim : NULL,
        binary_mo_data ? decoded_str_binary_mo_data : NULL
        );

    return inline_object_1_local_var;
end:
    return NULL;
}

OpenAPI_inline_object_1_t *OpenAPI_inline_object_1_copy(OpenAPI_inline_object_1_t *dst, OpenAPI_inline_object_1_t *src)
{
    cJSON *item = NULL;
    char *content = NULL;

    ogs_assert(src);
    item = OpenAPI_inline_object_1_convertToJSON(src);
    if (!item) {
        ogs_error("OpenAPI_inline_object_1_convertToJSON() failed");
        return NULL;
    }

    content = cJSON_Print(item);
    cJSON_Delete(item);

    if (!content) {
        ogs_error("cJSON_Print() failed");
        return NULL;
    }

    item = cJSON_Parse(content);
    ogs_free(content);
    if (!item) {
        ogs_error("cJSON_Parse() failed");
        return NULL;
    }

    OpenAPI_inline_object_1_free(dst);
    dst = OpenAPI_inline_object_1_parseFromJSON(item);
    cJSON_Delete(item);

    return dst;
}

