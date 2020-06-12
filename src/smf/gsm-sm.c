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

void smf_gsm_state_initial(ogs_fsm_t *s, smf_event_t *e)
{
    ogs_assert(s);

    OGS_FSM_TRAN(s, &smf_gsm_state_operational);
}

void smf_gsm_state_final(ogs_fsm_t *s, smf_event_t *e)
{
}

void smf_gsm_state_operational(ogs_fsm_t *s, smf_event_t *e)
{
    smf_sess_t *sess = NULL;

    ogs_sbi_session_t *session = NULL;
    ogs_sbi_message_t *message = NULL;

    ogs_assert(s);
    ogs_assert(e);

    smf_sm_debug(e);

    sess = e->sess;
    ogs_assert(sess);

    switch (e->id) {
    case OGS_FSM_ENTRY_SIG:
        break;

    case OGS_FSM_EXIT_SIG:
        break;

    case SMF_EVT_SBI_SERVER:
        message = e->sbi.message;
        ogs_assert(message);
        session = e->sbi.session;
        ogs_assert(session);

        SWITCH(message->h.service.name)
        CASE(OGS_SBI_SERVICE_NAME_NSMF_PDUSESSION)
            SWITCH(message->h.method)
            CASE(OGS_SBI_HTTP_METHOD_POST)
                SWITCH(message->h.resource.component[0])
                CASE(OGS_SBI_RESOURCE_NAME_SM_CONTEXTS)
                    ogs_fatal("asdfkljasdfasdf");
                    break;
                DEFAULT
                    ogs_error("Invalid resource name [%s]",
                            message->h.resource.component[0]);
                    smf_sbi_send_sm_context_create_error(session,
                            OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                            "Invalid resource name",
                            message->h.resource.component[0]);
                END
                break;

            DEFAULT
                ogs_error("Invalid HTTP method [%s]",
                        message->h.method);
                smf_sbi_send_sm_context_create_error(session,
                        OGS_SBI_HTTP_STATUS_FORBIDDEN,
                        "Invalid HTTP method", message->h.method);
            END
            break;

        DEFAULT
            ogs_error("Invalid API name [%s]", message->h.service.name);
            smf_sbi_send_sm_context_create_error(session,
                    OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                    "Invalid API name", message->h.service.name);
        END
        break;

    case SMF_EVT_SBI_CLIENT:
        message = e->sbi.message;
        ogs_assert(message);
        sess = e->sbi.data;
        ogs_assert(sess);
        session = sess->session;
        ogs_assert(session);

        ogs_timer_stop(sess->sbi.client_wait_timer);
        ogs_fatal("TODO");


        break;
    default:
        ogs_error("[%s] Unknown event %s", sess->imsi_bcd, smf_event_get_name(e));
        break;
    }
}

void smf_gsm_state_exception(ogs_fsm_t *s, smf_event_t *e)
{
    smf_sess_t *sess = NULL;
    ogs_assert(s);
    ogs_assert(e);

    smf_sm_debug(e);

    sess = e->sess;
    ogs_assert(sess);

    switch (e->id) {
    case OGS_FSM_ENTRY_SIG:
        break;

    case OGS_FSM_EXIT_SIG:
        break;

    default:
        ogs_error("[%s] Unknown event %s", sess->imsi_bcd, smf_event_get_name(e));
        break;
    }
}
