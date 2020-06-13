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

#include "context.h"
#include "sbi-path.h"

static int server_cb(ogs_sbi_server_t *server,
        ogs_sbi_session_t *session, ogs_sbi_request_t *request)
{
    smf_event_t *e = NULL;
    int rv;

    ogs_assert(session);
    ogs_assert(request);

    e = smf_event_new(SMF_EVT_SBI_SERVER);
    ogs_assert(e);

    e->sbi.server = server;
    e->sbi.session = session;
    e->sbi.request = request;

    rv = ogs_queue_push(smf_self()->queue, e);
    if (rv != OGS_OK) {
        ogs_warn("ogs_queue_push() failed:%d", (int)rv);
        smf_event_free(e);
        return OGS_ERROR;
    }

    return OGS_OK;
}

static int client_cb(ogs_sbi_response_t *response, void *data)
{
    smf_event_t *e = NULL;
    int rv;

    ogs_assert(response);

    e = smf_event_new(SMF_EVT_SBI_CLIENT);
    ogs_assert(e);
    e->sbi.response = response;
    e->sbi.data = data;

    rv = ogs_queue_push(smf_self()->queue, e);
    if (rv != OGS_OK) {
        ogs_warn("ogs_queue_push() failed:%d", (int)rv);
        smf_event_free(e);
        return OGS_ERROR;
    }

    return OGS_OK;
}

int smf_sbi_open(void)
{
    ogs_sbi_nf_instance_t *nf_instance = NULL;

    ogs_sbi_server_start_all(server_cb);

    ogs_list_for_each(&ogs_sbi_self()->nf_instance_list, nf_instance) {
        ogs_sbi_nf_service_t *service = NULL;

        ogs_sbi_nf_instance_build_default(nf_instance, smf_self()->nf_type);

        service = ogs_sbi_nf_service_build_default(nf_instance,
                (char*)OGS_SBI_SERVICE_NAME_NSMF_PDUSESSION);
        ogs_assert(service);
        ogs_sbi_nf_service_add_version(service, (char*)OGS_SBI_API_V1,
                (char*)OGS_SBI_API_V1_0_0, NULL);

        smf_nf_fsm_init(nf_instance);
        smf_sbi_setup_client_callback(nf_instance);
    }

    return OGS_OK;
}

void smf_sbi_close(void)
{
    ogs_sbi_server_stop_all();
}

void smf_sbi_setup_client_callback(ogs_sbi_nf_instance_t *nf_instance)
{
    ogs_sbi_client_t *client = NULL;
    ogs_sbi_nf_service_t *nf_service = NULL;
    ogs_assert(nf_instance);

    client = nf_instance->client;
    ogs_assert(client);

    client->cb = client_cb;

    ogs_list_for_each(&nf_instance->nf_service_list, nf_service) {
        client = nf_service->client;
        if (client)
            client->cb = client_cb;
    }
}

static ogs_sbi_nf_instance_t *find_or_discover_nf_instance(smf_sess_t *sess)
{
    ogs_sbi_session_t *session = NULL;
    bool nrf = true;
    bool nf = true;

    ogs_assert(sess);
    session = sess->session;
    ogs_assert(session);
    ogs_assert(sess->sbi.nf_type);

    if (!OGS_SBI_NF_INSTANCE_GET(sess->nf_types, OpenAPI_nf_type_NRF))
        nrf = ogs_sbi_nf_types_associate(sess->nf_types,
                OpenAPI_nf_type_NRF, smf_nf_state_registered);
    if (!OGS_SBI_NF_INSTANCE_GET(sess->nf_types,
                sess->sbi.nf_type))
        nf = ogs_sbi_nf_types_associate(sess->nf_types,
                sess->sbi.nf_type, smf_nf_state_registered);

    if (nrf == false && nf == false) {
        ogs_error("[%s] Cannot discover [%s]", sess->imsi_bcd,
                OpenAPI_nf_type_ToString(sess->sbi.nf_type));
        ogs_sbi_server_send_error(session,
                OGS_SBI_HTTP_STATUS_SERVICE_UNAVAILABLE, NULL,
                "Cannot discover", sess->imsi_bcd);
        return NULL;
    }

    if (nf == false) {
        ogs_warn("[%s] Try to discover [%s]", sess->imsi_bcd,
                OpenAPI_nf_type_ToString(sess->sbi.nf_type));
        ogs_timer_start(sess->sbi.client_wait_timer,
                smf_timer_cfg(SMF_TIMER_SBI_CLIENT_WAIT)->duration);

        ogs_nnrf_disc_send_nf_discover(
            sess->nf_types[OpenAPI_nf_type_NRF].nf_instance,
            sess->sbi.nf_type, OpenAPI_nf_type_SMF, sess);

        return NULL;
    }

    return sess->nf_types[sess->sbi.nf_type].nf_instance;
}

