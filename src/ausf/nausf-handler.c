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
#include "nausf-handler.h"

bool ausf_nausf_handle_authenticate(
        ausf_ue_t *ausf_ue, ogs_sbi_server_t *server,
        ogs_sbi_session_t *session, ogs_sbi_message_t *recvmsg)
{
#if 0
    int status;
    bool handled;
    ogs_sbi_response_t *response = NULL;
#endif

    OpenAPI_authentication_info_t *AuthenticationInfo = NULL;
    char *serving_network_name = NULL;

    ogs_assert(ausf_ue);
    ogs_assert(server);
    ogs_assert(session);
    ogs_assert(recvmsg);

    AuthenticationInfo = recvmsg->AuthenticationInfo;
    if (!AuthenticationInfo) {
        ogs_error("No AuthenticationInfo");
        ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                recvmsg, "No AuthenticationInfo", NULL);
        return false;
    }

    serving_network_name = AuthenticationInfo->serving_network_name;
    if (!AuthenticationInfo) {
        ogs_error("No AuthenticationInfo");
        ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                recvmsg, "No AuthenticationInfo", NULL);
        return false;
    }

    ausf_ue->serving_network_name = ogs_strdup(serving_network_name);
    ogs_assert(ausf_ue->serving_network_name);

    ausf_nudm_ueau_discover_and_send_get(ausf_ue, session);

    return true;
}
