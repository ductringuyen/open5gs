/*
 * Copyright (C) 2019,2020 by Sukchan Lee <acetcom@gmail.com>
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

#include "nudr-handler.h"

bool udm_nudr_dr_handle_get(
        ogs_sbi_session_t *session, ogs_sbi_message_t *recvmsg)
{
    udm_ue_t *udm_ue = NULL;

    ogs_sbi_message_t sendmsg;
    ogs_sbi_response_t *response = NULL;

    const char *rand_string = "4d45b0eeb8386b629f136968837a7b0b"; /* For test */
    char randbuf[OGS_RAND_LEN];
    char autnbuf[OGS_AUTN_LEN];
    char ikbuf[OGS_KEY_LEN];
    char ckbuf[OGS_KEY_LEN];
    char akbuf[OGS_AK_LEN];
    char xresbuf[OGS_MAX_RES_LEN];
    size_t xres_len = 8;

    char rand[OGS_KEYSTRLEN(OGS_RAND_LEN)];
    char autn[OGS_KEYSTRLEN(OGS_AUTN_LEN)];
    char xres[OGS_KEYSTRLEN(OGS_MAX_RES_LEN)];

    OpenAPI_authentication_subscription_t *AuthenticationSubscription = NULL;
    OpenAPI_authentication_info_result_t AuthenticationInfoResult;
    OpenAPI_authentication_vector_t AuthenticationVector;

    ogs_assert(session);
    udm_ue = ogs_sbi_session_get_data(session);
    ogs_assert(udm_ue);

    ogs_assert(recvmsg);
    AuthenticationSubscription = recvmsg->AuthenticationSubscription;
    ogs_assert(AuthenticationSubscription);

    SWITCH(recvmsg->h.resource.component[2])
    CASE(OGS_SBI_RESOURCE_NAME_AUTHENTICATION_DATA)
        SWITCH(recvmsg->h.resource.component[3])
        CASE(OGS_SBI_RESOURCE_NAME_AUTHENTICATION_SUBSCRIPTION)

            if (AuthenticationSubscription->authentication_method !=
                    OpenAPI_auth_method_5G_AKA) {
                ogs_error("[%s] Not supported Auth Method [%d]",
                        udm_ue->id,
                        AuthenticationSubscription->authentication_method);
                ogs_sbi_server_send_error(session,
                        OGS_SBI_HTTP_STATUS_NOT_IMPLEMENTED,
                        recvmsg, "Not supported Auth Method", udm_ue->id);
                return false;

            }

            if (!AuthenticationSubscription->enc_permanent_key) {
                ogs_error("[%s] No encPermanentKey", udm_ue->id);
                ogs_sbi_server_send_error(session,
                        OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR,
                        recvmsg, "No encPermanentKey", udm_ue->id);
                return false;
            }
            if (!AuthenticationSubscription->enc_opc_key) {
                ogs_error("[%s] No encPermanentKey", udm_ue->id);
                ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR,
                    recvmsg, "No encPermanentKey", udm_ue->id);
                return false;
            }
            if (!AuthenticationSubscription->authentication_management_field) {
                ogs_error("[%s] No authenticationManagementField", udm_ue->id);
                ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR,
                    recvmsg, "No authenticationManagementField", udm_ue->id);
                return false;
            }
            if (!AuthenticationSubscription->sequence_number) {
                ogs_error("[%s] No SequenceNumber", udm_ue->id);
                ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR,
                    recvmsg, "No SequenceNumber", udm_ue->id);
                return false;
            }
            if (AuthenticationSubscription->sequence_number->sqn_scheme !=
                    OpenAPI_sqn_scheme_NON_TIME_BASED) {
                ogs_error("[%s] No SequenceNumber.sqnScheme [%d]", udm_ue->id,
                    AuthenticationSubscription->sequence_number->sqn_scheme);
                ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR,
                    recvmsg, "No SequenceNumber.sqnScheme", udm_ue->id);
                return false;
            }
            if (!AuthenticationSubscription->sequence_number->sqn) {
                ogs_error("[%s] No SequenceNumber.sqn", udm_ue->id);
                ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR,
                    recvmsg, "No SequenceNumber.sqn", udm_ue->id);
                return false;
            }

            memset(&AuthenticationInfoResult,
                    0, sizeof(AuthenticationInfoResult));

            AuthenticationInfoResult.auth_type = OpenAPI_auth_type_5G_AKA;

            /* FIX IT! TODO! NEW! */
            ogs_random(randbuf, OGS_RAND_LEN);
            OGS_HEX(rand_string, strlen(rand_string), randbuf);
            milenage_generate(
                (uint8_t *)AuthenticationSubscription->enc_opc_key,
                (uint8_t *)AuthenticationSubscription->
                                authentication_management_field,
                (uint8_t *)AuthenticationSubscription->enc_permanent_key,
                (uint8_t *)AuthenticationSubscription->sequence_number->sqn,
                (uint8_t *)randbuf, (uint8_t *)autnbuf,
                (uint8_t *)ikbuf, (uint8_t *)ckbuf, (uint8_t *)akbuf,
                (uint8_t *)xresbuf, &xres_len);

            memset(&AuthenticationVector, 0, sizeof(AuthenticationVector));
            AuthenticationVector.av_type = OpenAPI_av_type_5G_HE_AKA;

            ogs_hex_to_ascii(randbuf, sizeof(randbuf), rand, sizeof(rand));
            AuthenticationVector.rand = rand;
            ogs_hex_to_ascii(xresbuf, sizeof(xresbuf), xres, sizeof(xres));
            AuthenticationVector.xres_star = xres;
            ogs_hex_to_ascii(autnbuf, sizeof(autnbuf), autn, sizeof(autn));
            AuthenticationVector.autn = autn;

            AuthenticationInfoResult.authentication_vector =
                &AuthenticationVector;
            break;

        DEFAULT
            ogs_error("Invalid resource name [%s]",
                    recvmsg->h.resource.component[3]);
            ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_MEHTOD_NOT_ALLOWED,
                    recvmsg, "Unknown resource name",
                    recvmsg->h.resource.component[3]);
            return false;
        END
        break;

    DEFAULT
        ogs_error("Invalid resource name [%s]",
                recvmsg->h.resource.component[2]);
        ogs_sbi_server_send_error(session,
                OGS_SBI_HTTP_STATUS_MEHTOD_NOT_ALLOWED,
                recvmsg, "Unknown resource name",
                recvmsg->h.resource.component[2]);
    END

    memset(&sendmsg, 0, sizeof(sendmsg));

    ogs_assert(AuthenticationInfoResult.auth_type);
    sendmsg.AuthenticationInfoResult = &AuthenticationInfoResult;

    response = ogs_sbi_build_response(&sendmsg, OGS_SBI_HTTP_STATUS_OK);
    ogs_assert(response);
    ogs_sbi_server_send_response(session, response);

    return true;
}
