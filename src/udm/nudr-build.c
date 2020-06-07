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

#include "nudr-build.h"

ogs_sbi_request_t *udm_nudr_dr_build_query_authentication(
        udm_ue_t *udm_ue, void *data)
{
    ogs_sbi_message_t message;
    ogs_sbi_request_t *request = NULL;

    ogs_assert(udm_ue);

    memset(&message, 0, sizeof(message));
    message.h.method = (char *)OGS_SBI_HTTP_METHOD_GET;
    message.h.service.name = (char *)OGS_SBI_SERVICE_NAME_NUDR_DR;
    message.h.api.version = (char *)OGS_SBI_API_V1;
    message.h.resource.component[0] =
        (char *)OGS_SBI_RESOURCE_NAME_SUBSCRIPTION_DATA;
    message.h.resource.component[1] = udm_ue->supi;
    message.h.resource.component[2] =
        (char *)OGS_SBI_RESOURCE_NAME_AUTHENTICATION_DATA;
    message.h.resource.component[3] =
        (char *)OGS_SBI_RESOURCE_NAME_AUTHENTICATION_SUBSCRIPTION;

    request = ogs_sbi_build_request(&message);
    ogs_assert(request);

    return request;
}

ogs_sbi_request_t *udm_nudr_dr_build_update_authentication(
        udm_ue_t *udm_ue, void *data)
{
    ogs_sbi_message_t message;
    ogs_sbi_request_t *request = NULL;

    ogs_assert(udm_ue);
    ogs_assert(udm_ue->sbi.auth_event);

    memset(&message, 0, sizeof(message));
    message.h.method = (char *)OGS_SBI_HTTP_METHOD_PUT;
    message.h.service.name = (char *)OGS_SBI_SERVICE_NAME_NUDR_DR;
    message.h.api.version = (char *)OGS_SBI_API_V1;
    message.h.resource.component[0] =
        (char *)OGS_SBI_RESOURCE_NAME_SUBSCRIPTION_DATA;
    message.h.resource.component[1] = udm_ue->supi;
    message.h.resource.component[2] =
        (char *)OGS_SBI_RESOURCE_NAME_AUTHENTICATION_DATA;
    message.h.resource.component[3] =
        (char *)OGS_SBI_RESOURCE_NAME_AUTHENTICATION_STATUS;

    message.AuthEvent = OpenAPI_auth_event_copy(
            message.AuthEvent, udm_ue->sbi.auth_event);

    request = ogs_sbi_build_request(&message);
    ogs_assert(request);

    OpenAPI_auth_event_free(message.AuthEvent);

    return request;
}

ogs_sbi_request_t *udm_nudr_dr_build_update_context(
        udm_ue_t *udm_ue, void *data)
{
    ogs_sbi_message_t message;
    ogs_sbi_request_t *request = NULL;

    ogs_assert(udm_ue);
    ogs_assert(udm_ue->sbi.amf_3gpp_access_registration);

    memset(&message, 0, sizeof(message));
    message.h.method = (char *)OGS_SBI_HTTP_METHOD_PUT;
    message.h.service.name = (char *)OGS_SBI_SERVICE_NAME_NUDR_DR;
    message.h.api.version = (char *)OGS_SBI_API_V1;
    message.h.resource.component[0] =
        (char *)OGS_SBI_RESOURCE_NAME_SUBSCRIPTION_DATA;
    message.h.resource.component[1] = udm_ue->supi;
    message.h.resource.component[2] =
        (char *)OGS_SBI_RESOURCE_NAME_CONTEXT_DATA;
    message.h.resource.component[3] =
        (char *)OGS_SBI_RESOURCE_NAME_AMF_3GPP_ACCESS;

    message.Amf3GppAccessRegistration =
        OpenAPI_amf3_gpp_access_registration_copy(
            message.Amf3GppAccessRegistration,
                udm_ue->sbi.amf_3gpp_access_registration);

    request = ogs_sbi_build_request(&message);
    ogs_assert(request);

    OpenAPI_amf3_gpp_access_registration_free(
            message.Amf3GppAccessRegistration);

    return request;
}

ogs_sbi_request_t *udm_nudr_dr_build_query_provisioned(
        udm_ue_t *udm_ue, void *data)
{
    char buf[OGS_PLMNIDSTRLEN];

    ogs_sbi_message_t message;
    ogs_sbi_request_t *request = NULL;

    ogs_assert(udm_ue);
    ogs_assert(udm_ue->sbi.provisioned_resource);

    memset(&message, 0, sizeof(message));
    message.h.method = (char *)OGS_SBI_HTTP_METHOD_GET;
    message.h.service.name = (char *)OGS_SBI_SERVICE_NAME_NUDR_DR;
    message.h.api.version = (char *)OGS_SBI_API_V1;
    message.h.resource.component[0] =
        (char *)OGS_SBI_RESOURCE_NAME_SUBSCRIPTION_DATA;
    message.h.resource.component[1] = udm_ue->supi;
    message.h.resource.component[2] =
        (char *)ogs_plmn_id_to_string(&udm_ue->serving_plmn_id, buf);
    message.h.resource.component[3] =
        (char *)OGS_SBI_RESOURCE_NAME_PROVISIONED_DATA;
    message.h.resource.component[4] = (char *)udm_ue->sbi.provisioned_resource;

    request = ogs_sbi_build_request(&message);
    ogs_assert(request);

    return request;
}
