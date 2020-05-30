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

#ifndef UDR_CONTEXT_H
#define UDR_CONTEXT_H

#include "ogs-app.h"
#include "ogs-sbi.h"

#include "udr-sm.h"
#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NUM_OF_SERVED_GUAMI     8

extern int __udr_log_domain;

#undef OGS_LOG_DOMAIN
#define OGS_LOG_DOMAIN __udr_log_domain

typedef struct udr_context_s {
    ogs_queue_t     *queue;         /* Queue for processing UPF control */
    ogs_timer_mgr_t *timer_mgr;     /* Timer Manager */
    ogs_pollset_t   *pollset;       /* Poll Set for I/O Multiplexing */

    OpenAPI_nf_type_e   nf_type;

} udr_context_t;

struct udr_ue_s {
    ogs_lnode_t     lnode;
    ogs_fsm_t       sm;     /* A state machine */

    char *id;
    char *serving_network_name;
    char *ausf_instance_id;

#define CLEAR_UDR_UE_ALL_TIMERS(__aUSF) \
    do { \
        CLEAR_UDR_UE_TIMER((__aUSF)->sbi_message_wait); \
    } while(0);
#define CLEAR_UDR_UE_TIMER(__aUSF_UE_TIMER) \
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

#define UDR_NF_INSTANCE_CLEAR(_cAUSE, _nFInstance) \
    do { \
        ogs_assert(_nFInstance); \
        if ((_nFInstance)->reference_count == 1) { \
            ogs_info("[%s] (%s) NF removed", (_nFInstance)->id, (_cAUSE)); \
            udr_nf_fsm_fini((_nFInstance)); \
        } else { \
            /* There is an assocation with other context */ \
            ogs_info("[%s:%d] (%s) NF suspended", \
                    _nFInstance->id, _nFInstance->reference_count, (_cAUSE)); \
            OGS_FSM_TRAN(&_nFInstance->sm, udr_nf_state_de_registered); \
            ogs_fsm_dispatch(&_nFInstance->sm, NULL); \
        } \
        ogs_sbi_nf_instance_remove(_nFInstance); \
    } while(0)

    ogs_sbi_nf_types_t nf_types;
};

void udr_context_init(void);
void udr_context_final(void);
udr_context_t *udr_self(void);

int udr_context_parse_config(void);

#ifdef __cplusplus
}
#endif

#endif /* UDR_CONTEXT_H */
