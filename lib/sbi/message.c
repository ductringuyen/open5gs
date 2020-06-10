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

#include "ogs-sbi.h"
#include "yuarel.h"

#include "gmime/gmime.h"

static OGS_POOL(request_pool, ogs_sbi_request_t);
static OGS_POOL(response_pool, ogs_sbi_response_t);

static int parse_header(
        ogs_sbi_message_t *message, ogs_sbi_header_t *header);
static void build_json(
        ogs_sbi_http_message_t *http, ogs_sbi_message_t *message);
static void build_content(
        ogs_sbi_http_message_t *http, ogs_sbi_message_t *message);

static int parse_json(ogs_sbi_message_t *message,
        char *content_type, char *json);
static int parse_multipart(
        ogs_sbi_message_t *sbi_message, ogs_sbi_http_message_t *http);
static int parse_content(
        ogs_sbi_message_t *message, ogs_sbi_http_message_t *http);

static void header_free(ogs_sbi_header_t *h);
static void http_message_free(ogs_sbi_http_message_t *http);

void ogs_sbi_message_init(int num_of_request_pool, int num_of_response_pool)
{
    ogs_pool_init(&request_pool, num_of_request_pool);
    ogs_pool_init(&response_pool, num_of_response_pool);
}

void ogs_sbi_message_final(void)
{
    ogs_pool_final(&request_pool);
    ogs_pool_final(&response_pool);
}

void ogs_sbi_message_free(ogs_sbi_message_t *message)
{
    int i;

    ogs_assert(message);

    if (message->NFProfile)
        OpenAPI_nf_profile_free(message->NFProfile);
    if (message->ProblemDetails)
        OpenAPI_problem_details_free(message->ProblemDetails);
    if (message->PatchItemList) {
        OpenAPI_lnode_t *node = NULL;
        OpenAPI_list_for_each(message->PatchItemList, node)
            OpenAPI_patch_item_free(node->data);
        OpenAPI_list_free(message->PatchItemList);
    }

    if (message->SubscriptionData)
        OpenAPI_subscription_data_free(message->SubscriptionData);
    if (message->NotificationData)
        OpenAPI_notification_data_free(message->NotificationData);
    if (message->SearchResult)
        OpenAPI_search_result_free(message->SearchResult);
    if (message->AuthenticationInfo)
        OpenAPI_authentication_info_free(message->AuthenticationInfo);
    if (message->AuthenticationInfoRequest)
        OpenAPI_authentication_info_request_free(
                message->AuthenticationInfoRequest);
    if (message->AuthenticationInfoResult)
        OpenAPI_authentication_info_result_free(
                message->AuthenticationInfoResult);
    if (message->AuthenticationSubscription)
        OpenAPI_authentication_subscription_free(
                message->AuthenticationSubscription);
    if (message->UeAuthenticationCtx)
        OpenAPI_ue_authentication_ctx_free(message->UeAuthenticationCtx);
    if (message->ConfirmationData)
        OpenAPI_confirmation_data_free(message->ConfirmationData);
    if (message->ConfirmationDataResponse)
        OpenAPI_confirmation_data_response_free(
                message->ConfirmationDataResponse);
    if (message->AuthEvent)
        OpenAPI_auth_event_free(message->AuthEvent);
    if (message->Amf3GppAccessRegistration)
        OpenAPI_amf3_gpp_access_registration_free(
                message->Amf3GppAccessRegistration);
    if (message->AuthEvent)
        OpenAPI_auth_event_free(message->AuthEvent);
    if (message->AccessAndMobilitySubscriptionData)
        OpenAPI_access_and_mobility_subscription_data_free(
                message->AccessAndMobilitySubscriptionData);
    if (message->SmfSelectionSubscriptionData)
        OpenAPI_smf_selection_subscription_data_free(
                message->SmfSelectionSubscriptionData);
    if (message->UeContextInSmfData)
        OpenAPI_ue_context_in_smf_data_free(message->UeContextInSmfData);
    if (message->SMContextCreateData)
        OpenAPI_sm_context_create_data_free(message->SMContextCreateData);

    if (message->gsm.buf)
        ogs_pkbuf_free(message->gsm.buf);

    for (i = 0; i < message->num_of_part; i++) {
        if (message->part[i].pkbuf)
            ogs_pkbuf_free(message->part[i].pkbuf);
    }
}

ogs_sbi_request_t *ogs_sbi_request_new(void)
{
    ogs_sbi_request_t *request = NULL;

    ogs_pool_alloc(&request_pool, &request);

    ogs_assert(request);
    memset(request, 0, sizeof(ogs_sbi_request_t));

    request->http.params = ogs_hash_make();
    request->http.headers = ogs_hash_make();

    return request;
}

ogs_sbi_response_t *ogs_sbi_response_new(void)
{
    ogs_sbi_response_t *response = NULL;

    ogs_pool_alloc(&response_pool, &response);
    ogs_assert(response);
    memset(response, 0, sizeof(ogs_sbi_response_t));

    response->http.params = ogs_hash_make();
    response->http.headers = ogs_hash_make();

    return response;
}

