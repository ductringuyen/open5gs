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

#include "event.h"
#include "context.h"

#define EVENT_POOL 32 /* FIXME : 32 */
static OGS_POOL(pool, udr_event_t);

void udr_event_init(void)
{
    ogs_pool_init(&pool, EVENT_POOL);

    udr_self()->queue = ogs_queue_create(EVENT_POOL);
    ogs_assert(udr_self()->queue);
    udr_self()->timer_mgr = ogs_timer_mgr_create();
    ogs_assert(udr_self()->timer_mgr);
    udr_self()->pollset = ogs_pollset_create();
    ogs_assert(udr_self()->pollset);
}

void udr_event_final(void)
{
    if (udr_self()->pollset)
        ogs_pollset_destroy(udr_self()->pollset);
    if (udr_self()->timer_mgr)
        ogs_timer_mgr_destroy(udr_self()->timer_mgr);
    if (udr_self()->queue)
        ogs_queue_destroy(udr_self()->queue);

    ogs_pool_final(&pool);
}

udr_event_t *udr_event_new(udr_event_e id)
{
    udr_event_t *e = NULL;

    ogs_pool_alloc(&pool, &e);
    ogs_assert(e);
    memset(e, 0, sizeof(*e));

    e->id = id;

    return e;
}

void udr_event_free(udr_event_t *e)
{
    ogs_assert(e);
    ogs_pool_free(&pool, e);
}

const char *udr_event_get_name(udr_event_t *e)
{
    if (e == NULL)
        return OGS_FSM_NAME_INIT_SIG;

    switch (e->id) {
    case OGS_FSM_ENTRY_SIG: 
        return OGS_FSM_NAME_ENTRY_SIG;
    case OGS_FSM_EXIT_SIG: 
        return OGS_FSM_NAME_EXIT_SIG;

    case UDR_EVT_SBI_SERVER:
        return "UDR_EVT_SBI_SERVER";
    case UDR_EVT_SBI_CLIENT:
        return "UDR_EVT_SBI_CLIENT";
    case UDR_EVT_SBI_TIMER:
        return "UDR_EVT_SBI_TIMER";

    default: 
       break;
    }

    return "UNKNOWN_EVENT";
}
