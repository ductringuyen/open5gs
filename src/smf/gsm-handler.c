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

#include "gsm-handler.h"
#include "sbi-path.h"

#undef OGS_LOG_DOMAIN
#define OGS_LOG_DOMAIN __gsm_log_domain

int gsm_handle_pdu_session_establishment_request(smf_sess_t *sess,
        ogs_nas_5gs_pdu_session_establishment_request_t *
            pdu_session_establishment_request)
{
    ogs_nas_integrity_protection_maximum_data_rate_t *
        integrity_protection_maximum_data_rate =
            &pdu_session_establishment_request->
                integrity_protection_maximum_data_rate;
    ogs_nas_pdu_session_type_t *pdu_session_type = NULL;
    ogs_sbi_session_t *session = NULL;

    ogs_assert(sess);
    session = sess->session;
    ogs_assert(session);
    ogs_assert(pdu_session_establishment_request);

    pdu_session_type = &pdu_session_establishment_request->pdu_session_type;
    ogs_assert(pdu_session_type);

    sess->integrity_protection.mbr_dl =
            integrity_protection_maximum_data_rate->dl;
    sess->integrity_protection.mbr_ul =
            integrity_protection_maximum_data_rate->ul;

    if (pdu_session_establishment_request->presencemask &
        OGS_NAS_5GS_PDU_SESSION_ESTABLISHMENT_REQUEST_PDU_SESSION_TYPE_PRESENT)
        sess->pdu_session_type = pdu_session_type->value;

    {
        ogs_pkbuf_t *gsmbuf = gsm_build_pdu_session_establishment_reject(sess,
            OGS_5GSM_CAUSE_SEMANTICALLY_INCORRECT_MESSAGE);
        smf_sbi_send_sm_context_create_error(session,
                OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                "Invalid resource name", "asdfasdf", gsmbuf);
    }

    return OGS_OK;
}
