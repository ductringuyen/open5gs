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

#include "sbi-path.h"
#include "nnrf-handler.h"
#include "nudr-handler.h"

bool udr_nudr_dr_handle_subscription_authentication(
        ogs_sbi_session_t *session, ogs_sbi_message_t *recvmsg)
{
    int rv;

    ogs_sbi_message_t sendmsg;
    ogs_sbi_response_t *response = NULL;
    ogs_dbi_auth_info_t auth_info;

    char k_string[OGS_KEYSTRLEN(OGS_KEY_LEN)];
    char opc_string[OGS_KEYSTRLEN(OGS_KEY_LEN)];
    char amf_string[OGS_KEYSTRLEN(OGS_AMF_LEN)];
    char sqn_string[OGS_KEYSTRLEN(OGS_SQN_LEN)];

    char sqn[OGS_SQN_LEN];
    char *supi = NULL;

    OpenAPI_authentication_subscription_t AuthenticationSubscription;
    OpenAPI_sequence_number_t SequenceNumber;
    OpenAPI_auth_event_t *AuthEvent = NULL;

    ogs_assert(session);
    ogs_assert(recvmsg);

    supi = recvmsg->h.resource.component[1];
    if (!supi) {
        ogs_error("No SUPI");
        ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                recvmsg, "No SUPI", NULL);
        return false;
    }

    if (strncmp(supi,
            OGS_DBI_SUPI_TYPE_IMSI, strlen(OGS_DBI_SUPI_TYPE_IMSI)) != 0) {
        ogs_error("[%s] Unknown SUPI Type", supi);
        ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_FORBIDDEN,
                recvmsg, "Unknwon SUPI Type", supi);
        return false;
    }

    rv = ogs_dbi_auth_info(supi, &auth_info);
    if (rv != OGS_OK) {
        ogs_fatal("[%s] Cannot find SUPI in DB", supi);
        ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_NOT_FOUND,
                recvmsg, "Cannot find SUPI Type", supi);
        return false;
    }

    SWITCH(recvmsg->h.resource.component[3])
    CASE(OGS_SBI_RESOURCE_NAME_AUTHENTICATION_SUBSCRIPTION)
        SWITCH(recvmsg->h.method)
        CASE(OGS_SBI_HTTP_METHOD_GET)
            memset(&AuthenticationSubscription, 0,
                    sizeof(AuthenticationSubscription));

            AuthenticationSubscription.authentication_method =
                OpenAPI_auth_method_5G_AKA;

            ogs_hex_to_ascii(auth_info.k, sizeof(auth_info.k),
                    k_string, sizeof(k_string));
            AuthenticationSubscription.enc_permanent_key = k_string;

            ogs_hex_to_ascii(auth_info.amf, sizeof(auth_info.amf),
                    amf_string, sizeof(amf_string));
            AuthenticationSubscription.authentication_management_field =
                    amf_string;

            if (!auth_info.use_opc)
                milenage_opc(auth_info.k, auth_info.op, auth_info.opc);

            ogs_hex_to_ascii(auth_info.opc, sizeof(auth_info.opc),
                    opc_string, sizeof(opc_string));
            AuthenticationSubscription.enc_opc_key = opc_string;

            ogs_uint64_to_buffer(auth_info.sqn, OGS_SQN_LEN, sqn);
            ogs_hex_to_ascii(sqn, sizeof(sqn), sqn_string, sizeof(sqn_string));

            memset(&SequenceNumber, 0, sizeof(SequenceNumber));
            SequenceNumber.sqn_scheme = OpenAPI_sqn_scheme_NON_TIME_BASED;
            SequenceNumber.sqn = sqn_string;
            AuthenticationSubscription.sequence_number = &SequenceNumber;

            memset(&sendmsg, 0, sizeof(sendmsg));

            ogs_assert(AuthenticationSubscription.authentication_method);
            sendmsg.AuthenticationSubscription =
                &AuthenticationSubscription;

            response = ogs_sbi_build_response(
                    &sendmsg, OGS_SBI_HTTP_STATUS_OK);
            ogs_assert(response);
            ogs_sbi_server_send_response(session, response);

            return true;

        DEFAULT
            ogs_error("Invalid HTTP method [%s]", recvmsg->h.method);
            ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_MEHTOD_NOT_ALLOWED,
                    recvmsg, "Invalid HTTP method", recvmsg->h.method);
        END
        break;

    CASE(OGS_SBI_RESOURCE_NAME_AUTHENTICATION_STATUS)
        SWITCH(recvmsg->h.method)
        CASE(OGS_SBI_HTTP_METHOD_PUT)
            AuthEvent = recvmsg->AuthEvent;
            if (!AuthEvent) {
                ogs_error("[%s] No AuthEvent", supi);
                ogs_sbi_server_send_error(
                        session, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                        recvmsg, "No AuthEvent", supi);
                return false;
            }

            memset(&sendmsg, 0, sizeof(sendmsg));
            rv = ogs_dbi_increment_sqn(supi);
            if (rv != OGS_OK) {
                ogs_fatal("[%s] Cannot increment SQN", supi);
                ogs_sbi_server_send_error(session,
                        OGS_SBI_HTTP_STATUS_INTERNAL_SERVER_ERROR,
                        recvmsg, "Cannot increment SQN", supi);
                return false;
            }

            response = ogs_sbi_build_response(
                    &sendmsg, OGS_SBI_HTTP_STATUS_NO_CONTENT);
            ogs_assert(response);
            ogs_sbi_server_send_response(session, response);

            return true;

        DEFAULT
            ogs_error("Invalid HTTP method [%s]", recvmsg->h.method);
            ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_MEHTOD_NOT_ALLOWED,
                    recvmsg, "Invalid HTTP method", recvmsg->h.method);
        END
        break;

    DEFAULT
        ogs_error("Invalid resource name [%s]",
                recvmsg->h.resource.component[3]);
        ogs_sbi_server_send_error(session,
                OGS_SBI_HTTP_STATUS_MEHTOD_NOT_ALLOWED,
                recvmsg, "Unknown resource name",
                recvmsg->h.resource.component[3]);
    END

    return false;
}

