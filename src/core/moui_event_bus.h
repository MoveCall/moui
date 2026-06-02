#ifndef MOUI_EVENT_BUS_H
#define MOUI_EVENT_BUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "../moui_conf.h"

typedef enum {
    MOUI_EVT_NONE = 0,
    MOUI_EVT_VALUE_CHANGED,
    MOUI_EVT_SCREEN_ENTER,
    MOUI_EVT_SCREEN_LEAVE,
    MOUI_EVT_BUTTON_CLICK,
    MOUI_EVT_LIST_SELECT,
    MOUI_EVT_TIMER,
    MOUI_EVT_CUSTOM,
} moui_evt_type_t;

typedef struct {
    moui_evt_type_t type;
    uint16_t       id;
    int32_t        value;
    void          *sender;
} moui_event_t;

typedef void (*moui_evt_handler_t)(const moui_event_t *evt, void *ctx);

typedef struct {
    struct {
        moui_evt_type_t    type;
        uint16_t          id;
        moui_evt_handler_t handler;
        void             *ctx;
    } slots[MOUI_EVT_MAX_HANDLERS];
    uint8_t count;
} moui_event_bus_t;

void moui_event_bus_init(moui_event_bus_t *bus);

bool moui_event_bus_subscribe(moui_event_bus_t *bus, moui_evt_type_t type,
                             uint16_t id, moui_evt_handler_t handler, void *ctx);

void moui_event_bus_unsubscribe(moui_event_bus_t *bus, moui_evt_handler_t handler);

void moui_event_bus_publish(moui_event_bus_t *bus, const moui_event_t *evt);

extern moui_event_bus_t moui_bus;


#ifdef __cplusplus
}
#endif
#endif
