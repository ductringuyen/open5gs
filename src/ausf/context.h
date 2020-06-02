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

#ifndef AUSF_CONTEXT_H
#define AUSF_CONTEXT_H

#include "ogs-app.h"
#include "ogs-crypt.h"
#include "ogs-sbi.h"

#include "ausf-sm.h"
#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NUM_OF_SERVED_GUAMI     8

extern int __ausf_log_domain;

#undef OGS_LOG_DOMAIN
#define OGS_LOG_DOMAIN __ausf_log_domain

typedef struct ausf_context_s {
    ogs_queue_t     *queue;         /* Queue for processing UPF control */
    ogs_timer_mgr_t *timer_mgr;     /* Timer Manager */
    ogs_pollset_t   *pollset;       /* Poll Set for I/O Multiplexing */

    OpenAPI_nf_type_e   nf_type;

    ogs_list_t      ausf_ue_list;
    ogs_hash_t      *ueid_hash;

} ausf_context_t;

struct ausf_ue_s {
    ogs_lnode_t     lnode;
    ogs_fsm_t       sm;     /* A state machine */

    char *id;
    char *serving_network_name;

    uint8_t hxres_star[OGS_MAX_RES_LEN];
    uint8_t xres_star[OGS_MAX_RES_LEN];
    uint8_t rand[OGS_RAND_LEN];

    struct {
        ogs_timer_t *timer;
    } sbi_server_wait, sbi_client_wait;

#define AUSF_NF_INSTANCE_CLEAR(_cAUSE, _nFInstance) \
    do { \
        ogs_assert(_nFInstance); \
        if ((_nFInstance)->reference_count == 1) { \
            ogs_info("[%s] (%s) NF removed", (_nFInstance)->id, (_cAUSE)); \
            ausf_nf_fsm_fini((_nFInstance)); \
        } else { \
            /* There is an assocation with other context */ \
            ogs_info("[%s:%d] (%s) NF suspended", \
                    _nFInstance->id, _nFInstance->reference_count, (_cAUSE)); \
            OGS_FSM_TRAN(&_nFInstance->sm, ausf_nf_state_de_registered); \
            ogs_fsm_dispatch(&_nFInstance->sm, NULL); \
        } \
        ogs_sbi_nf_instance_remove(_nFInstance); \
    } while(0)

    ogs_sbi_nf_types_t nf_types;

    ogs_sbi_session_t *session;
};

void ausf_context_init(void);
void ausf_context_final(void);
ausf_context_t *ausf_self(void);

int ausf_context_parse_config(void);

ausf_ue_t *ausf_ue_add(char *id);
void ausf_ue_remove(ausf_ue_t *ausf_ue);
void ausf_ue_remove_all(void);
ausf_ue_t *ausf_ue_find(char *id);

#ifdef __cplusplus
}
#endif

#endif /* AUSF_CONTEXT_H */