bool udr_nudr_dr_handle_subscription_context(
        ogs_sbi_session_t *session, ogs_sbi_message_t *recvmsg)
{
    ogs_sbi_message_t sendmsg;
    ogs_sbi_response_t *response = NULL;

    char *supi = NULL;

    ogs_assert(session);
    ogs_assert(recvmsg);

    supi = recvmsg->h.resource.component[1];
    if (!supi) {
        ogs_error("No SUPI");
        ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                recvmsg, "No SUPI", NULL);
        return false;
    }

    if (strncmp(supi,
            OGS_DBI_SUPI_TYPE_IMSI, strlen(OGS_DBI_SUPI_TYPE_IMSI)) != 0) {
        ogs_error("[%s] Unknown SUPI Type", supi);
        ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_FORBIDDEN,
                recvmsg, "Unknwon SUPI Type", supi);
        return false;
    }

    SWITCH(recvmsg->h.resource.component[3])
    CASE(OGS_SBI_RESOURCE_NAME_AMF_3GPP_ACCESS)
        SWITCH(recvmsg->h.method)
        CASE(OGS_SBI_HTTP_METHOD_PUT)
            memset(&sendmsg, 0, sizeof(sendmsg));

            response = ogs_sbi_build_response(
                    &sendmsg, OGS_SBI_HTTP_STATUS_NO_CONTENT);
            ogs_assert(response);
            ogs_sbi_server_send_response(session, response);

            return true;

        DEFAULT
            ogs_error("Invalid HTTP method [%s]", recvmsg->h.method);
            ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_MEHTOD_NOT_ALLOWED,
                    recvmsg, "Invalid HTTP method", recvmsg->h.method);
        END
        break;

    DEFAULT
        ogs_error("Invalid resource name [%s]",
                recvmsg->h.resource.component[3]);
        ogs_sbi_server_send_error(session,
                OGS_SBI_HTTP_STATUS_MEHTOD_NOT_ALLOWED,
                recvmsg, "Unknown resource name",
                recvmsg->h.resource.component[3]);
    END

    return false;
}

bool udr_nudr_dr_handle_subscription_provisioned(
        ogs_sbi_session_t *session, ogs_sbi_message_t *recvmsg)
{
    int rv;

    ogs_sbi_message_t sendmsg;
    ogs_sbi_response_t *response = NULL;

    char *supi = NULL;

    ogs_assert(session);
    ogs_assert(recvmsg);

    supi = recvmsg->h.resource.component[1];
    if (!supi) {
        ogs_error("No SUPI");
        ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                recvmsg, "No SUPI", NULL);
        return false;
    }

    if (strncmp(supi,
            OGS_DBI_SUPI_TYPE_IMSI, strlen(OGS_DBI_SUPI_TYPE_IMSI)) != 0) {
        ogs_error("[%s] Unknown SUPI Type", supi);
        ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_FORBIDDEN,
                recvmsg, "Unknwon SUPI Type", supi);
        return false;
    }

    SWITCH(recvmsg->h.resource.component[4])
    CASE(OGS_SBI_RESOURCE_NAME_AM_DATA)
        SWITCH(recvmsg->h.method)
        CASE(OGS_SBI_HTTP_METHOD_GET)

            ogs_fatal("am-data");

            return true;

        DEFAULT
            ogs_error("Invalid HTTP method [%s]", recvmsg->h.method);
            ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_MEHTOD_NOT_ALLOWED,
                    recvmsg, "Invalid HTTP method", recvmsg->h.method);
        END
        break;

    DEFAULT
        ogs_error("Invalid resource name [%s]",
                recvmsg->h.resource.component[3]);
        ogs_sbi_server_send_error(session,
                OGS_SBI_HTTP_STATUS_MEHTOD_NOT_ALLOWED,
                recvmsg, "Unknown resource name",
                recvmsg->h.resource.component[3]);
    END

    return false;
}
