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
    ogs_dbi_auth_info_t auth_info;
    const char *id_type = NULL;
    const char *ue_id = NULL;
    int rv;

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

    return true;
}
