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

#ifndef UDM_CONTEXT_H
#define UDM_CONTEXT_H

#include "ogs-app.h"
#include "ogs-sbi.h"

#include "udm-sm.h"
#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NUM_OF_SERVED_GUAMI     8

extern int __udm_log_domain;

#undef OGS_LOG_DOMAIN
#define OGS_LOG_DOMAIN __udm_log_domain

typedef struct udm_context_s {
    ogs_queue_t     *queue;         /* Queue for processing UPF control */
    ogs_timer_mgr_t *timer_mgr;     /* Timer Manager */
    ogs_pollset_t   *pollset;       /* Poll Set for I/O Multiplexing */

    OpenAPI_nf_type_e   nf_type;

    ogs_list_t      udm_ue_list;
    ogs_hash_t      *ueid_hash;

} udm_context_t;

struct udm_ue_s {
    ogs_lnode_t     lnode;
    ogs_fsm_t       sm;     /* A state machine */

    char *id;
    char *serving_network_name;
    char *ausf_instance_id;
    char *ue_id;

#define CLEAR_UDM_UE_ALL_TIMERS(__aUSF) \
    do { \
        CLEAR_UDM_UE_TIMER((__aUSF)->sbi_message_wait); \
    } while(0);
#define CLEAR_UDM_UE_TIMER(__aUSF_UE_TIMER) \
    do { \
        ogs_timer_stop((__aUSF_UE_TIMER).timer); \
        if ((__aUSF_UE_TIMER).pkbuf) { \
            ogs_pkbuf_free((__aUSF_UE_TIMER).pkbuf); \
            (__aUSF_UE_TIMER).pkbuf = NULL; \
        } \
        (__aUSF_UE_TIMER).retry_count = 0; \
    } while(0);
    struct {
        ogs_pkbuf_t     *pkbuf;
        ogs_timer_t     *timer;
        uint32_t        retry_count;;
    } sbi_message_wait;

#define UDM_NF_INSTANCE_CLEAR(_cAUSE, _nFInstance) \
    do { \
        ogs_assert(_nFInstance); \
        if ((_nFInstance)->reference_count == 1) { \
            ogs_info("[%s] (%s) NF removed", (_nFInstance)->id, (_cAUSE)); \
            udm_nf_fsm_fini((_nFInstance)); \
        } else { \
            /* There is an assocation with other context */ \
            ogs_info("[%s:%d] (%s) NF suspended", \
                    _nFInstance->id, _nFInstance->reference_count, (_cAUSE)); \
            OGS_FSM_TRAN(&_nFInstance->sm, udm_nf_state_de_registered); \
            ogs_fsm_dispatch(&_nFInstance->sm, NULL); \
        } \
        ogs_sbi_nf_instance_remove(_nFInstance); \
    } while(0)

    ogs_sbi_nf_types_t nf_types;
};

void udm_context_init(void);
void udm_context_final(void);
udm_context_t *udm_self(void);

int udm_context_parse_config(void);

udm_ue_t *udm_ue_add(ogs_sbi_session_t *session, char *id);
void udm_ue_remove(udm_ue_t *udm_ue);
void udm_ue_remove_all(void);
udm_ue_t *udm_ue_find(char *id);

#ifdef __cplusplus
}
#endif

#endif /* UDM_CONTEXT_H */