void ogs_sbi_request_free(ogs_sbi_request_t *request)
{
    ogs_assert(request);

    if (request->h.uri)
        ogs_free(request->h.uri);

    header_free(&request->h);
    http_message_free(&request->http);

    ogs_pool_free(&request_pool, request);
}

void ogs_sbi_response_free(ogs_sbi_response_t *response)
{
    ogs_assert(response);

    if (response->h.uri)
        ogs_free(response->h.uri);

    header_free(&response->h);
    http_message_free(&response->http);

    ogs_pool_free(&response_pool, response);
}

ogs_sbi_request_t *ogs_sbi_build_request(ogs_sbi_message_t *message)
{
    ogs_sbi_request_t *request = NULL;

    ogs_assert(message);

    request = ogs_sbi_request_new();
    ogs_assert(request);

    ogs_assert(message->h.method);
    request->h.method = ogs_strdup(message->h.method);
    if (message->h.uri) {
        request->h.uri = ogs_strdup(message->h.uri);
    } else {
        int i;

        ogs_assert(message->h.service.name);
        request->h.service.name = ogs_strdup(message->h.service.name);
        ogs_assert(message->h.api.version);
        request->h.api.version = ogs_strdup(message->h.api.version);

        ogs_assert(message->h.resource.component[0]);
        for (i = 0; i < OGS_SBI_MAX_NUM_OF_RESOURCE_COMPONENT &&
                            message->h.resource.component[i]; i++)
            request->h.resource.component[i] = ogs_strdup(
                    message->h.resource.component[i]);
    }

    /* URL Param */
    if (message->param.nf_type) {
        char *v = OpenAPI_nf_type_ToString(message->param.nf_type);
        ogs_assert(v);
        ogs_sbi_header_set(request->http.params, OGS_SBI_PARAM_NF_TYPE, v);
    }
    if (message->param.requester_nf_type) {
        char *v = OpenAPI_nf_type_ToString(message->param.requester_nf_type);
        ogs_assert(v);
        ogs_sbi_header_set(request->http.params,
                OGS_SBI_PARAM_REQUESTER_NF_TYPE, v);
    }
    if (message->param.target_nf_type) {
        char *v = OpenAPI_nf_type_ToString(message->param.target_nf_type);
        ogs_assert(v);
        ogs_sbi_header_set(request->http.params,
                OGS_SBI_PARAM_TARGET_NF_TYPE, v);
    }
    if (message->param.limit) {
        char *v = ogs_msprintf("%d", message->param.limit);
        ogs_assert(v);
        ogs_sbi_header_set(request->http.params, OGS_SBI_PARAM_LIMIT, v);
    }

    /* HTTP Message */
    build_content(&request->http, message);

    if (message->http.content_type) {
        ogs_sbi_header_set(request->http.headers,
                OGS_SBI_CONTENT_TYPE, message->http.content_type);
    } else {
        if (request->http.gsmbuf) {
            ogs_sbi_header_set(request->http.headers,
                    OGS_SBI_CONTENT_TYPE, OGS_SBI_CONTENT_MULTIPART_TYPE);
        } else if (request->http.content) {
            ogs_sbi_header_set(request->http.headers,
                    OGS_SBI_CONTENT_TYPE, OGS_SBI_CONTENT_JSON_TYPE);
        }
    }

    if (message->http.accept) {
        ogs_sbi_header_set(request->http.headers, OGS_SBI_ACCEPT,
                message->http.accept);
    } else {
        SWITCH(message->h.method)
        CASE(OGS_SBI_HTTP_METHOD_DELETE)
            ogs_sbi_header_set(request->http.headers, OGS_SBI_ACCEPT,
                OGS_SBI_CONTENT_PROBLEM_TYPE);
            break;
        DEFAULT
            if (request->http.gsmbuf)
                ogs_sbi_header_set(request->http.headers, OGS_SBI_ACCEPT,
                    OGS_SBI_CONTENT_MULTIPART_TYPE ","
                            OGS_SBI_CONTENT_PROBLEM_TYPE);
            else
                ogs_sbi_header_set(request->http.headers, OGS_SBI_ACCEPT,
                    OGS_SBI_CONTENT_JSON_TYPE "," OGS_SBI_CONTENT_PROBLEM_TYPE);
            break;
        END
    }

    if (message->http.content_encoding)
        ogs_sbi_header_set(request->http.headers,
                OGS_SBI_ACCEPT_ENCODING, message->http.content_encoding);

    return request;
}

ogs_sbi_response_t *ogs_sbi_build_response(
        ogs_sbi_message_t *message, int status)
{
    ogs_sbi_response_t *response = NULL;

    ogs_assert(message);

    response = ogs_sbi_response_new();
    ogs_assert(response);

    response->status = status;

    /* HTTP Message */
    if (response->status != OGS_SBI_HTTP_STATUS_NO_CONTENT) {
        build_content(&response->http, message);
        if (response->http.content) {
            if (message->http.content_type)
                ogs_sbi_header_set(response->http.headers,
                        OGS_SBI_CONTENT_TYPE, message->http.content_type);
            else
                ogs_sbi_header_set(response->http.headers,
                        OGS_SBI_CONTENT_TYPE, OGS_SBI_CONTENT_JSON_TYPE);
        }
    }

    if (message->http.location) {
        ogs_sbi_header_set(response->http.headers, "Location",
                message->http.location);
    }
    if (message->http.cache_control)
        ogs_sbi_header_set(response->http.headers, "Cache-Control",
                message->http.cache_control);

    return response;
}

