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

#include "sbi-path.h"
#include "nsmf-handler.h"

bool smf_nsmf_handle_create_sm_context(
        smf_sess_t *sess, ogs_sbi_message_t *message)
{
    ogs_sbi_session_t *session = NULL;

    OpenAPI_sm_context_create_data_t *SMContextCreateData = NULL;

    ogs_assert(sess);
    session = sess->session;
    ogs_assert(session);

    ogs_assert(message);

    SMContextCreateData = message->SMContextCreateData;
    if (!SMContextCreateData) {
        ogs_error("[%s:%d] No SMContextCreateData", sess->supi, sess->psi);
        smf_sbi_send_sm_context_create_error(session,
                OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                "No SMContextCreateData", sess->supi_psi_keybuf);
        return false;
    }

    if (SMContextCreateData->dnn) {
        if (sess->dnn) ogs_free(sess->dnn);
        sess->dnn = ogs_strdup(SMContextCreateData->dnn);
    }

    return true;
}
