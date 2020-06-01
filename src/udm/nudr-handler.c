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

    ogs_assert(session);
    ogs_assert(recvmsg);
    udm_ue = ogs_sbi_session_get_data(session);
    ogs_assert(udm_ue);

    OpenAPI_authentication_info_result_t AuthenticationInfoResult;

    SWITCH(recvmsg->h.resource.component[2])
    CASE(OGS_SBI_RESOURCE_NAME_AUTHENTICATION_DATA)
        SWITCH(recvmsg->h.resource.component[3])
        CASE(OGS_SBI_RESOURCE_NAME_AUTHENTICATION_SUBSCRIPTION)

            ogs_fatal("asdfkljasdfasdF");
#if 0
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
#endif
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