int ogs_sbi_parse_request(
        ogs_sbi_message_t *message, ogs_sbi_request_t *request)
{
    int rv;
    ogs_hash_index_t *hi;

    ogs_assert(request);
    ogs_assert(message);

    rv = parse_header(message, &request->h);
    if (rv != OGS_OK) {
        ogs_error("sbi_parse_header() failed");
        return OGS_ERROR;
    }

    for (hi = ogs_hash_first(request->http.params);
            hi; hi = ogs_hash_next(hi)) {
        if (!strcmp(ogs_hash_this_key(hi), OGS_SBI_PARAM_NF_TYPE)) {
            message->param.nf_type =
                OpenAPI_nf_type_FromString(ogs_hash_this_val(hi));
        } else if (!strcmp(ogs_hash_this_key(hi),
                    OGS_SBI_PARAM_TARGET_NF_TYPE)) {
            message->param.target_nf_type =
                OpenAPI_nf_type_FromString(ogs_hash_this_val(hi));
        } else if (!strcmp(ogs_hash_this_key(hi),
                    OGS_SBI_PARAM_REQUESTER_NF_TYPE)) {
            message->param.requester_nf_type =
                OpenAPI_nf_type_FromString(ogs_hash_this_val(hi));
        } else if (!strcmp(ogs_hash_this_key(hi),
                    OGS_SBI_PARAM_LIMIT)) {
            message->param.limit = atoi(ogs_hash_this_val(hi));
        }
    }

    for (hi = ogs_hash_first(request->http.headers);
            hi; hi = ogs_hash_next(hi)) {
        if (!strcmp(ogs_hash_this_key(hi), OGS_SBI_ACCEPT_ENCODING)) {
            message->http.content_encoding = ogs_hash_this_val(hi);
        } else if (!strcmp(ogs_hash_this_key(hi), OGS_SBI_CONTENT_TYPE)) {
            message->http.content_type = ogs_hash_this_val(hi);
        } else if (!strcmp(ogs_hash_this_key(hi), OGS_SBI_ACCEPT)) {
            message->http.accept = ogs_hash_this_val(hi);
        }
    }

    if (parse_content(message, &request->http) != OGS_OK) {
        ogs_error("parse_content() failed");
        return OGS_ERROR;
    }

    return OGS_OK;
}

int ogs_sbi_parse_response(
        ogs_sbi_message_t *message, ogs_sbi_response_t *response)
{
    int rv;
    ogs_hash_index_t *hi;

    ogs_assert(response);
    ogs_assert(message);

    rv = parse_header(message, &response->h);
    if (rv != OGS_OK) {
        ogs_error("sbi_parse_header() failed");
        return OGS_ERROR;
    }

    for (hi = ogs_hash_first(response->http.headers);
            hi; hi = ogs_hash_next(hi)) {
        if (!strcmp(ogs_hash_this_key(hi), OGS_SBI_CONTENT_TYPE))
            message->http.content_type = ogs_hash_this_val(hi);
        else if (!strcmp(ogs_hash_this_key(hi), OGS_SBI_LOCATION))
            message->http.location = ogs_hash_this_val(hi);
    }

    message->res_status = response->status;

    if (parse_content(message, &response->http) != OGS_OK) {
        ogs_error("parse_content() failed");
        return OGS_ERROR;
    }

    return OGS_OK;
}

void ogs_sbi_header_set(ogs_hash_t *ht, const void *key, const void *val)
{
    ogs_hash_set(ht, key, strlen(key), ogs_strdup(val));
}

void *ogs_sbi_header_get(ogs_hash_t *ht, const void *key)
{
    return ogs_hash_get(ht, key, strlen(key));
}

static int parse_header(
        ogs_sbi_message_t *message, ogs_sbi_header_t *header)
{
    struct yuarel yuarel;
    char *saveptr = NULL;
    char *uri = NULL, *p = NULL;;

    char *component = NULL;
    int i = 0;

    ogs_assert(message);
    ogs_assert(header);

    memset(message, 0, sizeof(*message));

    message->h.method = header->method;
    ogs_assert(message->h.method);
    message->h.uri = header->uri;
    ogs_assert(message->h.uri);

    uri = ogs_strdup(header->uri);
    ogs_assert(uri);
    p = uri;

    if (p[0] != '/') {
        int rv = yuarel_parse(&yuarel, p);
        if (rv != OGS_OK) {
            ogs_error("yuarel_parse() failed");
            ogs_free(uri);
            return OGS_ERROR;
        }

        p = yuarel.path;
    }

    header->service.name = ogs_sbi_parse_uri(p, "/", &saveptr);
    if (!header->service.name) {
        ogs_error("ogs_sbi_parse_uri() failed");
        ogs_free(uri);
        return OGS_ERROR;
    }
    message->h.service.name = header->service.name;

    header->api.version = ogs_sbi_parse_uri(NULL, "/", &saveptr);
    if (!header->api.version) {
        ogs_error("ogs_sbi_parse_uri() failed");
        ogs_free(uri);
        return OGS_ERROR;
    }
    message->h.api.version = header->api.version;

    for (i = 0; i < OGS_SBI_MAX_NUM_OF_RESOURCE_COMPONENT &&
            (component = ogs_sbi_parse_uri(NULL, "/", &saveptr)) != NULL;
         i++) {
        header->resource.component[i] = component;
        message->h.resource.component[i] = component;
    }

    ogs_free(uri);

    return OGS_OK;
}