void smf_sbi_send(smf_sess_t *sess, ogs_sbi_nf_instance_t *nf_instance)
{
    ogs_sbi_request_t *request = NULL;

    ogs_assert(sess);
    request = sess->sbi.request;
    ogs_assert(request);

    ogs_assert(nf_instance);

    ogs_timer_start(sess->sbi.client_wait_timer,
            smf_timer_cfg(SMF_TIMER_SBI_CLIENT_WAIT)->duration);

    ogs_sbi_client_send_request_to_nf_instance(
            nf_instance, sess->sbi.request, sess);
}

void smf_sbi_discover_and_send(
        OpenAPI_nf_type_e nf_type, smf_sess_t *sess, void *data,
        ogs_sbi_request_t *(*build)(smf_sess_t *sess, void *data))
{
    ogs_sbi_nf_instance_t *nf_instance = NULL;

    ogs_assert(sess);
    ogs_assert(nf_type);
    ogs_assert(build);

    sess->sbi.nf_type = nf_type;
    if (sess->sbi.request)
        ogs_sbi_request_free(sess->sbi.request);
    sess->sbi.request = (*build)(sess, data);

    if (!nf_instance)
        nf_instance = find_or_discover_nf_instance(sess);

    if (!nf_instance) return;

    return smf_sbi_send(sess, nf_instance);
}

void smf_sbi_send_sm_context_create_error(
        ogs_sbi_session_t *session,
        int status, const char *title, const char *detail,
        ogs_pkbuf_t *gsmbuf)
{
    ogs_sbi_message_t sendmsg;
    ogs_sbi_response_t *response = NULL;

    OpenAPI_sm_context_create_error_t SMContextCreateError;
    OpenAPI_problem_details_t problem;
    OpenAPI_ref_to_binary_data_t n1_sm_msg;

    ogs_assert(session);

    memset(&problem, 0, sizeof(problem));
    problem.status = status;
    problem.title = (char*)title;
    problem.detail = (char*)detail;

    memset(&sendmsg, 0, sizeof(sendmsg));
    sendmsg.SMContextCreateError = &SMContextCreateError;

    memset(&SMContextCreateError, 0, sizeof(SMContextCreateError));
    SMContextCreateError.error = &problem;

    if (gsmbuf) {
        SMContextCreateError.n1_sm_msg = &n1_sm_msg;
        n1_sm_msg.content_id = (char *)OGS_SBI_MULTIPART_5GSM_ID;
        sendmsg.part[0].content_id = (char *)OGS_SBI_MULTIPART_5GSM_ID;
        sendmsg.part[0].content_type = (char *)OGS_SBI_CONTENT_5GNAS_TYPE;
        sendmsg.part[0].pkbuf = gsmbuf;
        sendmsg.num_of_part = 1;
    }

    response = ogs_sbi_build_response(&sendmsg, problem.status);
    ogs_assert(response);

    ogs_sbi_server_send_response(session, response);

    if (gsmbuf)
        ogs_pkbuf_free(gsmbuf);
}
