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

#include "test-common.h"

ogs_pkbuf_t *testgsm_build_pdu_session_establishment_request(
        test_sess_t *test_sess)
{
    ogs_nas_5gs_message_t message;
    ogs_nas_5gs_pdu_session_establishment_request_t
        *pdu_session_establishment_request =
            &message.gsm.pdu_session_establishment_request;
    ogs_nas_integrity_protection_maximum_data_rate_t
        *integrity_protection_maximum_data_rate = NULL;
    ogs_nas_pdu_session_type_t *pdu_session_type = NULL;
          
    test_ue_t *test_ue = NULL;
    ogs_pkbuf_t *pkbuf = NULL;

    ogs_assert(test_sess);
    test_ue = test_sess;
    ogs_assert(test_ue);

    integrity_protection_maximum_data_rate =
        &pdu_session_establishment_request->
            integrity_protection_maximum_data_rate;
    pdu_session_type = &pdu_session_establishment_request->pdu_session_type;

    memset(&message, 0, sizeof(message));

    message.gsm.h.extended_protocol_discriminator =
        OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GSM;
    message.gsm.h.pdu_session_identity = test_sess->id;
    message.gsm.h.procedure_transaction_identity = test_sess->pti;
    message.gsm.h.message_type = OGS_NAS_5GS_PDU_SESSION_ESTABLISHMENT_REQUEST;

    integrity_protection_maximum_data_rate->ul = 0xff;
    integrity_protection_maximum_data_rate->dl = 0xff;

    pdu_session_type->value = test_sess->pdn_type;

    return ogs_nas_5gs_plain_encode(&message);
}
