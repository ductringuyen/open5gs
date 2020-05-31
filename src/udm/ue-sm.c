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
#include "nudm-handler.h"

void udm_ue_fsm_init(udm_ue_t *udm_ue)
{
    udm_event_t e;

    ogs_assert(udm_ue);
    e.udm_ue = udm_ue;

    ogs_fsm_create(&udm_ue->sm, udm_ue_state_initial, udm_ue_state_final);
    ogs_fsm_init(&udm_ue->sm, &e);
}

void udm_ue_fsm_fini(udm_ue_t *udm_ue)
{
    udm_event_t e;

    ogs_assert(udm_ue);
    e.udm_ue = udm_ue;

    ogs_fsm_fini(&udm_ue->sm, &e);
    ogs_fsm_delete(&udm_ue->sm);
}

void udm_ue_state_initial(ogs_fsm_t *s, udm_event_t *e)
{
    udm_ue_t *udm_ue = NULL;

    ogs_assert(s);
    ogs_assert(e);

    udm_sm_debug(e);

    udm_ue = e->udm_ue;
    ogs_assert(udm_ue);

    udm_ue->sbi_message_wait.timer = ogs_timer_add(udm_self()->timer_mgr,
            udm_timer_sbi_message_wait_expire, udm_ue);

    OGS_FSM_TRAN(s, &udm_ue_state_will_authenticate);
}

void udm_ue_state_final(ogs_fsm_t *s, udm_event_t *e)
{
    udm_ue_t *udm_ue = NULL;

    ogs_assert(s);
    ogs_assert(e);

    udm_sm_debug(e);

    udm_ue = e->udm_ue;
    ogs_assert(udm_ue);

    CLEAR_UDM_UE_ALL_TIMERS(udm_ue);
    ogs_timer_delete(udm_ue->sbi_message_wait.timer);
}

void udm_ue_state_will_authenticate(ogs_fsm_t *s, udm_event_t *e)
{
    udm_ue_t *udm_ue = NULL;

    ogs_sbi_server_t *server = NULL;
    ogs_sbi_session_t *session = NULL;
    ogs_sbi_message_t *message = NULL;

    ogs_assert(s);
    ogs_assert(e);

    udm_sm_debug(e);

    udm_ue = e->udm_ue;
    ogs_assert(udm_ue);

    switch (e->id) {
    case OGS_FSM_ENTRY_SIG:
        break;

    case OGS_FSM_EXIT_SIG:
        break;

    case UDM_EVT_SBI_SERVER:
        message = e->sbi.message;
        ogs_assert(message);
        session = e->sbi.session;
        ogs_assert(session);
        server = e->sbi.server;
        ogs_assert(server);

        SWITCH(message->h.method)
        CASE(OGS_SBI_HTTP_METHOD_POST)
            udm_nudm_handle_authenticate(server, session, message);
            break;
        DEFAULT
            ogs_error("[%s] Invalid HTTP method [%s]",
                    udm_ue->id, message->h.method);
            ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_MEHTOD_NOT_ALLOWED, message,
                    "Invalid HTTP method", message->h.method);
        END
        break;

    default:
        ogs_error("[%s] Unknown event %s", udm_ue->id, udm_event_get_name(e));
        break;
    }
}

void udm_ue_state_authenticated(ogs_fsm_t *s, udm_event_t *e)
{
    udm_ue_t *udm_ue = NULL;

    ogs_assert(s);
    ogs_assert(e);

    udm_sm_debug(e);

    udm_ue = e->udm_ue;
    ogs_assert(udm_ue);

    switch (e->id) {
    case OGS_FSM_ENTRY_SIG:
        break;

    case OGS_FSM_EXIT_SIG:
        break;

    case UDM_EVT_SBI_CLIENT:
        break;

    case UDM_EVT_SBI_TIMER:
        break;

    default:
        ogs_error("[%s] Unknown event %s", udm_ue->id, udm_event_get_name(e));
        break;
    }
}

void udm_ue_state_exception(ogs_fsm_t *s, udm_event_t *e)
{
    udm_ue_t *udm_ue = NULL;
    ogs_assert(s);
    ogs_assert(e);

    udm_sm_debug(e);

    udm_ue = e->udm_ue;
    ogs_assert(udm_ue);

    switch (e->id) {
    case OGS_FSM_ENTRY_SIG:
        break;

    case OGS_FSM_EXIT_SIG:
        break;

    default:
        ogs_error("[%s] Unknown event %s", udm_ue->id, udm_event_get_name(e));
        break;
    }
}
