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

#include "nsmf-handler.h"
#include "nas-path.h"

#include "gmm-build.h"

int amf_nsmf_pdu_session_handle_create_sm_context(
        amf_ue_t *amf_ue, ogs_sbi_message_t *message)
{
#if 0
    OpenAPI_sm_context_created_data_t *SMContextCreatedData = NULL;
#endif
    OpenAPI_sm_context_create_error_t *SMContextCreateError = NULL;
    OpenAPI_ref_to_binary_data_t *n1_sm_msg = NULL;
    ogs_pkbuf_t *gsmbuf = NULL;

    ogs_assert(amf_ue);
    ogs_assert(message);

    if (message->res_status != OGS_SBI_HTTP_STATUS_CREATED &&
        message->res_status != OGS_SBI_HTTP_STATUS_OK) {
        SMContextCreateError = message->SMContextCreateError;
        if (!SMContextCreateError) {
            ogs_error("[%s] No SMContextCreateError", amf_ue->suci);
            return OGS_ERROR;
        }
        if (!SMContextCreateError->error) {
            ogs_error("[%s] No Error", amf_ue->suci);
            return OGS_ERROR;
        }

        n1_sm_msg = SMContextCreateError->n1_sm_msg;
        if (n1_sm_msg && n1_sm_msg->content_id) {
            gsmbuf = ogs_sbi_find_part_by_content_id(
                    message, n1_sm_msg->content_id);
            if (gsmbuf) {
                gsmbuf = ogs_pkbuf_copy(gsmbuf);
                ogs_assert(gsmbuf);
                nas_5gs_send_gsm_reject(amf_ue, gsmbuf);
            }
        }

        return OGS_ERROR;
    }

    return OGS_OK;
}
