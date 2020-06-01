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
#include "nudm-handler.h"

void ausf_ue_fsm_init(ausf_ue_t *ausf_ue)
{
    ausf_event_t e;

    ogs_assert(ausf_ue);
    e.ausf_ue = ausf_ue;

    ogs_fsm_create(&ausf_ue->sm, ausf_ue_state_initial, ausf_ue_state_final);
    ogs_fsm_init(&ausf_ue->sm, &e);
}

void ausf_ue_fsm_fini(ausf_ue_t *ausf_ue)
{
    ausf_event_t e;

    ogs_assert(ausf_ue);
    e.ausf_ue = ausf_ue;

    ogs_fsm_fini(&ausf_ue->sm, &e);
    ogs_fsm_delete(&ausf_ue->sm);
}

void ausf_ue_state_initial(ogs_fsm_t *s, ausf_event_t *e)
{
    ausf_ue_t *ausf_ue = NULL;

    ogs_assert(s);
    ogs_assert(e);

    ausf_sm_debug(e);

    ausf_ue = e->ausf_ue;
    ogs_assert(ausf_ue);

    ausf_ue->sbi_message_wait.timer = ogs_timer_add(ausf_self()->timer_mgr,
            ausf_timer_sbi_message_wait_expire, ausf_ue);

    OGS_FSM_TRAN(s, &ausf_ue_state_will_authenticate);
}

void ausf_ue_state_final(ogs_fsm_t *s, ausf_event_t *e)
{
    ausf_ue_t *ausf_ue = NULL;

    ogs_assert(s);
    ogs_assert(e);

    ausf_sm_debug(e);

    ausf_ue = e->ausf_ue;
    ogs_assert(ausf_ue);

    CLEAR_AUSF_UE_ALL_TIMERS(ausf_ue);
    ogs_timer_delete(ausf_ue->sbi_message_wait.timer);
}

void ausf_ue_state_will_authenticate(ogs_fsm_t *s, ausf_event_t *e)
{
    ausf_ue_t *ausf_ue = NULL;

    ogs_sbi_server_t *server = NULL;
    ogs_sbi_session_t *session = NULL;
    ogs_sbi_message_t *message = NULL;

    ogs_assert(s);
    ogs_assert(e);

    ausf_sm_debug(e);

    ausf_ue = e->ausf_ue;
    ogs_assert(ausf_ue);

    switch (e->id) {
    case OGS_FSM_ENTRY_SIG:
        break;

    case OGS_FSM_EXIT_SIG:
        break;

    case AUSF_EVT_SBI_SERVER:
        message = e->sbi.message;
        ogs_assert(message);
        session = e->sbi.session;
        ogs_assert(session);
        server = e->sbi.server;
        ogs_assert(server);

        SWITCH(message->h.method)
        CASE(OGS_SBI_HTTP_METHOD_POST)
            ausf_nausf_auth_handle_authenticate(server, session, message);
            break;
        DEFAULT
            ogs_error("[%s] Invalid HTTP method [%s]",
                    ausf_ue->id, message->h.method);
            ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_MEHTOD_NOT_ALLOWED, message,
                    "Invalid HTTP method", message->h.method);
        END
        break;

    case AUSF_EVT_SBI_CLIENT:
        message = e->sbi.message;
        ogs_assert(message);
        session = e->sbi.data;
        ogs_assert(session);
        ausf_ue = ogs_sbi_session_get_data(session);
        ogs_assert(ausf_ue);

        SWITCH(message->h.service.name)
        CASE(OGS_SBI_SERVICE_NAME_NUDM_UEAU)
            session = e->sbi.data;
            ogs_assert(session);
            ausf_ue = ogs_sbi_session_get_data(session);
            ogs_assert(ausf_ue);

            SWITCH(message->h.method)
            CASE(OGS_SBI_HTTP_METHOD_POST)
                if (message->res_status == OGS_SBI_HTTP_STATUS_OK) {
                    ogs_timer_stop(ausf_ue->sbi_message_wait.timer);

                    ausf_nudm_ueau_handle_get(session, message);
                } else {
                    ogs_error("[%s] HTTP response error [%d]",
                            ausf_ue->id, message->res_status);
                    ogs_sbi_server_send_error(
                            session, message->res_status,
                            NULL, "HTTP response error", ausf_ue->id);
                }
                break;

            DEFAULT
                ogs_error("[%s] Invalid HTTP method [%s]",
                        ausf_ue->id, message->h.method);
                ogs_assert_if_reached();
            END
            break;

        DEFAULT
            ogs_error("Invalid API name [%s]", message->h.service.name);
            ogs_assert_if_reached();
        END
        break;


    default:
        ogs_error("[%s] Unknown event %s", ausf_ue->id, ausf_event_get_name(e));
        break;
    }
}

void ausf_ue_state_authenticated(ogs_fsm_t *s, ausf_event_t *e)
{
    ausf_ue_t *ausf_ue = NULL;

    ogs_assert(s);
    ogs_assert(e);

    ausf_sm_debug(e);

    ausf_ue = e->ausf_ue;
    ogs_assert(ausf_ue);

    switch (e->id) {
    case OGS_FSM_ENTRY_SIG:
        break;

    case OGS_FSM_EXIT_SIG:
        break;

    case AUSF_EVT_SBI_CLIENT:
        break;

    case AUSF_EVT_SBI_TIMER:
        break;

    default:
        ogs_error("[%s] Unknown event %s", ausf_ue->id, ausf_event_get_name(e));
        break;
    }
}

void ausf_ue_state_exception(ogs_fsm_t *s, ausf_event_t *e)
{
    ausf_ue_t *ausf_ue = NULL;
    ogs_assert(s);
    ogs_assert(e);

    ausf_sm_debug(e);

    ausf_ue = e->ausf_ue;
    ogs_assert(ausf_ue);

    switch (e->id) {
    case OGS_FSM_ENTRY_SIG:
        break;

    case OGS_FSM_EXIT_SIG:
        break;

    default:
        ogs_error("[%s] Unknown event %s", ausf_ue->id, ausf_event_get_name(e));
        break;
    }
}