static void build_content(
        ogs_sbi_http_message_t *http, ogs_sbi_message_t *message)
{
    ogs_assert(message);
    ogs_assert(http);

    build_json(http, message);

    if (message->gsm.buf)
        http->gsmbuf = ogs_pkbuf_copy(message->gsm.buf);
}

static void build_json(
        ogs_sbi_http_message_t *http, ogs_sbi_message_t *message)
{
    cJSON *item = NULL;

    ogs_assert(message);
    ogs_assert(http);

    if (message->ProblemDetails) {
        item = OpenAPI_problem_details_convertToJSON(message->ProblemDetails);
        ogs_assert(item);
    } else if (message->NFProfile) {
        item = OpenAPI_nf_profile_convertToJSON(message->NFProfile);
        ogs_assert(item);
    } else if (message->PatchItemList) {
        OpenAPI_lnode_t *node = NULL;

        item = cJSON_CreateArray();
        ogs_assert(item);

        OpenAPI_list_for_each(message->PatchItemList, node) {
            cJSON *patchItem = OpenAPI_patch_item_convertToJSON(node->data);
            ogs_assert(patchItem);
            cJSON_AddItemToArray(item, patchItem);
        }
    } else if (message->SubscriptionData) {
        item = OpenAPI_subscription_data_convertToJSON(
                message->SubscriptionData);
        ogs_assert(item);
    } else if (message->NotificationData) {
        item = OpenAPI_notification_data_convertToJSON(
                message->NotificationData);
        ogs_assert(item);
    } else if (message->SearchResult) {
        item = OpenAPI_search_result_convertToJSON(message->SearchResult);
        ogs_assert(item);
    } else if (message->links) {
        item = ogs_sbi_links_convertToJSON(message->links);
        ogs_assert(item);
    } else if (message->AuthenticationInfo) {
        item = OpenAPI_authentication_info_convertToJSON(
                message->AuthenticationInfo);
        ogs_assert(item);
    } else if (message->AuthenticationInfoRequest) {
        item = OpenAPI_authentication_info_request_convertToJSON(
                message->AuthenticationInfoRequest);
        ogs_assert(item);
    } else if (message->AuthenticationInfoResult) {
        item = OpenAPI_authentication_info_result_convertToJSON(
                message->AuthenticationInfoResult);
        ogs_assert(item);
    } else if (message->AuthenticationSubscription) {
        item = OpenAPI_authentication_subscription_convertToJSON(
                message->AuthenticationSubscription);
        ogs_assert(item);
    } else if (message->UeAuthenticationCtx) {
        item = OpenAPI_ue_authentication_ctx_convertToJSON(
                message->UeAuthenticationCtx);
        ogs_assert(item);
    } else if (message->ConfirmationData) {
        item = OpenAPI_confirmation_data_convertToJSON(
                message->ConfirmationData);
        ogs_assert(item);
    } else if (message->ConfirmationDataResponse) {
        item = OpenAPI_confirmation_data_response_convertToJSON(
                message->ConfirmationDataResponse);
        ogs_assert(item);
    } else if (message->AuthEvent) {
        item = OpenAPI_auth_event_convertToJSON(message->AuthEvent);
        ogs_assert(item);
    } else if (message->Amf3GppAccessRegistration) {
        item = OpenAPI_amf3_gpp_access_registration_convertToJSON(
                message->Amf3GppAccessRegistration);
        ogs_assert(item);
    } else if (message->AccessAndMobilitySubscriptionData) {
        item = OpenAPI_access_and_mobility_subscription_data_convertToJSON(
                message->AccessAndMobilitySubscriptionData);
        ogs_assert(item);
    } else if (message->SmfSelectionSubscriptionData) {
        item = OpenAPI_smf_selection_subscription_data_convertToJSON(
                message->SmfSelectionSubscriptionData);
        ogs_assert(item);
    } else if (message->UeContextInSmfData) {
        item = OpenAPI_ue_context_in_smf_data_convertToJSON(
                message->UeContextInSmfData);
        ogs_assert(item);
    } else if (message->SMContextCreateData) {
        item = OpenAPI_sm_context_create_data_convertToJSON(
                message->SMContextCreateData);
        ogs_assert(item);
    }

    if (item) {
        http->content = cJSON_Print(item);
        ogs_assert(http->content);
        cJSON_Delete(item);
    }
}

