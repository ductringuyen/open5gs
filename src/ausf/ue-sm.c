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

#include "context.h"

#include "sbi-path.h"
#include "nnrf-handler.h"

void ausf_ue_fsm_init(ausf_ue_t *asuf_ue)
{
    ausf_event_t e;

    ogs_assert(asuf_ue);
    e.sbi.data = asuf_ue;

    ogs_fsm_create(&asuf_ue->sm, ausf_ue_state_initial, ausf_ue_state_final);
    ogs_fsm_init(&asuf_ue->sm, &e);
}

void ausf_ue_fsm_fini(ausf_ue_t *asuf_ue)
{
    ausf_event_t e;

    ogs_assert(asuf_ue);
    e.sbi.data = asuf_ue;

    ogs_fsm_fini(&asuf_ue->sm, &e);
    ogs_fsm_delete(&asuf_ue->sm);
}

void ausf_ue_state_initial(ogs_fsm_t *s, ausf_event_t *e)
{
    ausf_ue_t *ausf_ue = NULL;

    ogs_assert(s);
    ogs_assert(e);

    ausf_sm_debug(e);

    ausf_ue = e->sbi.data;
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

    ausf_ue = e->sbi.data;
    ogs_assert(ausf_ue);

    CLEAR_AUSF_UE_ALL_TIMERS(ausf_ue);
    ogs_timer_delete(ausf_ue->sbi_message_wait.timer);
}

void ausf_ue_state_will_authenticate(ogs_fsm_t *s, ausf_event_t *e)
{
    ausf_ue_t *ausf_ue = NULL;
    ogs_sbi_client_t *client = NULL;
    ogs_sbi_message_t *message = NULL;
    ogs_sockaddr_t *addr = NULL;

    ogs_assert(s);
    ogs_assert(e);

    ausf_sm_debug(e);

    ausf_ue = e->sbi.data;
    ogs_assert(ausf_ue);

    switch (e->id) {
    case OGS_FSM_ENTRY_SIG:
#if 0
        ogs_timer_start(nf_instance->t_registration_interval,
                ausf_timer_cfg(AUSF_TIMER_NF_INSTANCE_REGISTRATION_INTERVAL)->
                    duration);

        ogs_sbi_send_nf_register(nf_instance);
#endif
        break;

    case OGS_FSM_EXIT_SIG:
#if 0
        ogs_timer_stop(nf_instance->t_registration_interval);
#endif
        break;

    case AUSF_EVT_SBI_CLIENT:
#if 0
        message = e->sbi.message;
        ogs_assert(message);

        SWITCH(message->h.service.name)
        CASE(OGS_SBI_SERVICE_NAME_NRF_NFM)

            SWITCH(message->h.resource.name)
            CASE(OGS_SBI_RESOURCE_NAME_NF_INSTANCES)

                if (message->res_status == OGS_SBI_HTTP_STATUS_OK ||
                    message->res_status == OGS_SBI_HTTP_STATUS_CREATED) {
                    ausf_nnrf_handle_nf_register(nf_instance, message);
                    OGS_FSM_TRAN(s, &ausf_ue_state_registered);
                } else {
                    ogs_error("HTTP Response Status Code [%d]",
                            message->res_status);
                    OGS_FSM_TRAN(s, &ausf_ue_state_exception);
                }
                break;

            DEFAULT
                ogs_error("Invalid resource name [%s]",
                        message->h.resource.name);
            END
            break;

        DEFAULT
            ogs_error("Invalid API name [%s]", message->h.service.name);
        END
#endif
        break;

    case AUSF_EVT_SBI_TIMER:
#if 0
        switch(e->timer_id) {
        case AUSF_TIMER_NF_INSTANCE_REGISTRATION_INTERVAL:
            client = nf_instance->client;
            ogs_assert(client);
            addr = client->addr;
            ogs_assert(addr);

            ogs_warn("Retry to registration with NRF [%s]", nf_instance->id);

            ogs_timer_start(nf_instance->t_registration_interval,
                ausf_timer_cfg(AUSF_TIMER_NF_INSTANCE_REGISTRATION_INTERVAL)->
                    duration);

            ogs_sbi_send_nf_register(nf_instance);
            break;

        default:
            ogs_error("Unknown timer[%s:%d]",
                    ausf_timer_get_name(e->timer_id), e->timer_id);
        }
#endif
        break;

    default:
        ogs_error("Unknown event %s", ausf_event_get_name(e));
        break;
    }
}

void ausf_ue_state_authenticated(ogs_fsm_t *s, ausf_event_t *e)
{
    ausf_ue_t *ausf_ue = NULL;
    ogs_sbi_client_t *client = NULL;
    ogs_sbi_message_t *message = NULL;
    ogs_assert(s);
    ogs_assert(e);

    ausf_sm_debug(e);

    ausf_ue = e->sbi.data;
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
        ogs_error("Unknown event %s", ausf_event_get_name(e));
        break;
    }
}

void ausf_ue_state_exception(ogs_fsm_t *s, ausf_event_t *e)
{
    ogs_sbi_nf_instance_t *nf_instance = NULL;
    ogs_sbi_client_t *client = NULL;
    ogs_sockaddr_t *addr = NULL;
    ogs_assert(s);
    ogs_assert(e);

    ausf_sm_debug(e);

    nf_instance = e->sbi.data;
    ogs_assert(nf_instance);

    switch (e->id) {
    case OGS_FSM_ENTRY_SIG:
        break;

    case OGS_FSM_EXIT_SIG:
        break;

    default:
        ogs_error("Unknown event %s", ausf_event_get_name(e));
        break;
    }
}
