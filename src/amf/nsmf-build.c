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

#include "nsmf-build.h"

ogs_sbi_request_t *amf_nsmf_pdu_session_build_create_sm_context(
        amf_ue_t *amf_ue, void *data)
{
    ogs_sbi_message_t message;
    ogs_sbi_request_t *request = NULL;

    ogs_sbi_server_t *server = NULL;
    ogs_sbi_header_t header;

    OpenAPI_sm_context_create_data_t SMContextCreateData;
    OpenAPI_plmn_id_nid_t plmn_id_nid;

    ogs_assert(amf_ue);
    ogs_assert(amf_ue->nas.access_type);

    memset(&message, 0, sizeof(message));
    message.h.method = (char *)OGS_SBI_HTTP_METHOD_POST;
    message.h.service.name = (char *)OGS_SBI_SERVICE_NAME_NSMF_PDUSESSION;
    message.h.api.version = (char *)OGS_SBI_API_V1;
    message.h.resource.component[0] =
        (char *)OGS_SBI_RESOURCE_NAME_SM_CONTEXTS;

    memset(&SMContextCreateData, 0, sizeof(SMContextCreateData));
    SMContextCreateData.unauthenticated_supi = -1;
    SMContextCreateData.indirect_forwarding_flag = -1;
    SMContextCreateData.cp_ciot_enabled = -1;
    SMContextCreateData.cp_only_ind = -1;
    SMContextCreateData.invoke_nef = -1;
    SMContextCreateData.ma_request_ind = -1;
    SMContextCreateData.ma_nw_upgrade_ind = -1;
    SMContextCreateData.extended_nas_sm_timer_ind = -1;
    SMContextCreateData.dl_data_waiting_ind = -1;
    SMContextCreateData.smf_transfer_ind = -1;

    SMContextCreateData.serving_nf_id = ogs_sbi_self()->nf_instance_id;

    plmn_id_nid.mcc = ogs_plmn_id_mcc_string(&amf_ue->tai.plmn_id);
    plmn_id_nid.mnc = ogs_plmn_id_mnc_string(&amf_ue->tai.plmn_id);
    plmn_id_nid.nid = NULL;
    SMContextCreateData.serving_network = &plmn_id_nid;

    SMContextCreateData.an_type = amf_ue->nas.access_type; 

    memset(&header, 0, sizeof(header));
    header.service.name = (char *)OGS_SBI_SERVICE_NAME_NAMF_CALLBACK;
    header.api.version = (char *)OGS_SBI_API_V1;
    header.resource.component[0] = amf_ue->supi;
    header.resource.component[1] =
        (char *)OGS_SBI_RESOURCE_NAME_SM_CONTEXT_STATUS;
    header.resource.component[2] = (char*)"10";

    server = ogs_list_first(&ogs_sbi_self()->server_list);
    ogs_assert(server);
    SMContextCreateData.sm_context_status_uri =
        ogs_sbi_server_uri(server, &header);

    message.SMContextCreateData = &SMContextCreateData;

    request = ogs_sbi_build_request(&message);
    ogs_assert(request);

    ogs_free(plmn_id_nid.mcc);
    ogs_free(plmn_id_nid.mnc);
    ogs_free(SMContextCreateData.sm_context_status_uri);

    return request;
}