static int parse_content(
        ogs_sbi_message_t *message, ogs_sbi_http_message_t *http)
{
    ogs_assert(message);
    ogs_assert(http);

    if (message->http.content_type &&
        !strncmp(message->http.content_type, OGS_SBI_CONTENT_MULTIPART_TYPE,
            strlen(OGS_SBI_CONTENT_MULTIPART_TYPE))) {
        int rv, i;

        rv = parse_multipart(message, http);
        message->num_of_part = http->num_of_part;
        for (i = 0; i < message->num_of_part; i++) {
            message->part[i].content_id = http->part[i].content_id;
            message->part[i].pkbuf = ogs_pkbuf_copy(http->part[i].pkbuf);
        }

        return rv;
    } else {
        return parse_json(message, message->http.content_type, http->content);
    }
}

static int parse_multipart(
        ogs_sbi_message_t *sbi_message, ogs_sbi_http_message_t *http)
{
    GMimeMessage *mime_message = NULL;
    GMimeParser *parser = NULL;
    GMimeStream *stream = NULL;
    GMimePartIter *iter = NULL;

    ogs_pkbuf_t *pkbuf = NULL;

    ogs_assert(sbi_message);
    ogs_assert(http);

    if (!http->content)
        return OGS_OK;

    ogs_assert(sbi_message->http.content_type);
    ogs_assert(http->content_length);

    pkbuf = ogs_pkbuf_alloc(NULL, OGS_MAX_SDU_LEN);
    ogs_pkbuf_put(pkbuf, OGS_MAX_SDU_LEN);
    ogs_snprintf((char *)pkbuf->data, OGS_MAX_SDU_LEN, "%s: %s\r\n\r\n",
            OGS_SBI_CONTENT_TYPE, sbi_message->http.content_type);
    ogs_pkbuf_trim(pkbuf, strlen((char*)pkbuf->data));
    ogs_pkbuf_put_data(pkbuf, http->content, http->content_length);

    stream = g_mime_stream_mem_new_with_buffer(
            (const char *)pkbuf->data, pkbuf->len);
    parser = g_mime_parser_new_with_stream(stream);
    g_object_unref(stream);
    mime_message = g_mime_parser_construct_message(parser, NULL);
    g_object_unref(parser);

    iter = g_mime_part_iter_new(mime_message->mime_part);
    if (g_mime_part_iter_is_valid(iter)) {
        GMimeObject *parent = g_mime_part_iter_get_parent(iter);

        do {
            GMimeObject *current = g_mime_part_iter_get_current(iter);
            GMimePart *part = (GMimePart *)current;
            const GMimeContentType *type = NULL;
            const char *content_id = NULL;
            GMimeDataWrapper *content = NULL;
            char *content_type = NULL;
            char buf[OGS_HUGE_LEN];
            int len, n;

            if (GMIME_IS_MULTIPART(parent) && GMIME_IS_PART(current)) {

                type = g_mime_object_get_content_type(current);
                if (!type) {
                    ogs_error("No Content-Type");
                    break;
                }

                content = g_mime_part_get_content(part);
                if (!content) {
                    ogs_error("No Content");
                    break;
                }

                content_id = g_mime_object_get_content_id(current);

                content_type = ogs_msprintf("%s/%s", type->type, type->subtype);
                ogs_assert(content_type);

                stream = g_mime_data_wrapper_get_stream(content);
                ogs_assert(stream);
                len = g_mime_stream_length(stream);
                if (len == -1) {
                    ogs_error("Unable to get length");
                    break;
                }

                n = g_mime_stream_read(stream, buf, sizeof(buf));
                if (len != n) {
                    ogs_error("Invalid length [%d != %d]", n, len);
                    break;
                }

                SWITCH(content_type)
                CASE(OGS_SBI_CONTENT_JSON_TYPE)
                    parse_json(sbi_message, content_type, buf);
                    break;

                CASE(OGS_SBI_CONTENT_5GNAS_TYPE)
                    if (http->num_of_part < OGS_SBI_MAX_NUM_OF_PART) {
                        http->part[http->num_of_part].content_id =
                                ogs_strdup(content_id);
                        http->part[http->num_of_part].pkbuf =
                                ogs_pkbuf_alloc(NULL, n);
                        ogs_pkbuf_put_data(
                                http->part[http->num_of_part].pkbuf, buf, n);
                        http->num_of_part++;
                    }
                    break;

                DEFAULT
                    ogs_error("Unknown subtype [%s]", type->subtype);
                END

                ogs_free(content_type);
            }
        } while (g_mime_part_iter_next(iter));
    }

    g_mime_part_iter_free(iter);

    g_object_unref(mime_message);
    ogs_pkbuf_free(pkbuf);

    return OGS_OK;
}

