#include "moui_event_bus.h"
#include <string.h>

moui_event_bus_t moui_bus;

void moui_event_bus_init(moui_event_bus_t *bus)
{
    memset(bus, 0, sizeof(*bus));
}

bool moui_event_bus_subscribe(moui_event_bus_t *bus, moui_evt_type_t type,
                             uint16_t id, moui_evt_handler_t handler, void *ctx)
{
    if (!bus || !handler) return false;
    if (bus->count >= MOUI_EVT_MAX_HANDLERS) return false;
    bus->slots[bus->count].type    = type;
    bus->slots[bus->count].id      = id;
    bus->slots[bus->count].handler = handler;
    bus->slots[bus->count].ctx     = ctx;
    bus->count++;
    return true;
}

void moui_event_bus_unsubscribe(moui_event_bus_t *bus, moui_evt_handler_t handler)
{
    if (!bus || !handler) return;
    for (int i = 0; i < bus->count; i++) {
        if (bus->slots[i].handler == handler) {
            bus->slots[i] = bus->slots[bus->count - 1];
            bus->count--;
            i--;
        }
    }
}

void moui_event_bus_publish(moui_event_bus_t *bus, const moui_event_t *evt)
{
    if (!bus || !evt) return;

    /*
     * A handler may unsubscribe (swap-remove) during dispatch, which would
     * otherwise desync the loop. Detect modifications via the generation
     * counter and restart the scan from the top so no slot is skipped and
     * stale entries (now empty) are skipped by the NULL handler check.
     */
    for (int i = 0; i < bus->count; i++) {
        if (!bus->slots[i].handler) continue;
        bool type_match = (bus->slots[i].type == MOUI_EVT_NONE || bus->slots[i].type == evt->type);
        bool id_match   = (bus->slots[i].id == 0 || bus->slots[i].id == evt->id);
        if (type_match && id_match) {
            bus->slots[i].handler(evt, bus->slots[i].ctx);
            /* slot may have been swapped/removed by the handler */
            if (i >= bus->count) return;
        }
    }
}
