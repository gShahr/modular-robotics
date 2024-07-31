#include "GuiLite.h"

#define UI_WIDTH  240
#define UI_HEIGHT 320

static c_surface* s_surface;
static c_display* s_display;

void create_ui(void* phy_fb, int screen_width, int screen_height, int color_bytes, struct EXTERNAL_GFX_OP* gfx_op)
{
    s_surface = new c_surface(UI_WIDTH, UI_HEIGHT, color_bytes, Z_ORDER_LEVEL_0);
    s_surface->set_active(true);
    s_surface->fill_rect(0, 0, UI_WIDTH, UI_HEIGHT, GL_RGB(0, 0, 0), Z_ORDER_LEVEL_0);
    s_display = new c_display(phy_fb, screen_width, screen_height, s_surface);
}

int main(int argc, char* argv[])
{
    // Initialize GuiLite
    create_ui(NULL, UI_WIDTH, UI_HEIGHT, 2, NULL);

    // Create a simple UI element
    c_button button;
    button.set_surface(s_surface);
    button.set_font_type(static_cast<const LATTICE_FONT_INFO*>(c_theme::get_font(FONT_DEFAULT)));
    button.set_str("Run");
    button.set_wnd_pos(50, 50, 140, 60);
    button.show_window();

    // Main loop
    while (1) {
        thread_sleep(100);
    }

    return 0;
}