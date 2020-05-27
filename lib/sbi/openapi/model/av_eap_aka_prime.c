
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "av_eap_aka_prime.h"

OpenAPI_av_eap_aka_prime_t *OpenAPI_av_eap_aka_prime_create(
    OpenAPI_av_type_t *av_type,
    char *rand,
    char *xres,
    char *autn,
    char *ck_prime,
    char *ik_prime
    )
{
    OpenAPI_av_eap_aka_prime_t *av_eap_aka_prime_local_var = OpenAPI_malloc(sizeof(OpenAPI_av_eap_aka_prime_t));
    if (!av_eap_aka_prime_local_var) {
        return NULL;
    }
    av_eap_aka_prime_local_var->av_type = av_type;
    av_eap_aka_prime_local_var->rand = rand;
    av_eap_aka_prime_local_var->xres = xres;
    av_eap_aka_prime_local_var->autn = autn;
    av_eap_aka_prime_local_var->ck_prime = ck_prime;
    av_eap_aka_prime_local_var->ik_prime = ik_prime;

    return av_eap_aka_prime_local_var;
}

void OpenAPI_av_eap_aka_prime_free(OpenAPI_av_eap_aka_prime_t *av_eap_aka_prime)
{
    if (NULL == av_eap_aka_prime) {
        return;
    }
    OpenAPI_lnode_t *node;
    OpenAPI_av_type_free(av_eap_aka_prime->av_type);
    ogs_free(av_eap_aka_prime->rand);
    ogs_free(av_eap_aka_prime->xres);
    ogs_free(av_eap_aka_prime->autn);
    ogs_free(av_eap_aka_prime->ck_prime);
    ogs_free(av_eap_aka_prime->ik_prime);
    ogs_free(av_eap_aka_prime);
}

cJSON *OpenAPI_av_eap_aka_prime_convertToJSON(OpenAPI_av_eap_aka_prime_t *av_eap_aka_prime)
{
    cJSON *item = NULL;

    if (av_eap_aka_prime == NULL) {
        ogs_error("OpenAPI_av_eap_aka_prime_convertToJSON() failed [AvEapAkaPrime]");
        return NULL;
    }

    item = cJSON_CreateObject();
    if (!av_eap_aka_prime->av_type) {
        ogs_error("OpenAPI_av_eap_aka_prime_convertToJSON() failed [av_type]");
        goto end;
    }
    cJSON *av_type_local_JSON = OpenAPI_av_type_convertToJSON(av_eap_aka_prime->av_type);
    if (av_type_local_JSON == NULL) {
        ogs_error("OpenAPI_av_eap_aka_prime_convertToJSON() failed [av_type]");
        goto end;
    }
    cJSON_AddItemToObject(item, "avType", av_type_local_JSON);
    if (item->child == NULL) {
        ogs_error("OpenAPI_av_eap_aka_prime_convertToJSON() failed [av_type]");
        goto end;
    }

    if (!av_eap_aka_prime->rand) {
        ogs_error("OpenAPI_av_eap_aka_prime_convertToJSON() failed [rand]");
        goto end;
    }
    if (cJSON_AddStringToObject(item, "rand", av_eap_aka_prime->rand) == NULL) {
        ogs_error("OpenAPI_av_eap_aka_prime_convertToJSON() failed [rand]");
        goto end;
    }

    if (!av_eap_aka_prime->xres) {
        ogs_error("OpenAPI_av_eap_aka_prime_convertToJSON() failed [xres]");
        goto end;
    }
    if (cJSON_AddStringToObject(item, "xres", av_eap_aka_prime->xres) == NULL) {
        ogs_error("OpenAPI_av_eap_aka_prime_convertToJSON() failed [xres]");
        goto end;
    }

    if (!av_eap_aka_prime->autn) {
        ogs_error("OpenAPI_av_eap_aka_prime_convertToJSON() failed [autn]");
        goto end;
    }
    if (cJSON_AddStringToObject(item, "autn", av_eap_aka_prime->autn) == NULL) {
        ogs_error("OpenAPI_av_eap_aka_prime_convertToJSON() failed [autn]");
        goto end;
    }

    if (!av_eap_aka_prime->ck_prime) {
        ogs_error("OpenAPI_av_eap_aka_prime_convertToJSON() failed [ck_prime]");
        goto end;
    }
    if (cJSON_AddStringToObject(item, "ckPrime", av_eap_aka_prime->ck_prime) == NULL) {
        ogs_error("OpenAPI_av_eap_aka_prime_convertToJSON() failed [ck_prime]");
        goto end;
    }

    if (!av_eap_aka_prime->ik_prime) {
        ogs_error("OpenAPI_av_eap_aka_prime_convertToJSON() failed [ik_prime]");
        goto end;
    }
    if (cJSON_AddStringToObject(item, "ikPrime", av_eap_aka_prime->ik_prime) == NULL) {
        ogs_error("OpenAPI_av_eap_aka_prime_convertToJSON() failed [ik_prime]");
        goto end;
    }

end:
    return item;
}

