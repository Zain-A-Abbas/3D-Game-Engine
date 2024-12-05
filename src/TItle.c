#include "Title.h"
#include "gf2d_font.h"
#include "gf2d_draw.h"
#include "gf3d_vgraphics.h"

TitleData titleData = {0};

void createTitle() {
    titleData.logoActor = gf2d_actor_load(
        "actors/logo.actor"
    );
}

void processTitle() {
    if (gfc_input_command_pressed("menu_up") || gfc_input_command_pressed("menu_down")) {
        if (titleData.selectedOption == 0) {
            titleData.selectedOption = 1;
        } else {
            titleData.selectedOption = 0;
        }
    }
    drawTitle();
}

void drawTitle() {
    GFC_Vector2D resolution = gf3d_vgraphics_get_resolution();
    // Background
    GFC_Rect background = {0};
    background.x = 0; background.y = 0;
    background.w = resolution.x; background.h = resolution.y;
    gf2d_draw_rect_filled(
        background,
        gfc_color(0.025, 0.0125, 0.1, 1.0)
    );

    // Logo
    gf2d_actor_draw(
        titleData.logoActor,
        0,
        gfc_vector2d(resolution.x / 2, resolution.y * 0.3),
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );

    // Options

    gf2d_font_draw_line_tag(
        "Start Game",
        FT_Large,
        gfc_color(0.8, 0.8, 0.8, 1.0),
        gfc_vector2d(120, 400)
    );
    gf2d_font_draw_line_tag(
        "Exit",
        FT_Large,
        gfc_color(0.8, 0.8, 0.8, 1.0),
        gfc_vector2d(120, 500)
    );
    
    // Draw highlight
    GFC_Rect highlight = {0};
    highlight.x = 120;
    highlight.w = 200;
    highlight.h = 2;
    highlight.y = 440 + 100 * titleData.selectedOption;
    gf2d_draw_rect_filled(
        highlight,
        gfc_color(1, 1, 1, 1)
    );
}