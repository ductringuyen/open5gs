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

bool udr_nudr_dr_handle_query_subscription_data(ogs_sbi_server_t *server,
        ogs_sbi_session_t *session, ogs_sbi_message_t *recvmsg)
{
    int rv;

    ogs_sbi_message_t sendmsg;
    ogs_sbi_response_t *response = NULL;
    ogs_dbi_auth_info_t auth_info;

    const char *id_type = NULL;
    const char *ue_id = NULL;

    char k[OGS_KEYSTRLEN(OGS_KEY_LEN)];
    char opc[OGS_KEYSTRLEN(OGS_KEY_LEN)];
    char amf[OGS_KEYSTRLEN(OGS_AMF_LEN)];
    char sqn_buf[OGS_SQN_LEN];
    char sqn[OGS_KEYSTRLEN(OGS_SQN_LEN)];

    OpenAPI_authentication_subscription_t AuthenticationSubscription;
    OpenAPI_sequence_number_t SequenceNumber;

    ogs_assert(session);
    ogs_assert(server);
    ogs_assert(recvmsg);

    if (!recvmsg->h.resource.component[1]) {
        ogs_error("No ueId");
        ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                recvmsg, "No ueId", NULL);
        return false;
    }

    if (strncmp(recvmsg->h.resource.component[1],
            OGS_DBI_UE_ID_TYPE_IMSI, strlen(OGS_DBI_UE_ID_TYPE_IMSI)) == 0) {
        id_type = OGS_DBI_UE_ID_TYPE_IMSI;
        ue_id = recvmsg->h.resource.component[1] +
            strlen(OGS_DBI_UE_ID_TYPE_IMSI) + 1;
    } else {
        ogs_error("Unknown ueId Type");
        ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_NOT_IMPLEMENTED,
                recvmsg, "Unknwon ueId Type", NULL);
        return false;
    }

    memset(&AuthenticationSubscription, 0, sizeof(AuthenticationSubscription));

    SWITCH(recvmsg->h.resource.component[2])
    CASE(OGS_SBI_RESOURCE_NAME_AUTHENTICATION_DATA)
        SWITCH(recvmsg->h.resource.component[3])
        CASE(OGS_SBI_RESOURCE_NAME_AUTHENTICATION_SUBSCRIPTION)
            rv = ogs_dbi_auth_info(id_type, ue_id, &auth_info);
            if (rv != OGS_OK) {
                ogs_fatal("Cannot find IMSI in DB : %s-%s", id_type, ue_id);
                ogs_sbi_server_send_error(session,
                        OGS_SBI_HTTP_STATUS_NOT_FOUND,
                        recvmsg, "Unknwon ueId Type", ue_id);
                return false;
            }

            AuthenticationSubscription.authentication_method =
                OpenAPI_auth_method_5G_AKA;

            ogs_hex_to_ascii(auth_info.k, sizeof(auth_info.k), k, sizeof(k));
            AuthenticationSubscription.enc_permanent_key = k;

            ogs_hex_to_ascii(auth_info.amf, sizeof(auth_info.amf),
                    amf, sizeof(amf));
            AuthenticationSubscription.authentication_management_field = amf;

            if (!auth_info.use_opc) {
                milenage_opc(auth_info.k, auth_info.op, auth_info.opc);
            }

            ogs_hex_to_ascii(auth_info.opc, sizeof(auth_info.opc),
                    opc, sizeof(opc));
            AuthenticationSubscription.enc_opc_key = opc;

            ogs_uint64_to_buffer(auth_info.sqn, OGS_SQN_LEN, sqn_buf);
            ogs_hex_to_ascii(sqn_buf, sizeof(sqn_buf), sqn, sizeof(sqn));

            memset(&SequenceNumber, 0, sizeof(SequenceNumber));
            SequenceNumber.sqn_scheme = OpenAPI_sqn_scheme_NON_TIME_BASED;
            SequenceNumber.sqn = sqn;
            AuthenticationSubscription.sequence_number = &SequenceNumber;
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

    ogs_assert(AuthenticationSubscription.authentication_method);
    sendmsg.AuthenticationSubscription = &AuthenticationSubscription;

    response = ogs_sbi_build_response(&sendmsg, OGS_SBI_HTTP_STATUS_OK);
    ogs_assert(response);
    ogs_sbi_server_send_response(session, response);

    return true;
}
