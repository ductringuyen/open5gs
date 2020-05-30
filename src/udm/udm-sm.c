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

void udm_state_initial(ogs_fsm_t *s, udm_event_t *e)
{
    udm_sm_debug(e);

    ogs_assert(s);

    OGS_FSM_TRAN(s, &udm_state_operational);
}

void udm_state_final(ogs_fsm_t *s, udm_event_t *e)
{
    udm_sm_debug(e);

    ogs_assert(s);
}

void udm_state_operational(ogs_fsm_t *s, udm_event_t *e)
{
    int rv;

    ogs_sbi_server_t *server = NULL;
    ogs_sbi_session_t *session = NULL;
    ogs_sbi_request_t *sbi_request = NULL;

    ogs_sbi_nf_instance_t *nf_instance = NULL;
    ogs_sbi_subscription_t *subscription = NULL;
    ogs_sbi_response_t *response = NULL;
    ogs_sbi_message_t message;

    udm_ue_t *udm_ue = NULL;
#if 0
    char *ue_id = NULL;
#endif

    udm_sm_debug(e);

    ogs_assert(s);

    switch (e->id) {
    case OGS_FSM_ENTRY_SIG:
        rv = udm_sbi_open();
        if (rv != OGS_OK) {
            ogs_fatal("Can't establish SBI path");
        }

        break;

    case OGS_FSM_EXIT_SIG:
        udm_sbi_close();
        break;

    case UDM_EVT_SBI_SERVER:
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
                    udm_nnrf_handle_nf_status_notify(
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

#if 0
        CASE(OGS_SBI_SERVICE_NAME_UDM_AUTH)
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

            udm_ue = udm_ue_find(ue_id);
            if (!udm_ue) {
                udm_ue = udm_ue_add(session, ue_id);
                ogs_assert(udm_ue);
            }

            ogs_assert(udm_ue);
            ogs_assert(OGS_FSM_STATE(&udm_ue->sm));

            e->udm_ue = udm_ue;
            e->sbi.message = &message;
            ogs_fsm_dispatch(&udm_ue->sm, e);
            if (OGS_FSM_CHECK(&udm_ue->sm, udm_ue_state_exception)) {
                ogs_error("[%s] State machine exception", udm_ue->id);
                ogs_sbi_message_free(&message);
                udm_ue_remove(udm_ue);
            }

            break;
#endif

        DEFAULT
            ogs_error("Invalid API name [%s]", message.h.service.name);
            ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_MEHTOD_NOT_ALLOWED, &message,
                    "Invalid API name", message.h.resource.name);
        END

        /* In lib/sbi/server.c, notify_completed() releases 'request' buffer. */
        ogs_sbi_message_free(&message);
        break;

    case UDM_EVT_SBI_CLIENT:
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

                if (OGS_FSM_CHECK(&nf_instance->sm, udm_nf_state_exception)) {
                    ogs_error("[%s] State machine exception", nf_instance->id);
                }
                break;

            CASE(OGS_SBI_RESOURCE_NAME_SUBSCRIPTIONS)
                subscription = e->sbi.data;
                ogs_assert(subscription);

                SWITCH(message.h.method)
                CASE(OGS_SBI_HTTP_METHOD_POST)
                    if (message.res_status == OGS_SBI_HTTP_STATUS_CREATED ||
                        message.res_status == OGS_SBI_HTTP_STATUS_OK) {
                        udm_nnrf_handle_nf_status_subscribe(
                                subscription, &message);
                    } else {
                        ogs_error("[%s] HTTP response error [%d]",
                                subscription->id, message.res_status);
                    }
                    break;

                CASE(OGS_SBI_HTTP_METHOD_DELETE)
                    if (message.res_status ==
                            OGS_SBI_HTTP_STATUS_NO_CONTENT) {
                        ogs_sbi_subscription_remove(subscription);
                    } else {
                        ogs_error("[%s] HTTP response error [%d]",
                                subscription->id, message.res_status);
                    }
                    break;

                DEFAULT
                    ogs_error("[%s] Invalid HTTP method [%s]",
                            subscription->id, message.h.method);
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
                udm_ue = ogs_sbi_session_get_data(session);
                ogs_assert(udm_ue);

                SWITCH(message.h.method)
                CASE(OGS_SBI_HTTP_METHOD_GET)
                    if (message.res_status == OGS_SBI_HTTP_STATUS_OK) {
                        ogs_timer_stop(udm_ue->sbi_message_wait.timer);

                        udm_nnrf_handle_nf_discover(session, &message);
                    } else {
                        ogs_error("[%s] HTTP response error [%d]",
                                udm_ue->id, message.res_status);
                    }
                    break;

                DEFAULT
                    ogs_error("[%s] Invalid HTTP method [%s]",
                            udm_ue->id, message.h.method);
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

    case UDM_EVT_SBI_TIMER:
        ogs_assert(e);

        switch(e->timer_id) {
        case UDM_TIMER_NF_INSTANCE_REGISTRATION_INTERVAL:
        case UDM_TIMER_NF_INSTANCE_HEARTBEAT_INTERVAL:
        case UDM_TIMER_NF_INSTANCE_HEARTBEAT:
        case UDM_TIMER_NF_INSTANCE_VALIDITY:
            nf_instance = e->sbi.data;
            ogs_assert(nf_instance);
            ogs_assert(OGS_FSM_STATE(&nf_instance->sm));

            ogs_fsm_dispatch(&nf_instance->sm, e);
            if (OGS_FSM_CHECK(&nf_instance->sm, udm_nf_state_exception))
                ogs_error("[%s] State machine exception [%d]",
                        nf_instance->id, e->timer_id);
            break;

        case UDM_TIMER_SUBSCRIPTION_VALIDITY:
            subscription = e->sbi.data;
            ogs_assert(subscription);

            ogs_info("[%s] Subscription validity expired", subscription->id);
            ogs_sbi_subscription_remove(subscription);

            ogs_nnrf_nfm_send_nf_status_subscribe(subscription->client,
                    udm_self()->nf_type, subscription->nf_instance_id);
            break;

        case UDM_TIMER_SBI_MESSAGE_WAIT:
            session = e->sbi.data;
            ogs_assert(session);
            udm_ue = ogs_sbi_session_get_data(session);

            ogs_error("[%s] Cannot receive SBI message", udm_ue->id);
            ogs_sbi_server_send_error(session,
                    OGS_SBI_HTTP_STATUS_SERVICE_UNAVAILABLE, NULL,
                    "Cannot receive SBI message", udm_ue->id);
            break;
        default:
            ogs_error("Unknown timer[%s:%d]",
                    udm_timer_get_name(e->timer_id), e->timer_id);
        }
        break;

    default:
        ogs_error("No handler for event %s", udm_event_get_name(e));
        break;
    }
}
