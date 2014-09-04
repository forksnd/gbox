/* //////////////////////////////////////////////////////////////////////////////////////
 * trace
 */
#define TB_TRACE_MODULE_NAME            "demo"
#define TB_TRACE_MODULE_DEBUG           (0)

/* //////////////////////////////////////////////////////////////////////////////////////
 * includes
 */ 
#include "demo.h"
#include "rect.h"

/* //////////////////////////////////////////////////////////////////////////////////////
 * types
 */

// the demo entry type
typedef struct __gb_demo_entry_t
{
    // the clos func
    tb_void_t           (*clos)(gb_window_ref_t window);

    // the draw func
    tb_void_t           (*draw)(gb_window_ref_t window, gb_canvas_ref_t canvas);
     
    // the event func
    tb_void_t           (*event)(gb_window_ref_t window, gb_event_ref_t event);

}gb_demo_entry_t;

/* //////////////////////////////////////////////////////////////////////////////////////
 * globals
 */

// the demo index
static tb_size_t        g_index = 0;

// the demo entries
static gb_demo_entry_t  g_entries[] =
{
    {gb_demo_rect_clos, gb_demo_rect_draw, gb_demo_rect_event}
};

// the matrix
static gb_matrix_t      g_matrix;

// the quality
static tb_size_t        g_quality = GB_QUALITY_LOW;

/* //////////////////////////////////////////////////////////////////////////////////////
 * private implementation
 */
static tb_void_t gb_demo_info(tb_bool_t killed, tb_cpointer_t priv)
{
    // check
    gb_window_ref_t window = (gb_window_ref_t)priv;
    tb_assert_and_check_return(window);

    // trace
    tb_trace_i("framerate: %{float}", gb_window_framerate(window));
}

/* //////////////////////////////////////////////////////////////////////////////////////
 * implementation
 */
tb_void_t gb_demo_clos(gb_window_ref_t window, tb_cpointer_t priv)
{
    // check
    tb_assert_abort(window);

    // trace
    tb_trace_d("clos");

    // clos entries
    tb_size_t index = 0;
    tb_size_t count = tb_arrayn(g_entries);
    for (index = 0; index < count; index++)
    {
        // done clos
        if (g_entries[index].clos) g_entries[index].clos(window);
    }
}
tb_void_t gb_demo_draw(gb_window_ref_t window, gb_canvas_ref_t canvas, tb_cpointer_t priv)
{
    // check
    tb_assert_abort(window && canvas);
    tb_assert_abort(g_index < tb_arrayn(g_entries));

    // clear it
    gb_canvas_draw_clear(canvas, GB_COLOR_DEFAULT);

    // the entry
    gb_demo_entry_t const* entry = &g_entries[g_index];
    tb_assert_abort(entry->draw);

    // enter matrix
    gb_matrix_copy(gb_canvas_save_matrix(canvas), &g_matrix);

    // done draw
    entry->draw(window, canvas);

    // leave matrix
    gb_canvas_load_matrix(canvas);
}
tb_void_t gb_demo_resize(gb_window_ref_t window, gb_canvas_ref_t canvas, tb_cpointer_t priv)
{
    // check
    tb_assert_abort(window && canvas);

    // trace
    tb_trace_d("resize: %lux%lu", gb_window_width(window), gb_window_height(window));

    // the x0 and y0
    gb_float_t x0 = gb_long_to_float(gb_window_width(window) >> 1);
    gb_float_t y0 = gb_long_to_float(gb_window_height(window) >> 1);

	// update matrix
	gb_matrix_init_translate(&g_matrix, x0, y0);	
}
tb_void_t gb_demo_event(gb_window_ref_t window, gb_event_ref_t event, tb_cpointer_t priv)
{
    // check
    tb_assert_abort(window && event);
    tb_assert_abort(g_index < tb_arrayn(g_entries));

    // keyboard
    if (event->type == GB_EVENT_TYPE_KEYBOARD && event->u.keyboard.pressed)
    {
        switch (event->u.keyboard.code)
        {
        case GB_KEY_BACK:
            gb_window_fullscreen(window, tb_false);
            break;
        case GB_KEY_LEFT:
            if (g_index) g_index--;
            break;
        case GB_KEY_RIGHT:
            if (g_index < tb_arrayn(g_entries)) g_index++;
            break;
        case 'f':
            gb_window_fullscreen(window, tb_true);
            break;
        case 'q':
            g_quality++;
            if (g_quality > GB_QUALITY_TOP) g_quality = GB_QUALITY_LOW;
            gb_quality_set(g_quality);
            break;
        case 'i':
            tb_timer_task_post(gb_window_timer(window), 1000, tb_true, gb_demo_info, (tb_cpointer_t)window);
            break;
        default:
            break;
        }
    }
    // mouse
    else if (event->type == GB_EVENT_TYPE_MOUSE)
    {
        // transform?
        gb_float_t  x;
        gb_float_t  y;
        tb_bool_t   transform = tb_false;
        if (event->u.mouse.code == GB_MOUSE_MOVE && event->u.mouse.button == GB_MOUSE_BUTTON_LEFT)
        {
            x = event->u.mouse.cursor.x;
            y = event->u.mouse.cursor.y;
            transform = tb_true;
        }

        // transform matrix
        if (transform)
        {
            // the dw and dh
            gb_float_t dw = gb_long_to_float(gb_window_width(window));
            gb_float_t dh = gb_long_to_float(gb_window_height(window));

            // the x0 and y0
            gb_float_t x0 = gb_rsh(dw, 1);
            gb_float_t y0 = gb_rsh(dh, 1);

            // the dx and dy
            gb_float_t dx = x > x0? (x - x0) : (x0 - x);
            gb_float_t dy = y > y0? (y - y0) : (y0 - y);
            dx = gb_lsh(dx, 1);
            dy = gb_lsh(dy, 1);

            // the an
            gb_float_t an = 0;
            if (y == y0) an = 0;
            else if (x == x0) an = gb_long_to_float(90);
            else an = gb_div(gb_atan(gb_div(dy, dx)) * 180, GB_PI);
            if (y < y0 && x < x0) an = gb_long_to_float(180) - an;
            if (y > y0 && x < x0) an += gb_long_to_float(180);
            if (y > y0 && x > x0) an = gb_long_to_float(360) - an;
            an = -an;

            // scale dx and dy
            dx = gb_lsh(dx, 2);
            dy = gb_lsh(dy, 2);

            // update matrix
            gb_matrix_init_translate(&g_matrix, x0, y0);
            gb_matrix_scale(&g_matrix, gb_div(dx, dw), gb_div(dy, dh));
            gb_matrix_rotate(&g_matrix, an);
        }
    }

#if defined(__gb_debug__) && TB_TRACE_MODULE_DEBUG
    // dump event
    gb_event_dump(event);
#endif

    // the entry
    gb_demo_entry_t const* entry = &g_entries[g_index];
    tb_assert_abort(entry->event);

    // done event
    entry->event(window, event);
}

