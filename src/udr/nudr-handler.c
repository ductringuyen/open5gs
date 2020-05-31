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

bool udr_nudr_dr_handle_query(ogs_sbi_server_t *server,
        ogs_sbi_session_t *session, ogs_sbi_message_t *recvmsg)
{
    ogs_fatal("asdlfkjasdfasf");
#if 0
    OpenAPI_authentication_info_request_t *AuthenticationInfoRequest = NULL;
    char *serving_network_name = NULL;
    char *ausf_instance_id = NULL;

    udr_ue_t *udr_ue = NULL;

    ogs_assert(session);
    udr_ue = ogs_sbi_session_get_data(session);
    ogs_assert(udr_ue);

    ogs_assert(server);
    ogs_assert(recvmsg);

    AuthenticationInfoRequest = recvmsg->AuthenticationInfoRequest;
    if (!AuthenticationInfoRequest) {
        ogs_error("No AuthenticationInfoRequest");
        ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                recvmsg, "No AuthenticationInfoRequest", NULL);
        return false;
    }

    serving_network_name = AuthenticationInfoRequest->serving_network_name;
    if (!AuthenticationInfoRequest) {
        ogs_error("No servingNetworkName");
        ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                recvmsg, "No servingNetworkName", NULL);
        return false;
    }

    ausf_instance_id = AuthenticationInfoRequest->ausf_instance_id;
    if (!AuthenticationInfoRequest) {
        ogs_error("No ausfInstanceId");
        ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                recvmsg, "No ausfInstanceId", NULL);
        return false;
    }

    udr_ue->serving_network_name = ogs_strdup(serving_network_name);
    ogs_assert(udr_ue->serving_network_name);
    udr_ue->ausf_instance_id = ogs_strdup(ausf_instance_id);
    ogs_assert(udr_ue->ausf_instance_id);
#endif

    return true;
}
