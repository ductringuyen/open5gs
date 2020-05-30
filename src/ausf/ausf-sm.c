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

void ausf_state_initial(ogs_fsm_t *s, ausf_event_t *e)
{
    ausf_sm_debug(e);

    ogs_assert(s);

    OGS_FSM_TRAN(s, &ausf_state_operational);
}

void ausf_state_final(ogs_fsm_t *s, ausf_event_t *e)
{
    ausf_sm_debug(e);

    ogs_assert(s);
}

void ausf_state_operational(ogs_fsm_t *s, ausf_event_t *e)
{
    int rv;

    ogs_sbi_server_t *server = NULL;
    ogs_sbi_session_t *session = NULL;
    ogs_sbi_request_t *sbi_request = NULL;

    ogs_sbi_nf_instance_t *nf_instance = NULL;
    ogs_sbi_subscription_t *subscription = NULL;
    ogs_sbi_response_t *response = NULL;
    ogs_sbi_message_t message;

    ausf_ue_t *ausf_ue = NULL;
    char *ue_id = NULL;

    ausf_sm_debug(e);

    ogs_assert(s);

    switch (e->id) {
    case OGS_FSM_ENTRY_SIG:
        rv = ausf_sbi_open();
        if (rv != OGS_OK) {
            ogs_fatal("Can't establish SBI path");
        }

        break;

    case OGS_FSM_EXIT_SIG:
        ausf_sbi_close();
        break;

    case AUSF_EVT_SBI_SERVER:
        sbi_request = e->sbi.request;
        ogs_assert(sbi_request);
        session = e->sbi.session;
        ogs_assert(session);
        server = e->sbi.server;
        ogs_assert(server);

        rv = ogs_sbi_parse_request(&message, sbi_request);
        if (rv != OGS_OK) {
            /* 'message' buffer is released in ogs_sbi_parse_request() */
            ogs_error("cannot parse HTTP message");
            ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                    NULL, "cannot parse HTTP message", NULL);
            break;
        }

        if (strcmp(message.h.api.version, OGS_SBI_API_VERSION) != 0) {
            ogs_error("Not supported version [%s]", message.h.api.version);
            ogs_sbi_server_send_error(session, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                    &message, "Not supported version", NULL);
            ogs_sbi_message_free(&message);
            break;
        }

        SWITCH(message.h.service.name)
        CASE(OGS_SBI_SERVICE_NAME_NRF_NFM)

            SWITCH(message.h.resource.name)
            CASE(OGS_SBI_RESOURCE_NAME_NF_STATUS_NOTIFY)
                SWITCH(message.h.method)
                CASE(OGS_SBI_HTTP_METHOD_POST)
                    ausf_nnrf_handle_nf_status_notify(
                            server, session, &message);
                    break;

                DEFAULT
                    ogs_error("Invalid HTTP method [%s]",
                            message.h.method);
                    ogs_sbi_server_send_error(session,
                            OGS_SBI_HTTP_STATUS_MEHTOD_NOT_ALLOWED,
                            &message,
                            "Invalid HTTP method", message.h.method);
                END
                break;

            DEFAULT
                ogs_error("Invalid resource name [%s]",
                        message.h.resource.name);
                ogs_sbi_server_send_error(session,
                        OGS_SBI_HTTP_STATUS_MEHTOD_NOT_ALLOWED, &message,
                        "Unknown resource name", message.h.resource.name);
            END
            break;

        CASE(OGS_SBI_SERVICE_NAME_AUSF_AUTH)
            SWITCH(message.h.method)
            CASE(OGS_SBI_HTTP_METHOD_POST)
                if (message.AuthenticationInfo)
                    ue_id = message.AuthenticationInfo->supi_or_suci;
                break;
            CASE(OGS_SBI_HTTP_METHOD_PUT)
                ue_id = message.h.resource.id;
                break;
            DEFAULT
            END

            if (!ue_id) {
                ogs_error("Not found [%s]", message.h.method);
                ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_NOT_FOUND,
                    &message, "Not found", message.h.method);
            }

            ausf_ue = ausf_ue_find(ue_id);
            if (!ausf_ue) {
                ausf_ue = ausf_ue_add(session, ue_id);
                ogs_assert(ausf_ue);
            }

            ogs_assert(ausf_ue);
            ogs_assert(OGS_FSM_STATE(&ausf_ue->sm));

            e->ausf_ue = ausf_ue;
            e->sbi.message = &message;
            ogs_fsm_dispatch(&ausf_ue->sm, e);
            if (OGS_FSM_CHECK(&ausf_ue->sm, ausf_ue_state_exception)) {
                ogs_error("State machine exception");
                ogs_sbi_message_free(&message);
                ausf_ue_remove(ausf_ue);
            }

            break;

        DEFAULT
            ogs_error("Invalid API name [%s]", message.h.service.name);
            ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_MEHTOD_NOT_ALLOWED, &message,
                    "Invalid API name", message.h.resource.name);
        END

        /* In lib/sbi/server.c, notify_completed() releases 'request' buffer. */
        ogs_sbi_message_free(&message);
        break;

    case AUSF_EVT_SBI_CLIENT:
        ogs_assert(e);

        response = e->sbi.response;
        ogs_assert(response);
        rv = ogs_sbi_parse_response(&message, response);
        if (rv != OGS_OK) {
            ogs_error("cannot parse HTTP response");
            ogs_sbi_message_free(&message);
            ogs_sbi_response_free(response);
            break;
        }

        if (strcmp(message.h.api.version, OGS_SBI_API_VERSION) != 0) {
            ogs_error("Not supported version [%s]", message.h.api.version);
            ogs_sbi_message_free(&message);
            ogs_sbi_response_free(response);
            break;
        }

        SWITCH(message.h.service.name)
        CASE(OGS_SBI_SERVICE_NAME_NRF_NFM)

            SWITCH(message.h.resource.name)
            CASE(OGS_SBI_RESOURCE_NAME_NF_INSTANCES)
                nf_instance = e->sbi.data;
                ogs_assert(nf_instance);
                ogs_assert(OGS_FSM_STATE(&nf_instance->sm));

                e->sbi.message = &message;
                ogs_fsm_dispatch(&nf_instance->sm, e);

                if (OGS_FSM_CHECK(&nf_instance->sm, ausf_nf_state_exception)) {
                    ogs_error("State machine exception");
                }
                break;

            CASE(OGS_SBI_RESOURCE_NAME_SUBSCRIPTIONS)
                subscription = e->sbi.data;
                ogs_assert(subscription);

                SWITCH(message.h.method)
                CASE(OGS_SBI_HTTP_METHOD_POST)
                    if (message.res_status == OGS_SBI_HTTP_STATUS_CREATED ||
                        message.res_status == OGS_SBI_HTTP_STATUS_OK) {
                        ausf_nnrf_handle_nf_status_subscribe(
                                subscription, &message);
                    } else {
                        ogs_error("HTTP response error : %d",
                                message.res_status);
                    }
                    break;

                CASE(OGS_SBI_HTTP_METHOD_DELETE)
                    if (message.res_status ==
                            OGS_SBI_HTTP_STATUS_NO_CONTENT) {
                        ogs_sbi_subscription_remove(subscription);
                    } else {
                        ogs_error("HTTP response error : %d",
                                message.res_status);
                    }
                    break;

                DEFAULT
                    ogs_error("Invalid HTTP method [%s]", message.h.method);
                END
                break;
            
            DEFAULT
                ogs_error("Invalid resource name [%s]",
                        message.h.resource.name);
            END
            break;

        CASE(OGS_SBI_SERVICE_NAME_NRF_DISC)
            SWITCH(message.h.resource.name)
            CASE(OGS_SBI_RESOURCE_NAME_NF_INSTANCES)
                session = e->sbi.data;
                ogs_assert(session);
                ausf_ue = ogs_sbi_session_get_data(session);
                ogs_assert(ausf_ue);

                SWITCH(message.h.method)
                CASE(OGS_SBI_HTTP_METHOD_GET)
                    if (message.res_status == OGS_SBI_HTTP_STATUS_OK) {
                        ogs_timer_stop(ausf_ue->sbi_message_wait.timer);

                        ausf_nnrf_handle_nf_discover(session, &message);
                    } else {
                        ogs_error("HTTP response error : %d",
                                message.res_status);
                    }
                    break;

                DEFAULT
                    ogs_error("Invalid HTTP method [%s]", message.h.method);
                END
                break;

            DEFAULT
                ogs_error("Invalid resource name [%s]",
                        message.h.resource.name);
            END
            break;

        DEFAULT
            ogs_error("Invalid API name [%s]", message.h.service.name);
        END

        ogs_sbi_message_free(&message);
        ogs_sbi_response_free(response);
        break;

    case AUSF_EVT_SBI_TIMER:
        ogs_assert(e);

        switch(e->timer_id) {
        case AUSF_TIMER_NF_INSTANCE_REGISTRATION_INTERVAL:
        case AUSF_TIMER_NF_INSTANCE_HEARTBEAT_INTERVAL:
        case AUSF_TIMER_NF_INSTANCE_HEARTBEAT:
        case AUSF_TIMER_NF_INSTANCE_VALIDITY:
            nf_instance = e->sbi.data;
            ogs_assert(nf_instance);
            ogs_assert(OGS_FSM_STATE(&nf_instance->sm));

            ogs_fsm_dispatch(&nf_instance->sm, e);
            if (OGS_FSM_CHECK(&nf_instance->sm, ausf_nf_state_exception))
                ogs_error("State machine exception [%d]", e->timer_id);
            break;

        case AUSF_TIMER_SUBSCRIPTION_VALIDITY:
            subscription = e->sbi.data;
            ogs_assert(subscription);

            ogs_info("[%s] Subscription validity expired", subscription->id);
            ogs_sbi_subscription_remove(subscription);

            ogs_nnrf_nfm_send_nf_status_subscribe(subscription->client,
                    ausf_self()->nf_type, subscription->nf_instance_id);
            break;

        case AUSF_TIMER_SBI_MESSAGE_WAIT:
            session = e->sbi.data;
            ogs_assert(session);
            ausf_ue = ogs_sbi_session_get_data(session);

            ogs_error("[%s] Cannot receive SBI message", ausf_ue->id);
            ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_SERVICE_UNAVAILABLE, NULL,
                    "Cannot receive SBI message", ausf_ue->id);
            break;
        default:
            ogs_error("Unknown timer[%s:%d]",
                    ausf_timer_get_name(e->timer_id), e->timer_id);
        }
        break;

    default:
        ogs_error("No handler for event %s", ausf_event_get_name(e));
        break;
    }
}
