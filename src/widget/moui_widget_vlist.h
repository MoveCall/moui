#ifndef MOUI_WIDGET_VLIST_H
#define MOUI_WIDGET_VLIST_H

#include "moui_widget.h"
#include "moui_widget_label.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct moui_vlist moui_vlist_t;

/**
 * Data provider callbacks for Virtualized List.
 */
typedef uint32_t (*moui_vlist_count_fn)(const moui_vlist_t *vl, void *user_data);
typedef int16_t  (*moui_vlist_height_fn)(const moui_vlist_t *vl, uint32_t index, void *user_data);
typedef void     (*moui_vlist_bind_fn)(const moui_vlist_t *vl, moui_widget_t *cell, uint32_t index, void *user_data);
typedef moui_widget_t* (*moui_vlist_create_cell_fn)(const moui_vlist_t *vl, void *user_data);

#define MOUI_VLIST_MAX_POOL_SIZE 24

struct moui_vlist {
    moui_widget_t           base;
    int32_t                 scroll_offset;    /* Vertical scroll position in px */
    int32_t                 item_height;      /* Uniform item height (if height_fn is NULL) */
    uint32_t                selected_index;   /* Currently focused / selected item index */
    
    moui_vlist_count_fn     count_fn;
    moui_vlist_height_fn    height_fn;
    moui_vlist_bind_fn      bind_fn;
    moui_vlist_create_cell_fn create_cell_fn;
    void                   *user_data;
    
    moui_widget_t          *cell_pool[MOUI_VLIST_MAX_POOL_SIZE];
    uint8_t                 pool_size;        /* Number of active cell instances in pool */
    
    /* Event Callbacks */
    void (*on_select)(moui_vlist_t *vl, uint32_t index);
};

/**
 * Initialize virtualized list widget.
 * @param vl Virtualized list widget pointer
 * @param item_height Default height for each cell item in px (e.g. 24)
 * @param count_fn Callback returning total data item count (e.g. 100,000)
 * @param bind_fn Callback updating cell widget with item data at specified index
 * @param user_data Opaque pointer passed to callbacks
 */
void moui_vlist_init(moui_vlist_t *vl,
                    int16_t item_height,
                    moui_vlist_count_fn count_fn,
                    moui_vlist_bind_fn bind_fn,
                    void *user_data);

/**
 * Register a pre-instantiated cell widget into the recycling pool.
 */
bool moui_vlist_add_cell(moui_vlist_t *vl, moui_widget_t *cell);

/**
 * Scroll virtualized list by delta pixels.
 */
void moui_vlist_scroll(moui_vlist_t *vl, int32_t delta);

/**
 * Scroll to ensure the specified item index is visible.
 */
void moui_vlist_scroll_to_index(moui_vlist_t *vl, uint32_t index);

/**
 * Set selected item index.
 */
void moui_vlist_set_selected(moui_vlist_t *vl, uint32_t index);

/**
 * Get total height of all virtual items combined.
 */
int32_t moui_vlist_get_total_height(const moui_vlist_t *vl);

#ifdef __cplusplus
}
#endif

#endif /* MOUI_WIDGET_VLIST_H */
