#include <string.h>
#include <M5GFX.h>

extern "C" {
#include "panel.h"
#include "scene.h"
#include "screen.h"
}

#define LCD_W 320
#define LCD_H 240
#define TARGET_W 320
#define TARGET_H 160

#define LEFT ((LCD_W - TARGET_W) / 2)
#define TOP ((LCD_H - TARGET_H) / 2)
#define MAX_VSCALE ((TARGET_H + SCREEN_H - 1) / SCREEN_H)
#define RGB565(c) ((uint16_t)((((c) >> 8) & 0xf800) | (((c) >> 5) & 0x07e0) | (((c) >> 3) & 0x001f)))

static M5GFX display;
static uint16_t s_rows[TARGET_W * MAX_VSCALE];
static uint8_t s_shown[SCREEN_BYTES];
static bool s_any_shown;

/* SCENE_ON = 0, SCENE_DIM = 1, SCENE_OFF = 2 */
static const uint8_t BRIGHTNESS[] = {240, 30, 0};

extern "C" bool panel_power(uint8_t power)
{
    display.setBrightness(power < 3 ? BRIGHTNESS[power] : 240);
    return true;
}

extern "C" bool panel_init(void)
{
    if (!display.init()) return false;
    display.setRotation(0);
    display.setSwapBytes(true);
    display.fillScreen(RGB565(SCREEN_DARK_RGB));
    return panel_power(SCENE_ON);
}

extern "C" void panel_present(const uint8_t *frame)
{
    if (s_any_shown && !memcmp(frame, s_shown, SCREEN_BYTES)) return;

    const uint16_t lit = RGB565(SCREEN_LIT_RGB);
    const uint16_t dark = RGB565(SCREEN_DARK_RGB);

    display.startWrite();
    for (int sy = 0; sy < SCREEN_H; ++sy) {
        const int y0 = (sy * TARGET_H) / SCREEN_H;
        const int y1 = ((sy + 1) * TARGET_H) / SCREEN_H;
        const int h = y1 - y0;
        if (h <= 0) continue;

        for (int dx = 0; dx < TARGET_W; ++dx) {
            const int sx = (dx * SCREEN_W) / TARGET_W;
            s_rows[dx] = fb_get(frame, sx, sy) ? lit : dark;
        }
        for (int k = 1; k < h; ++k) {
            memcpy(s_rows + k * TARGET_W, s_rows, TARGET_W * sizeof(s_rows[0]));
        }
        display.pushImage(LEFT, TOP + y0, TARGET_W, h, s_rows);
    }
    display.endWrite();

    memcpy(s_shown, frame, SCREEN_BYTES);
    s_any_shown = true;
}