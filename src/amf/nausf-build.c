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

#include "nausf-build.h"

ogs_sbi_request_t *amf_nausf_build_authenticate(amf_ue_t *amf_ue)
{
    ogs_sbi_message_t message;
    ogs_sbi_request_t *request = NULL;

    OpenAPI_authentication_info_t *AuthenticationInfo = NULL;

    ogs_assert(amf_ue);

    memset(&message, 0, sizeof(message));
    message.h.method = (char *)OGS_SBI_HTTP_METHOD_POST;
    message.h.service.name = (char *)OGS_SBI_SERVICE_NAME_AUSF_AUTH;
    message.h.api.version = (char *)OGS_SBI_API_VERSION;
    message.h.resource.name = (char *)OGS_SBI_RESOURCE_NAME_UE_AUTHENTICATIONS;

    message.http.accept = (char *)(OGS_SBI_CONTENT_3GPPHAL_TYPE ","
                                    OGS_SBI_CONTENT_PROBLEM_TYPE);

    AuthenticationInfo = ogs_calloc(1, sizeof(*AuthenticationInfo));
    ogs_assert(AuthenticationInfo);

    ogs_assert(amf_ue->id);
    AuthenticationInfo->supi_or_suci = amf_ue->id;
    AuthenticationInfo->serving_network_name =
        ogs_plmn_id_string(&amf_ue->tai.plmn_id);

    message.AuthenticationInfo = AuthenticationInfo;

    request = ogs_sbi_build_request(&message);
    ogs_assert(request);

    ogs_free(AuthenticationInfo->serving_network_name);
    ogs_free(AuthenticationInfo);

    return request;
}
