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
#include "nas-path.h"
#include "nausf-build.h"
#include "nudm-build.h"

static int server_cb(ogs_sbi_server_t *server,
        ogs_sbi_session_t *session, ogs_sbi_request_t *request)
{
    amf_event_t *e = NULL;
    int rv;

    ogs_assert(session);
    ogs_assert(request);

    e = amf_event_new(AMF_EVT_SBI_SERVER);
    ogs_assert(e);

    e->sbi.server = server;
    e->sbi.session = session;
    e->sbi.request = request;

    rv = ogs_queue_push(amf_self()->queue, e);
    if (rv != OGS_OK) {
        ogs_warn("ogs_queue_push() failed:%d", (int)rv);
        amf_event_free(e);
        return OGS_ERROR;
    }

    return OGS_OK;
}

static int client_cb(ogs_sbi_response_t *response, void *data)
{
    amf_event_t *e = NULL;
    int rv;

    ogs_assert(response);

    e = amf_event_new(AMF_EVT_SBI_CLIENT);
    ogs_assert(e);
    e->sbi.response = response;
    e->sbi.data = data;

    rv = ogs_queue_push(amf_self()->queue, e);
    if (rv != OGS_OK) {
        ogs_warn("ogs_queue_push() failed:%d", (int)rv);
        amf_event_free(e);
        return OGS_ERROR;
    }

    return OGS_OK;
}

int amf_sbi_open(void)
{
    ogs_sbi_nf_instance_t *nf_instance = NULL;

    ogs_sbi_server_start_all(server_cb);

    ogs_list_for_each(&ogs_sbi_self()->nf_instance_list, nf_instance) {
        ogs_sbi_nf_service_t *service = NULL;

        ogs_sbi_nf_instance_build_default(nf_instance, amf_self()->nf_type);

        service = ogs_sbi_nf_service_build_default(nf_instance,
                (char*)OGS_SBI_SERVICE_NAME_NSMF_PDUSESSION);
        ogs_assert(service);
        ogs_sbi_nf_service_add_version(service, (char*)OGS_SBI_API_V1,
                (char*)OGS_SBI_API_V1_0_0, NULL);

        amf_nf_fsm_init(nf_instance);
        amf_sbi_setup_client_callback(nf_instance);
    }

    return OGS_OK;
}

void amf_sbi_close(void)
{
    ogs_sbi_server_stop_all();
}

void amf_sbi_setup_client_callback(ogs_sbi_nf_instance_t *nf_instance)
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

static ogs_sbi_nf_instance_t *find_or_discover_nf_instance(amf_ue_t *amf_ue)
{
    bool nrf = true;
    bool nf = true;

    ogs_assert(amf_ue);
    ogs_assert(amf_ue->sbi.discover.nf_type);

    if (!OGS_SBI_NF_INSTANCE_GET(amf_ue->nf_types, OpenAPI_nf_type_NRF))
        nrf = ogs_sbi_nf_types_associate(amf_ue->nf_types,
                OpenAPI_nf_type_NRF, amf_nf_state_registered);
    if (!OGS_SBI_NF_INSTANCE_GET(amf_ue->nf_types,
                amf_ue->sbi.discover.nf_type))
        nf = ogs_sbi_nf_types_associate(amf_ue->nf_types,
                amf_ue->sbi.discover.nf_type, amf_nf_state_registered);

    if (nrf == false && nf == false) {
        ogs_error("[%s] Cannot discover [%s]", amf_ue->suci,
                OpenAPI_nf_type_ToString(amf_ue->sbi.discover.nf_type));
        nas_5gs_send_nas_reject(
                amf_ue, OGS_5GMM_CAUSE_PROTOCOL_ERROR_UNSPECIFIED);
        return NULL;
    }

    if (nf == false) {
        ogs_warn("[%s] Try to discover [%s]", amf_ue->suci,
                OpenAPI_nf_type_ToString(amf_ue->sbi.discover.nf_type));
        ogs_timer_start(amf_ue->sbi_client_wait.timer,
                amf_timer_cfg(AMF_TIMER_SBI_CLIENT_WAIT)->duration);

        ogs_nnrf_disc_send_nf_discover(
            amf_ue->nf_types[OpenAPI_nf_type_NRF].nf_instance,
            amf_ue->sbi.discover.nf_type, OpenAPI_nf_type_AMF, amf_ue);

        return NULL;
    }

    return amf_ue->nf_types[amf_ue->sbi.discover.nf_type].nf_instance;
}

void amf_sbi_send(amf_ue_t *amf_ue, ogs_sbi_nf_instance_t *nf_instance)
{
    ogs_sbi_request_t *request = NULL;

    ogs_assert(amf_ue);
    request = amf_ue->sbi.discover.request;
    ogs_assert(request);

    ogs_assert(nf_instance);

    ogs_timer_start(amf_ue->sbi_client_wait.timer,
            amf_timer_cfg(AMF_TIMER_SBI_CLIENT_WAIT)->duration);

    ogs_sbi_client_send_request_to_nf_instance(nf_instance, request, amf_ue);
}

void amf_sbi_discover_and_send(
        amf_ue_t *amf_ue, OpenAPI_nf_type_e nf_type,
        ogs_sbi_request_t *(*build)(amf_ue_t *amf_ue))
{
    ogs_sbi_nf_instance_t *nf_instance = NULL;

    ogs_assert(amf_ue);
    ogs_assert(nf_type);
    ogs_assert(build);

    amf_ue->sbi.discover.nf_type = nf_type;
    if (amf_ue->sbi.discover.request)
        ogs_sbi_request_free(amf_ue->sbi.discover.request);
    amf_ue->sbi.discover.request = (*build)(amf_ue);

    if (!nf_instance)
        nf_instance = find_or_discover_nf_instance(amf_ue);

    if (!nf_instance) return;

    return amf_sbi_send(amf_ue, nf_instance);
}