OpenAPI_av_eap_aka_prime_t *OpenAPI_av_eap_aka_prime_parseFromJSON(cJSON *av_eap_aka_primeJSON)
{
    OpenAPI_av_eap_aka_prime_t *av_eap_aka_prime_local_var = NULL;
    cJSON *av_type = cJSON_GetObjectItemCaseSensitive(av_eap_aka_primeJSON, "avType");
    if (!av_type) {
        ogs_error("OpenAPI_av_eap_aka_prime_parseFromJSON() failed [av_type]");
        goto end;
    }

    OpenAPI_av_type_t *av_type_local_nonprim = NULL;

    av_type_local_nonprim = OpenAPI_av_type_parseFromJSON(av_type);

    cJSON *rand = cJSON_GetObjectItemCaseSensitive(av_eap_aka_primeJSON, "rand");
    if (!rand) {
        ogs_error("OpenAPI_av_eap_aka_prime_parseFromJSON() failed [rand]");
        goto end;
    }


    if (!cJSON_IsString(rand)) {
        ogs_error("OpenAPI_av_eap_aka_prime_parseFromJSON() failed [rand]");
        goto end;
    }

    cJSON *xres = cJSON_GetObjectItemCaseSensitive(av_eap_aka_primeJSON, "xres");
    if (!xres) {
        ogs_error("OpenAPI_av_eap_aka_prime_parseFromJSON() failed [xres]");
        goto end;
    }


    if (!cJSON_IsString(xres)) {
        ogs_error("OpenAPI_av_eap_aka_prime_parseFromJSON() failed [xres]");
        goto end;
    }

    cJSON *autn = cJSON_GetObjectItemCaseSensitive(av_eap_aka_primeJSON, "autn");
    if (!autn) {
        ogs_error("OpenAPI_av_eap_aka_prime_parseFromJSON() failed [autn]");
        goto end;
    }


    if (!cJSON_IsString(autn)) {
        ogs_error("OpenAPI_av_eap_aka_prime_parseFromJSON() failed [autn]");
        goto end;
    }

    cJSON *ck_prime = cJSON_GetObjectItemCaseSensitive(av_eap_aka_primeJSON, "ckPrime");
    if (!ck_prime) {
        ogs_error("OpenAPI_av_eap_aka_prime_parseFromJSON() failed [ck_prime]");
        goto end;
    }


    if (!cJSON_IsString(ck_prime)) {
        ogs_error("OpenAPI_av_eap_aka_prime_parseFromJSON() failed [ck_prime]");
        goto end;
    }

    cJSON *ik_prime = cJSON_GetObjectItemCaseSensitive(av_eap_aka_primeJSON, "ikPrime");
    if (!ik_prime) {
        ogs_error("OpenAPI_av_eap_aka_prime_parseFromJSON() failed [ik_prime]");
        goto end;
    }


    if (!cJSON_IsString(ik_prime)) {
        ogs_error("OpenAPI_av_eap_aka_prime_parseFromJSON() failed [ik_prime]");
        goto end;
    }

    av_eap_aka_prime_local_var = OpenAPI_av_eap_aka_prime_create (
        av_type_local_nonprim,
        ogs_strdup(rand->valuestring),
        ogs_strdup(xres->valuestring),
        ogs_strdup(autn->valuestring),
        ogs_strdup(ck_prime->valuestring),
        ogs_strdup(ik_prime->valuestring)
        );

    return av_eap_aka_prime_local_var;
end:
    return NULL;
}

