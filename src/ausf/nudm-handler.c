/*
 * Copyright (C) 2019 by Sukchan Lee <acetcom@gmail.com>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "nudm-handler.h"

static const char *links_member_name(OpenAPI_auth_type_e auth_type)
{
    if (auth_type == OpenAPI_auth_type_5G_AKA ||
        auth_type == OpenAPI_auth_type_EAP_AKA_PRIME) {
        return "5g-aka";
    } else if (auth_type == OpenAPI_auth_type_EAP_TLS) {
        return "eap_session";
    }

    ogs_assert_if_reached();
    return NULL;
}

bool ausf_nudm_ueau_handle_get(
        ogs_sbi_session_t *session, ogs_sbi_message_t *recvmsg)
{
    ausf_ue_t *ausf_ue = NULL;

    ogs_sbi_message_t sendmsg;
    ogs_sbi_response_t *response = NULL;

    uint8_t rand[OGS_RAND_LEN];
    uint8_t autn[OGS_AUTN_LEN];
    uint8_t xres_star[OGS_MAX_RES_LEN];
    uint8_t kausf[OGS_SHA256_DIGEST_SIZE];

    OpenAPI_authentication_info_result_t *AuthenticationInfoResult = NULL;
    OpenAPI_authentication_vector_t *AuthenticationVector = NULL;
    OpenAPI_ue_authentication_ctx_t UeAuthenticationCtx;
    OpenAPI_av5g_aka_t AV5G_AKA;
    OpenAPI_map_t *LinksValueScheme = NULL;
    OpenAPI_links_value_schema_t LinksValueSchemeValue;

    ogs_assert(session);
    ausf_ue = ogs_sbi_session_get_data(session);
    ogs_assert(ausf_ue);

    ogs_assert(recvmsg);

    SWITCH(recvmsg->h.resource.component[1])
    CASE(OGS_SBI_RESOURCE_NAME_SECURITY_INFORMATION)
        SWITCH(recvmsg->h.resource.component[2])
        CASE(OGS_SBI_RESOURCE_NAME_GENERATE_AUTH_DATA)

            AuthenticationInfoResult = recvmsg->AuthenticationInfoResult;
            if (!AuthenticationInfoResult) {
                ogs_error("[%s] No AuthenticationInfoResult", ausf_ue->id);
                ogs_sbi_server_send_error(session,
                        OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR,
                        recvmsg, "No AuthenticationInfoResult", ausf_ue->id);
                return false;
            }

            if (AuthenticationInfoResult->auth_type !=
                    OpenAPI_auth_type_5G_AKA) {
                ogs_error("[%s] Not supported Auth Method [%d]",
                    ausf_ue->id, AuthenticationInfoResult->auth_type);
                ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_NOT_IMPLEMENTED,
                    recvmsg, "Not supported Auth Method", ausf_ue->id);
                return false;
            }

            AuthenticationVector =
                AuthenticationInfoResult->authentication_vector;
            if (!AuthenticationVector) {
                ogs_error("[%s] No AuthenticationVector", ausf_ue->id);
                ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR,
                    recvmsg, "No AuthenticationVector", ausf_ue->id);
                return false;
            }

            if (AuthenticationVector->av_type != OpenAPI_av_type_5G_HE_AKA) {
                ogs_error("[%s] Not supported Auth Method [%d]",
                    ausf_ue->id, AuthenticationVector->av_type);
                ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_NOT_IMPLEMENTED,
                    recvmsg, "Not supported Auth Method", ausf_ue->id);
                return false;
            }

            if (!AuthenticationVector->rand) {
                ogs_error("[%s] No AuthenticationVector.rand", ausf_ue->id);
                ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR,
                    recvmsg, "No AuthenticationVector.rand", ausf_ue->id);
                return false;
            }

            if (!AuthenticationVector->xres_star) {
                ogs_error("[%s] No AuthenticationVector.xresStar", ausf_ue->id);
                ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR,
                    recvmsg, "No AuthenticationVector.xresStar", ausf_ue->id);
                return false;
            }

            if (!AuthenticationVector->autn) {
                ogs_error("[%s] No AuthenticationVector.autn", ausf_ue->id);
                ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR,
                    recvmsg, "No AuthenticationVector.autn", ausf_ue->id);
                return false;
            }

            if (!AuthenticationVector->kausf) {
                ogs_error("[%s] No AuthenticationVector.kausf", ausf_ue->id);
                ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR,
                    recvmsg, "No AuthenticationVector.kausf", ausf_ue->id);
                return false;
            }

            memset(&UeAuthenticationCtx, 0, sizeof(UeAuthenticationCtx));

            UeAuthenticationCtx.auth_type = OpenAPI_auth_type_5G_AKA;

            memset(&AV5G_AKA, 0, sizeof(AV5G_AKA));
            AV5G_AKA.rand = AuthenticationVector->rand;
            AV5G_AKA.autn = AuthenticationVector->autn;
            AV5G_AKA.hxres_star = AuthenticationVector->xres_star;
            UeAuthenticationCtx._5g_auth_data = &AV5G_AKA;

            memset(&LinksValueSchemeValue, 0, sizeof(LinksValueSchemeValue));
            LinksValueSchemeValue.href = (char*)"asdfsadf";
            LinksValueScheme = OpenAPI_map_create(
                    (char *)links_member_name(UeAuthenticationCtx.auth_type),
                    &LinksValueSchemeValue);

            UeAuthenticationCtx._links = OpenAPI_list_create();
            OpenAPI_list_add(UeAuthenticationCtx._links, LinksValueScheme);

            memset(&sendmsg, 0, sizeof(sendmsg));

            sendmsg.UeAuthenticationCtx = &UeAuthenticationCtx;
#if 0
            sendmsg.http.content_type = (char *)OGS_SBI_CONTENT_3GPPHAL_TYPE;
#endif

            response = ogs_sbi_build_response(&sendmsg,
                OGS_SBI_HTTP_STATUS_CREATED);
            ogs_assert(response);
            ogs_sbi_server_send_response(session, response);

            OpenAPI_list_free(UeAuthenticationCtx._links);
            OpenAPI_map_free(LinksValueScheme);

            return true;

        DEFAULT
            ogs_error("Invalid resource name [%s]",
                    recvmsg->h.resource.component[2]);
        END
        break;

    DEFAULT
        ogs_error("Invalid resource name [%s]",
                recvmsg->h.resource.component[1]);
    END

    ogs_assert_if_reached();
    return false;
}
