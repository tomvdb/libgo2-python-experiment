#include <go2/input.h>
#include <go2/display.h>
#include <drm/drm_fourcc.h>

#include <stdio.h>
#include <string.h>

#include "go2wrapper.h"
#include "font_petme128_8x8.h"

// libgo2 stuff
go2_gamepad_state_t outGamepadState;
go2_battery_state_t batteryState;
go2_display_t* display;
go2_surface_t* surface;
go2_presenter_t* presenter;
go2_input_t* input;

uint32_t color_format;
int height;
int width;
int bytes_per_pixel;

void initDevice() {
	input = go2_input_create();

	display = go2_display_create();
	presenter = go2_presenter_create(display, DRM_FORMAT_RGB565, 0xff00ff00); // ABGR
	height = go2_display_height_get(display);
	width = go2_display_width_get(display);
    surface = go2_surface_create(display, width, height, DRM_FORMAT_RGB565);

    bytes_per_pixel = go2_drm_format_get_bpp(go2_surface_format_get(surface)) / 8;

    go2_display_backlight_set(display, (uint32_t)50);
}

void destroyDevice() {
	go2_input_destroy(input);
	go2_presenter_destroy(presenter);
	go2_display_destroy(display);
}

__uint16_t readButtons()
{
	__uint16_t buttonValue = 0;

	go2_input_gamepad_read(input,&outGamepadState);	

	buttonValue ^= (-outGamepadState.buttons.a ^ buttonValue) & (1UL << 0);
	buttonValue ^= (-outGamepadState.buttons.b ^ buttonValue) & (1UL << 1);
	buttonValue ^= (-outGamepadState.buttons.x ^ buttonValue) & (1UL << 2);
	buttonValue ^= (-outGamepadState.buttons.y ^ buttonValue) & (1UL << 3);

	return buttonValue;
}

uint32_t readBatteryLevel()
{
	go2_input_battery_read(input, &batteryState);

	return batteryState.level;
}

void fillColor(__uint16_t col)
{
	for ( int x = 0; x <  width-1; x++)
		for ( int y = 0; y  < height-1; y++)
		{
			drawPixel(y,x, col);
		}
}

void drawText(int x0, int y0, char *str, __uint16_t col)
{
	// https://github.com/micropython/micropython/blob/master/extmod/modframebuf.c
	for (; *str; ++str) {
        // get char and make sure its in range of font
        int chr = *(uint8_t *)str;

		if (chr < 32 || chr > 127) {
            chr = 127;
        }
        // get char data
        const uint8_t *chr_data = &font_petme128_8x8[(chr - 32) * 8];
        // loop over char data
        for (int j = 0; j < 8; j++, x0++) {
            if (0 <= x0 && x0 < height) { // clip x
                uint8_t vline_data = chr_data[j]; // each byte is a column of 8 pixels, LSB at top
                for (int y = y0; vline_data; vline_data >>= 1, y++) { // scan over vertical column
                    if (vline_data & 1) { // only draw if pixel set
                        if (0 <= y && y < width) { // clip y
                            drawPixel( x0, y, col);
                        }
                    }
                }
            }
        }
	  }
	}


void drawLine(int x1, int y1, int x2, int y2, __uint16_t col)
{

    int dx = x2 - x1;
    int sx;
    if (dx > 0) {
        sx = 1;
    } else {
        dx = -dx;
        sx = -1;
    }

    int dy = y2 - y1;
    int sy;
    if (dy > 0) {
        sy = 1;
    } else {
        dy = -dy;
        sy = -1;
    }

    _Bool steep;
    if (dy > dx) {
        int temp;
        temp = x1;
        x1 = y1;
        y1 = temp;
        temp = dx;
        dx = dy;
        dy = temp;
        temp = sx;
        sx = sy;
        sy = temp;
        steep = 1;
    } else {
        steep = 0;
    }

    int e = 2 * dy - dx;
    for (int i = 0; i < dx; ++i) {
        if (steep) {
            if (0 <= y1 && y1 < height && 0 <= x1 && x1 < width) {
                drawPixel( y1, x1, col);
            }
        } else {
            if (0 <= x1 && x1 < height && 0 <= y1 && y1 < width) {
                drawPixel(x1, y1, col);
            }
        }
        while (e >= 0) {
            y1 += sy;
            e -= 2 * dx;
        }
        x1 += sx;
        e += 2 * dy;
    }

    if (0 <= x2 && x2 < height && 0 <= y2 && y2 < width) {
        drawPixel(x2, y2, col);
    }	
}

void drawPixel(int x, int y, __uint16_t c) {
	// https://github.com/kruffin/go2radio/blob/master/main.cpp
	uint8_t* dst = (uint8_t*)go2_surface_map(surface);
	
	// swap the x and y while translating x
	// →[*][ ][ ][ ]
	//  [ ][ ][ ][ ]
	// to:
	//  [ ][*]
	//  [ ][ ]
	//  [ ][ ]
	//  [ ][ ]
	//      ↑

	int yfinal = height - 1 - x;
	int xfinal = y;
	memcpy(dst + (yfinal * go2_surface_stride_get(surface) + xfinal*bytes_per_pixel), (unsigned char*)&c, sizeof(c));
}

void flipScreen()
{
	go2_presenter_post(presenter, surface, 
						0, 0, width, height,
						0, 0, width, height,
						GO2_ROTATION_DEGREES_0);
}


