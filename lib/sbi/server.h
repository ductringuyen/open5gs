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

#if !defined(OGS_SBI_INSIDE) && !defined(OGS_SBI_COMPILATION)
#error "This header cannot be included directly."
#endif

#ifndef OGS_SBI_SERVER_H
#define OGS_SBI_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ogs_sbi_server_s ogs_sbi_server_t;
typedef struct ogs_sbi_session_s ogs_sbi_session_t;

typedef struct ogs_sbi_server_s {
    ogs_lnode_t     lnode;                  /* A node of list_t */

    ogs_sockaddr_t  *addr;                  /* Listen socket address */

    struct {
        const char  *key;
        const char  *pem;
    } tls;

    int (*cb)(ogs_sbi_server_t *server, ogs_sbi_session_t *session,
            ogs_sbi_request_t *request);
    void *data;

    ogs_list_t      suspended_session_list; /* MHD suspended list */

    void            *mhd;                   /* MHD instance */
    ogs_poll_t      *poll;                  /* MHD server poll */

} ogs_sbi_server_t;

void ogs_sbi_server_init(int num_of_connection_pool);
void ogs_sbi_server_final(void);

ogs_sbi_server_t *ogs_sbi_server_add(ogs_sockaddr_t *addr);
void ogs_sbi_server_remove(ogs_sbi_server_t *server);
void ogs_sbi_server_remove_all(void);

void ogs_sbi_server_start(ogs_sbi_server_t *server, int (*cb)(
            ogs_sbi_server_t *server, ogs_sbi_session_t *session,
            ogs_sbi_request_t *request));
void ogs_sbi_server_start_all(int (*cb)(
            ogs_sbi_server_t *server, ogs_sbi_session_t *session,
            ogs_sbi_request_t *request));
void ogs_sbi_server_stop(ogs_sbi_server_t *server);
void ogs_sbi_server_stop_all(void);

void ogs_sbi_server_send_response(
        ogs_sbi_session_t *session, ogs_sbi_response_t *response);
void ogs_sbi_server_send_error(ogs_sbi_session_t *session,
        int status, ogs_sbi_message_t *message,
        const char *title, const char *detail);
void ogs_sbi_server_send_problem(
        ogs_sbi_session_t *session, OpenAPI_problem_details_t *problem);

#ifdef __cplusplus
}
#endif

#endif /* OGS_SBI_SERVER_H */