static int parse_json(ogs_sbi_message_t *message,
        char *content_type, char *json)
{
    int rv = OGS_OK;
    cJSON *item = NULL;

    ogs_assert(message);

    if (!json)
        return OGS_OK;

    item = cJSON_Parse(json);
    if (!item) {
        ogs_error("JSON parse error");
        return OGS_ERROR;
    }

    if (content_type &&
        !strncmp(content_type, OGS_SBI_CONTENT_PROBLEM_TYPE,
            strlen(OGS_SBI_CONTENT_PROBLEM_TYPE))) {
        message->ProblemDetails = OpenAPI_problem_details_parseFromJSON(item);
    } else if (content_type &&
                !strncmp(content_type, OGS_SBI_CONTENT_PATCH_TYPE,
                    strlen(OGS_SBI_CONTENT_PATCH_TYPE))) {
        if (item) {
            OpenAPI_patch_item_t *patch_item = NULL;
            cJSON *patchJSON = NULL;
            message->PatchItemList = OpenAPI_list_create();
            cJSON_ArrayForEach(patchJSON, item) {
                if (!cJSON_IsObject(patchJSON)) {
                    rv = OGS_ERROR;
                    ogs_error("Unknown JSON");
                    goto cleanup;
                }

                patch_item = OpenAPI_patch_item_parseFromJSON(patchJSON);
                OpenAPI_list_add(message->PatchItemList, patch_item);
            }
        }
    } else {
        SWITCH(message->h.service.name)
        CASE(OGS_SBI_SERVICE_NAME_NNRF_NFM)

            SWITCH(message->h.resource.component[0])
            CASE(OGS_SBI_RESOURCE_NAME_NF_INSTANCES)
                message->NFProfile =
                    OpenAPI_nf_profile_parseFromJSON(item);
                if (!message->NFProfile) {
                    rv = OGS_ERROR;
                    ogs_error("JSON parse error");
                }
                break;

            CASE(OGS_SBI_RESOURCE_NAME_SUBSCRIPTIONS)
                message->SubscriptionData =
                    OpenAPI_subscription_data_parseFromJSON(item);
                if (!message->SubscriptionData) {
                    rv = OGS_ERROR;
                    ogs_error("JSON parse error");
                }
                break;

            CASE(OGS_SBI_RESOURCE_NAME_NF_STATUS_NOTIFY)
                message->NotificationData =
                    OpenAPI_notification_data_parseFromJSON(item);
                if (!message->NotificationData) {
                    rv = OGS_ERROR;
                    ogs_error("JSON parse error");
                }
                break;

            DEFAULT
                rv = OGS_ERROR;
                ogs_error("Unknown resource name [%s]",
                        message->h.resource.component[0]);
            END
            break;

        CASE(OGS_SBI_SERVICE_NAME_NNRF_DISC)
            SWITCH(message->h.resource.component[0])
            CASE(OGS_SBI_RESOURCE_NAME_NF_INSTANCES)
                message->SearchResult =
                    OpenAPI_search_result_parseFromJSON(item);
                if (!message->SearchResult) {
                    rv = OGS_ERROR;
                    ogs_error("JSON parse error");
                }
                break;

            DEFAULT
                rv = OGS_ERROR;
                ogs_error("Unknown resource name [%s]",
                        message->h.resource.component[0]);
            END
            break;

        CASE(OGS_SBI_SERVICE_NAME_NAUSF_AUTH)
            SWITCH(message->h.resource.component[0])
            CASE(OGS_SBI_RESOURCE_NAME_UE_AUTHENTICATIONS)
                SWITCH(message->h.method)
                CASE(OGS_SBI_HTTP_METHOD_POST)
                    if (message->res_status ==
                            OGS_SBI_HTTP_STATUS_CREATED) {
                        message->UeAuthenticationCtx =
                        OpenAPI_ue_authentication_ctx_parseFromJSON(item);
                        if (!message->UeAuthenticationCtx) {
                            rv = OGS_ERROR;
                            ogs_error("JSON parse error");
                        }
                    } else {
                        message->AuthenticationInfo =
                            OpenAPI_authentication_info_parseFromJSON(item);
                        if (!message->AuthenticationInfo) {
                            rv = OGS_ERROR;
                            ogs_error("JSON parse error");
                        }
                    }
                    break;
                CASE(OGS_SBI_HTTP_METHOD_PUT)
                    if (message->res_status == OGS_SBI_HTTP_STATUS_OK) {
                        message->ConfirmationDataResponse =
                            OpenAPI_confirmation_data_response_parseFromJSON(
                                    item);
                        if (!message->ConfirmationDataResponse) {
                            rv = OGS_ERROR;
                            ogs_error("JSON parse error");
                        }
                    } else {
                        message->ConfirmationData =
                            OpenAPI_confirmation_data_parseFromJSON(item);
                        if (!message->ConfirmationData) {
                            rv = OGS_ERROR;
                            ogs_error("JSON parse error");
                        }
                    }
                    break;
                DEFAULT
                    rv = OGS_ERROR;
                    ogs_error("Unknown method [%s]", message->h.method);
                END
                break;

            DEFAULT
                rv = OGS_ERROR;
                ogs_error("Unknown resource name [%s]",
                        message->h.resource.component[0]);
            END
            break;

        CASE(OGS_SBI_SERVICE_NAME_NUDM_UEAU)
            SWITCH(message->h.resource.component[1])
            CASE(OGS_SBI_RESOURCE_NAME_SECURITY_INFORMATION)
                SWITCH(message->h.resource.component[2])
                CASE(OGS_SBI_RESOURCE_NAME_GENERATE_AUTH_DATA)
                    if (message->res_status == OGS_SBI_HTTP_STATUS_OK) {
                        message->AuthenticationInfoResult =
                        OpenAPI_authentication_info_result_parseFromJSON(
                                item);
                        if (!message->AuthenticationInfoResult) {
                            rv = OGS_ERROR;
                            ogs_error("JSON parse error");
                        }
                    } else {
                        message->AuthenticationInfoRequest =
                        OpenAPI_authentication_info_request_parseFromJSON(
                                item);
                        if (!message->AuthenticationInfoRequest) {
                            rv = OGS_ERROR;
                            ogs_error("JSON parse error");
                        }
                    }
                    break;
                DEFAULT
                    rv = OGS_ERROR;
                    ogs_error("Unknown resource name [%s]",
                            message->h.resource.component[2]);
                END
                break;

            CASE(OGS_SBI_RESOURCE_NAME_AUTH_EVENTS)
                message->AuthEvent = OpenAPI_auth_event_parseFromJSON(item);
                if (!message->AuthEvent) {
                    rv = OGS_ERROR;
                    ogs_error("JSON parse error");
                }
                break;

            DEFAULT
                rv = OGS_ERROR;
                ogs_error("Unknown resource name [%s]",
                        message->h.resource.component[1]);
            END
            break;

        CASE(OGS_SBI_SERVICE_NAME_NUDM_UECM)
            SWITCH(message->h.resource.component[1])
            CASE(OGS_SBI_RESOURCE_NAME_REGISTRATIONS)
                SWITCH(message->h.resource.component[2])
                CASE(OGS_SBI_RESOURCE_NAME_AMF_3GPP_ACCESS)
                    message->Amf3GppAccessRegistration =
                        OpenAPI_amf3_gpp_access_registration_parseFromJSON(
                                item);
                    if (!message->Amf3GppAccessRegistration) {
                        rv = OGS_ERROR;
                        ogs_error("JSON parse error");
                    }
                    break;
                DEFAULT
                    rv = OGS_ERROR;
                    ogs_error("Unknown resource name [%s]",
                            message->h.resource.component[2]);
                END
                break;

            DEFAULT
                rv = OGS_ERROR;
                ogs_error("Unknown resource name [%s]",
                        message->h.resource.component[1]);
            END
            break;

        CASE(OGS_SBI_SERVICE_NAME_NUDM_SDM)
            SWITCH(message->h.resource.component[1])
            CASE(OGS_SBI_RESOURCE_NAME_AM_DATA)
                message->AccessAndMobilitySubscriptionData =
                    OpenAPI_access_and_mobility_subscription_data_parseFromJSON(
                            item);
                if (!message->AccessAndMobilitySubscriptionData) {
                    rv = OGS_ERROR;
                    ogs_error("JSON parse error");
                }
                break;

            CASE(OGS_SBI_RESOURCE_NAME_SMF_SELECT_DATA)
                message->SmfSelectionSubscriptionData =
                    OpenAPI_smf_selection_subscription_data_parseFromJSON(item);
                if (!message->SmfSelectionSubscriptionData) {
                    rv = OGS_ERROR;
                    ogs_error("JSON parse error");
                }
                break;

            CASE(OGS_SBI_RESOURCE_NAME_UE_CONTEXT_IN_SMF_DATA)
                message->UeContextInSmfData =
                    OpenAPI_ue_context_in_smf_data_parseFromJSON(item);
                if (!message->UeContextInSmfData) {
                    rv = OGS_ERROR;
                    ogs_error("JSON parse error");
                }
                break;

            DEFAULT
                rv = OGS_ERROR;
                ogs_error("Unknown resource name [%s]",
                        message->h.resource.component[1]);
            END
            break;

        CASE(OGS_SBI_SERVICE_NAME_NUDR_DR)
            SWITCH(message->h.resource.component[0])
            CASE(OGS_SBI_RESOURCE_NAME_SUBSCRIPTION_DATA)
                SWITCH(message->h.resource.component[2])
                CASE(OGS_SBI_RESOURCE_NAME_AUTHENTICATION_DATA)
                    SWITCH(message->h.resource.component[3])
                    CASE(OGS_SBI_RESOURCE_NAME_AUTHENTICATION_SUBSCRIPTION)
                        if (message->res_status == OGS_SBI_HTTP_STATUS_OK) {
                            message->AuthenticationSubscription =
                                OpenAPI_authentication_subscription_parseFromJSON(item);
                            if (!message->AuthenticationSubscription) {
                                rv = OGS_ERROR;
                                ogs_error("JSON parse error");
                            }
                        }
                        break;
                    CASE(OGS_SBI_RESOURCE_NAME_AUTHENTICATION_STATUS)
                        message->AuthEvent =
                            OpenAPI_auth_event_parseFromJSON(item);
                        if (!message->AuthEvent) {
                            rv = OGS_ERROR;
                            ogs_error("JSON parse error");
                        }
                        break;
                    DEFAULT
                        rv = OGS_ERROR;
                        ogs_error("Unknown resource name [%s]",
                                message->h.resource.component[3]);
                    END
                    break;

                CASE(OGS_SBI_RESOURCE_NAME_CONTEXT_DATA)
                    message->Amf3GppAccessRegistration =
                        OpenAPI_amf3_gpp_access_registration_parseFromJSON(
                                item);
                    if (!message->Amf3GppAccessRegistration) {
                        rv = OGS_ERROR;
                        ogs_error("JSON parse error");
                    }
                    break;

                DEFAULT
                    SWITCH(message->h.resource.component[3])
                    CASE(OGS_SBI_RESOURCE_NAME_PROVISIONED_DATA)
                        SWITCH(message->h.resource.component[4])
                        CASE(OGS_SBI_RESOURCE_NAME_AM_DATA)
                            message->AccessAndMobilitySubscriptionData =
                                OpenAPI_access_and_mobility_subscription_data_parseFromJSON(item);
                            if (!message->AccessAndMobilitySubscriptionData) {
                                rv = OGS_ERROR;
                                ogs_error("JSON parse error");
                            }
                            break;

                        CASE(OGS_SBI_RESOURCE_NAME_SMF_SELECT_DATA)
                            message->SmfSelectionSubscriptionData =
                                OpenAPI_smf_selection_subscription_data_parseFromJSON(item);
                            if (!message->SmfSelectionSubscriptionData) {
                                rv = OGS_ERROR;
                                ogs_error("JSON parse error");
                            }
                            break;

                        CASE(OGS_SBI_RESOURCE_NAME_UE_CONTEXT_IN_SMF_DATA)
                            message->UeContextInSmfData =
                                OpenAPI_ue_context_in_smf_data_parseFromJSON(
                                        item);
                            if (!message->UeContextInSmfData) {
                                rv = OGS_ERROR;
                                ogs_error("JSON parse error");
                            }
                            break;

                        DEFAULT
                            rv = OGS_ERROR;
                            ogs_error("Unknown resource name [%s]",
                                    message->h.resource.component[4]);
                        END
                        break;

                    DEFAULT
                        rv = OGS_ERROR;
                        ogs_error("Unknown resource name [%s]",
                                message->h.resource.component[3]);
                    END
                END
                break;

            DEFAULT
                rv = OGS_ERROR;
                ogs_error("Unknown resource name [%s]",
                        message->h.resource.component[0]);
            END
            break;

        CASE(OGS_SBI_SERVICE_NAME_NSMF_PDUSESSION)
            SWITCH(message->h.resource.component[0])
            CASE(OGS_SBI_RESOURCE_NAME_SM_CONTEXTS)
                message->SMContextCreateData =
                    OpenAPI_sm_context_create_data_parseFromJSON(item);
                if (!message->SMContextCreateData) {
                    rv = OGS_ERROR;
                    ogs_error("JSON parse error");
                }
                break;

            DEFAULT
                rv = OGS_ERROR;
                ogs_error("Unknown resource name [%s]",
                        message->h.resource.component[0]);
            END
            break;

        DEFAULT
            rv = OGS_ERROR;
            ogs_error("Not implemented API name [%s]",
                    message->h.service.name);
        END
    }

cleanup:

    cJSON_Delete(item);
    return rv;
}

