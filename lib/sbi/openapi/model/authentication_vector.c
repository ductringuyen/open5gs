
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "authentication_vector.h"

OpenAPI_authentication_vector_t *OpenAPI_authentication_vector_create(
    OpenAPI_av_type_t *av_type,
    char *rand,
    char *xres,
    char *autn,
    char *ck_prime,
    char *ik_prime,
    char *xres_star,
    char *kausf
    )
{
    OpenAPI_authentication_vector_t *authentication_vector_local_var = OpenAPI_malloc(sizeof(OpenAPI_authentication_vector_t));
    if (!authentication_vector_local_var) {
        return NULL;
    }
    authentication_vector_local_var->av_type = av_type;
    authentication_vector_local_var->rand = rand;
    authentication_vector_local_var->xres = xres;
    authentication_vector_local_var->autn = autn;
    authentication_vector_local_var->ck_prime = ck_prime;
    authentication_vector_local_var->ik_prime = ik_prime;
    authentication_vector_local_var->xres_star = xres_star;
    authentication_vector_local_var->kausf = kausf;

    return authentication_vector_local_var;
}

void OpenAPI_authentication_vector_free(OpenAPI_authentication_vector_t *authentication_vector)
{
    if (NULL == authentication_vector) {
        return;
    }
    OpenAPI_lnode_t *node;
    OpenAPI_av_type_free(authentication_vector->av_type);
    ogs_free(authentication_vector->rand);
    ogs_free(authentication_vector->xres);
    ogs_free(authentication_vector->autn);
    ogs_free(authentication_vector->ck_prime);
    ogs_free(authentication_vector->ik_prime);
    ogs_free(authentication_vector->xres_star);
    ogs_free(authentication_vector->kausf);
    ogs_free(authentication_vector);
}

cJSON *OpenAPI_authentication_vector_convertToJSON(OpenAPI_authentication_vector_t *authentication_vector)
{
    cJSON *item = NULL;

    if (authentication_vector == NULL) {
        ogs_error("OpenAPI_authentication_vector_convertToJSON() failed [AuthenticationVector]");
        return NULL;
    }

    item = cJSON_CreateObject();
    if (!authentication_vector->av_type) {
        ogs_error("OpenAPI_authentication_vector_convertToJSON() failed [av_type]");
        goto end;
    }
    cJSON *av_type_local_JSON = OpenAPI_av_type_convertToJSON(authentication_vector->av_type);
    if (av_type_local_JSON == NULL) {
        ogs_error("OpenAPI_authentication_vector_convertToJSON() failed [av_type]");
        goto end;
    }
    cJSON_AddItemToObject(item, "avType", av_type_local_JSON);
    if (item->child == NULL) {
        ogs_error("OpenAPI_authentication_vector_convertToJSON() failed [av_type]");
        goto end;
    }

    if (!authentication_vector->rand) {
        ogs_error("OpenAPI_authentication_vector_convertToJSON() failed [rand]");
        goto end;
    }
    if (cJSON_AddStringToObject(item, "rand", authentication_vector->rand) == NULL) {
        ogs_error("OpenAPI_authentication_vector_convertToJSON() failed [rand]");
        goto end;
    }

    if (!authentication_vector->xres) {
        ogs_error("OpenAPI_authentication_vector_convertToJSON() failed [xres]");
        goto end;
    }
    if (cJSON_AddStringToObject(item, "xres", authentication_vector->xres) == NULL) {
        ogs_error("OpenAPI_authentication_vector_convertToJSON() failed [xres]");
        goto end;
    }

    if (!authentication_vector->autn) {
        ogs_error("OpenAPI_authentication_vector_convertToJSON() failed [autn]");
        goto end;
    }
    if (cJSON_AddStringToObject(item, "autn", authentication_vector->autn) == NULL) {
        ogs_error("OpenAPI_authentication_vector_convertToJSON() failed [autn]");
        goto end;
    }

    if (!authentication_vector->ck_prime) {
        ogs_error("OpenAPI_authentication_vector_convertToJSON() failed [ck_prime]");
        goto end;
    }
    if (cJSON_AddStringToObject(item, "ckPrime", authentication_vector->ck_prime) == NULL) {
        ogs_error("OpenAPI_authentication_vector_convertToJSON() failed [ck_prime]");
        goto end;
    }

    if (!authentication_vector->ik_prime) {
        ogs_error("OpenAPI_authentication_vector_convertToJSON() failed [ik_prime]");
        goto end;
    }
    if (cJSON_AddStringToObject(item, "ikPrime", authentication_vector->ik_prime) == NULL) {
        ogs_error("OpenAPI_authentication_vector_convertToJSON() failed [ik_prime]");
        goto end;
    }

    if (!authentication_vector->xres_star) {
        ogs_error("OpenAPI_authentication_vector_convertToJSON() failed [xres_star]");
        goto end;
    }
    if (cJSON_AddStringToObject(item, "xresStar", authentication_vector->xres_star) == NULL) {
        ogs_error("OpenAPI_authentication_vector_convertToJSON() failed [xres_star]");
        goto end;
    }

    if (!authentication_vector->kausf) {
        ogs_error("OpenAPI_authentication_vector_convertToJSON() failed [kausf]");
        goto end;
    }
    if (cJSON_AddStringToObject(item, "kausf", authentication_vector->kausf) == NULL) {
        ogs_error("OpenAPI_authentication_vector_convertToJSON() failed [kausf]");
        goto end;
    }

end:
    return item;
}

