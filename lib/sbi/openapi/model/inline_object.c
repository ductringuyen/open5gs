
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "inline_object.h"

OpenAPI_inline_object_t *OpenAPI_inline_object_create(
    OpenAPI_sm_context_create_data_t *json_data,
    OpenAPI_binary_t* binary_data_n1_sm_message,
    OpenAPI_binary_t* binary_data_n2_sm_information,
    OpenAPI_binary_t* binary_data_n2_sm_information_ext1
    )
{
    OpenAPI_inline_object_t *inline_object_local_var = OpenAPI_malloc(sizeof(OpenAPI_inline_object_t));
    if (!inline_object_local_var) {
        return NULL;
    }
    inline_object_local_var->json_data = json_data;
    inline_object_local_var->binary_data_n1_sm_message = binary_data_n1_sm_message;
    inline_object_local_var->binary_data_n2_sm_information = binary_data_n2_sm_information;
    inline_object_local_var->binary_data_n2_sm_information_ext1 = binary_data_n2_sm_information_ext1;

    return inline_object_local_var;
}

void OpenAPI_inline_object_free(OpenAPI_inline_object_t *inline_object)
{
    if (NULL == inline_object) {
        return;
    }
    OpenAPI_lnode_t *node;
    OpenAPI_sm_context_create_data_free(inline_object->json_data);
    ogs_free(inline_object->binary_data_n1_sm_message->data);
    ogs_free(inline_object->binary_data_n2_sm_information->data);
    ogs_free(inline_object->binary_data_n2_sm_information_ext1->data);
    ogs_free(inline_object);
}

cJSON *OpenAPI_inline_object_convertToJSON(OpenAPI_inline_object_t *inline_object)
{
    cJSON *item = NULL;

    if (inline_object == NULL) {
        ogs_error("OpenAPI_inline_object_convertToJSON() failed [inline_object]");
        return NULL;
    }

    item = cJSON_CreateObject();
    if (inline_object->json_data) {
        cJSON *json_data_local_JSON = OpenAPI_sm_context_create_data_convertToJSON(inline_object->json_data);
        if (json_data_local_JSON == NULL) {
            ogs_error("OpenAPI_inline_object_convertToJSON() failed [json_data]");
            goto end;
        }
        cJSON_AddItemToObject(item, "jsonData", json_data_local_JSON);
        if (item->child == NULL) {
            ogs_error("OpenAPI_inline_object_convertToJSON() failed [json_data]");
            goto end;
        }
    }

    if (inline_object->binary_data_n1_sm_message) {
        char* encoded_str_binary_data_n1_sm_message = OpenAPI_base64encode(inline_object->binary_data_n1_sm_message->data,inline_object->binary_data_n1_sm_message->len);
        if (cJSON_AddStringToObject(item, "binaryDataN1SmMessage", encoded_str_binary_data_n1_sm_message) == NULL) {
            ogs_error("OpenAPI_inline_object_convertToJSON() failed [binary_data_n1_sm_message]");
            goto end;
        }
        ogs_free(encoded_str_binary_data_n1_sm_message);
    }

    if (inline_object->binary_data_n2_sm_information) {
        char* encoded_str_binary_data_n2_sm_information = OpenAPI_base64encode(inline_object->binary_data_n2_sm_information->data,inline_object->binary_data_n2_sm_information->len);
        if (cJSON_AddStringToObject(item, "binaryDataN2SmInformation", encoded_str_binary_data_n2_sm_information) == NULL) {
            ogs_error("OpenAPI_inline_object_convertToJSON() failed [binary_data_n2_sm_information]");
            goto end;
        }
        ogs_free(encoded_str_binary_data_n2_sm_information);
    }

    if (inline_object->binary_data_n2_sm_information_ext1) {
        char* encoded_str_binary_data_n2_sm_information_ext1 = OpenAPI_base64encode(inline_object->binary_data_n2_sm_information_ext1->data,inline_object->binary_data_n2_sm_information_ext1->len);
        if (cJSON_AddStringToObject(item, "binaryDataN2SmInformationExt1", encoded_str_binary_data_n2_sm_information_ext1) == NULL) {
            ogs_error("OpenAPI_inline_object_convertToJSON() failed [binary_data_n2_sm_information_ext1]");
            goto end;
        }
        ogs_free(encoded_str_binary_data_n2_sm_information_ext1);
    }

end:
    return item;
}