static void header_free(ogs_sbi_header_t *h)
{
    int i;
    ogs_assert(h);

    if (h->method) ogs_free(h->method);
    if (h->service.name) ogs_free(h->service.name);
    if (h->api.version) ogs_free(h->api.version);

    for (i = 0; i < OGS_SBI_MAX_NUM_OF_RESOURCE_COMPONENT &&
                        h->resource.component[i]; i++)
        ogs_free(h->resource.component[i]);
}

static void http_message_free(ogs_sbi_http_message_t *http)
{
    int i;
    ogs_assert(http);

    if (http->params) {
        ogs_hash_index_t *hi;
        for (hi = ogs_hash_first(http->params); hi; hi = ogs_hash_next(hi)) {
            char *val = ogs_hash_this_val(hi);
            ogs_free(val);
        }
        ogs_hash_destroy(http->params);
    }

    if (http->headers) {
        ogs_hash_index_t *hi;
        for (hi = ogs_hash_first(http->headers); hi; hi = ogs_hash_next(hi)) {
            char *val = ogs_hash_this_val(hi);
            ogs_free(val);
        }
        ogs_hash_destroy(http->headers);
    }
    if (http->content)
        ogs_free(http->content);

    if (http->gsmbuf)
        ogs_pkbuf_free(http->gsmbuf);

    for (i = 0; i < http->num_of_part; i++) {
        if (http->part[i].pkbuf)
            ogs_pkbuf_free(http->part[i].pkbuf);
        if (http->part[i].content_id)
            ogs_free(http->part[i].content_id);
    }
}