OpenAPI_authentication_vector_t *OpenAPI_authentication_vector_parseFromJSON(cJSON *authentication_vectorJSON)
{
    OpenAPI_authentication_vector_t *authentication_vector_local_var = NULL;
    cJSON *av_type = cJSON_GetObjectItemCaseSensitive(authentication_vectorJSON, "avType");
    if (!av_type) {
        ogs_error("OpenAPI_authentication_vector_parseFromJSON() failed [av_type]");
        goto end;
    }

    OpenAPI_av_type_t *av_type_local_nonprim = NULL;

    av_type_local_nonprim = OpenAPI_av_type_parseFromJSON(av_type);

    cJSON *rand = cJSON_GetObjectItemCaseSensitive(authentication_vectorJSON, "rand");
    if (!rand) {
        ogs_error("OpenAPI_authentication_vector_parseFromJSON() failed [rand]");
        goto end;
    }


    if (!cJSON_IsString(rand)) {
        ogs_error("OpenAPI_authentication_vector_parseFromJSON() failed [rand]");
        goto end;
    }

    cJSON *xres = cJSON_GetObjectItemCaseSensitive(authentication_vectorJSON, "xres");
    if (!xres) {
        ogs_error("OpenAPI_authentication_vector_parseFromJSON() failed [xres]");
        goto end;
    }


    if (!cJSON_IsString(xres)) {
        ogs_error("OpenAPI_authentication_vector_parseFromJSON() failed [xres]");
        goto end;
    }

    cJSON *autn = cJSON_GetObjectItemCaseSensitive(authentication_vectorJSON, "autn");
    if (!autn) {
        ogs_error("OpenAPI_authentication_vector_parseFromJSON() failed [autn]");
        goto end;
    }


    if (!cJSON_IsString(autn)) {
        ogs_error("OpenAPI_authentication_vector_parseFromJSON() failed [autn]");
        goto end;
    }

    cJSON *ck_prime = cJSON_GetObjectItemCaseSensitive(authentication_vectorJSON, "ckPrime");
    if (!ck_prime) {
        ogs_error("OpenAPI_authentication_vector_parseFromJSON() failed [ck_prime]");
        goto end;
    }


    if (!cJSON_IsString(ck_prime)) {
        ogs_error("OpenAPI_authentication_vector_parseFromJSON() failed [ck_prime]");
        goto end;
    }

    cJSON *ik_prime = cJSON_GetObjectItemCaseSensitive(authentication_vectorJSON, "ikPrime");
    if (!ik_prime) {
        ogs_error("OpenAPI_authentication_vector_parseFromJSON() failed [ik_prime]");
        goto end;
    }


    if (!cJSON_IsString(ik_prime)) {
        ogs_error("OpenAPI_authentication_vector_parseFromJSON() failed [ik_prime]");
        goto end;
    }

    cJSON *xres_star = cJSON_GetObjectItemCaseSensitive(authentication_vectorJSON, "xresStar");
    if (!xres_star) {
        ogs_error("OpenAPI_authentication_vector_parseFromJSON() failed [xres_star]");
        goto end;
    }


    if (!cJSON_IsString(xres_star)) {
        ogs_error("OpenAPI_authentication_vector_parseFromJSON() failed [xres_star]");
        goto end;
    }

    cJSON *kausf = cJSON_GetObjectItemCaseSensitive(authentication_vectorJSON, "kausf");
    if (!kausf) {
        ogs_error("OpenAPI_authentication_vector_parseFromJSON() failed [kausf]");
        goto end;
    }


    if (!cJSON_IsString(kausf)) {
        ogs_error("OpenAPI_authentication_vector_parseFromJSON() failed [kausf]");
        goto end;
    }

    authentication_vector_local_var = OpenAPI_authentication_vector_create (
        av_type_local_nonprim,
        ogs_strdup(rand->valuestring),
        ogs_strdup(xres->valuestring),
        ogs_strdup(autn->valuestring),
        ogs_strdup(ck_prime->valuestring),
        ogs_strdup(ik_prime->valuestring),
        ogs_strdup(xres_star->valuestring),
        ogs_strdup(kausf->valuestring)
        );

    return authentication_vector_local_var;
end:
    return NULL;
}

