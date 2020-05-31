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
    ogs_dbi_auth_info_t auth_info;
    int rv;

    ogs_assert(session);
    ogs_assert(server);
    ogs_assert(recvmsg);

    rv = ogs_dbi_auth_info((char *)"2089300007487", &auth_info);
    if (rv != OGS_OK) {
        ogs_fatal("Cannot find IMSI in DB : %s", "asdfkljsdf");
        return false;
    }
    ogs_fatal("asdflkjasdf");

    return true;
}