OpenAPI_inline_object_t *OpenAPI_inline_object_parseFromJSON(cJSON *inline_objectJSON)
{
    OpenAPI_inline_object_t *inline_object_local_var = NULL;
    cJSON *json_data = cJSON_GetObjectItemCaseSensitive(inline_objectJSON, "jsonData");

    OpenAPI_sm_context_create_data_t *json_data_local_nonprim = NULL;
    if (json_data) {
        json_data_local_nonprim = OpenAPI_sm_context_create_data_parseFromJSON(json_data);
    }

    cJSON *binary_data_n1_sm_message = cJSON_GetObjectItemCaseSensitive(inline_objectJSON, "binaryDataN1SmMessage");

    OpenAPI_binary_t* decoded_str_binary_data_n1_sm_message = OpenAPI_malloc(sizeof(OpenAPI_binary_t));
    if (binary_data_n1_sm_message) {
        if (!cJSON_IsString(binary_data_n1_sm_message)) {
            ogs_error("OpenAPI_inline_object_parseFromJSON() failed [binary_data_n1_sm_message]");
            goto end;
        }
        decoded_str_binary_data_n1_sm_message->data = OpenAPI_base64decode(binary_data_n1_sm_message->valuestring, strlen(binary_data_n1_sm_message->valuestring), &decoded_str_binary_data_n1_sm_message->len);
        if (!decoded_str_binary_data_n1_sm_message->data) {
            ogs_error("OpenAPI_inline_object_parseFromJSON() failed [binary_data_n1_sm_message]");
            goto end;
        }
    }

    cJSON *binary_data_n2_sm_information = cJSON_GetObjectItemCaseSensitive(inline_objectJSON, "binaryDataN2SmInformation");

    OpenAPI_binary_t* decoded_str_binary_data_n2_sm_information = OpenAPI_malloc(sizeof(OpenAPI_binary_t));
    if (binary_data_n2_sm_information) {
        if (!cJSON_IsString(binary_data_n2_sm_information)) {
            ogs_error("OpenAPI_inline_object_parseFromJSON() failed [binary_data_n2_sm_information]");
            goto end;
        }
        decoded_str_binary_data_n2_sm_information->data = OpenAPI_base64decode(binary_data_n2_sm_information->valuestring, strlen(binary_data_n2_sm_information->valuestring), &decoded_str_binary_data_n2_sm_information->len);
        if (!decoded_str_binary_data_n2_sm_information->data) {
            ogs_error("OpenAPI_inline_object_parseFromJSON() failed [binary_data_n2_sm_information]");
            goto end;
        }
    }

    cJSON *binary_data_n2_sm_information_ext1 = cJSON_GetObjectItemCaseSensitive(inline_objectJSON, "binaryDataN2SmInformationExt1");

    OpenAPI_binary_t* decoded_str_binary_data_n2_sm_information_ext1 = OpenAPI_malloc(sizeof(OpenAPI_binary_t));
    if (binary_data_n2_sm_information_ext1) {
        if (!cJSON_IsString(binary_data_n2_sm_information_ext1)) {
            ogs_error("OpenAPI_inline_object_parseFromJSON() failed [binary_data_n2_sm_information_ext1]");
            goto end;
        }
        decoded_str_binary_data_n2_sm_information_ext1->data = OpenAPI_base64decode(binary_data_n2_sm_information_ext1->valuestring, strlen(binary_data_n2_sm_information_ext1->valuestring), &decoded_str_binary_data_n2_sm_information_ext1->len);
        if (!decoded_str_binary_data_n2_sm_information_ext1->data) {
            ogs_error("OpenAPI_inline_object_parseFromJSON() failed [binary_data_n2_sm_information_ext1]");
            goto end;
        }
    }

    inline_object_local_var = OpenAPI_inline_object_create (
        json_data ? json_data_local_nonprim : NULL,
        binary_data_n1_sm_message ? decoded_str_binary_data_n1_sm_message : NULL,
        binary_data_n2_sm_information ? decoded_str_binary_data_n2_sm_information : NULL,
        binary_data_n2_sm_information_ext1 ? decoded_str_binary_data_n2_sm_information_ext1 : NULL
        );

    return inline_object_local_var;
end:
    return NULL;
}

OpenAPI_inline_object_t *OpenAPI_inline_object_copy(OpenAPI_inline_object_t *dst, OpenAPI_inline_object_t *src)
{
    cJSON *item = NULL;
    char *content = NULL;

    ogs_assert(src);
    item = OpenAPI_inline_object_convertToJSON(src);
    if (!item) {
        ogs_error("OpenAPI_inline_object_convertToJSON() failed");
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

    OpenAPI_inline_object_free(dst);
    dst = OpenAPI_inline_object_parseFromJSON(item);
    cJSON_Delete(item);

    return dst;
}

