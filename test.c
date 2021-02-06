#include <go2/input.h>
#include <go2/display.h>
#include <drm/drm_fourcc.h>

#include <stdio.h>
#include <string.h>

// libgo2 stuff
go2_gamepad_state_t outGamepadState;
go2_display_t* display;
go2_surface_t* surface;
go2_presenter_t* presenter;
go2_input_t* input;

uint32_t color_format;
int height;
int width;
int bytes_per_pixel;

void initGo2() {
	input = go2_input_create();

	display = go2_display_create();
	presenter = go2_presenter_create(display, DRM_FORMAT_RGB565, 0xff00ff00); // ABGR
	height = go2_display_height_get(display);
	width = go2_display_width_get(display);
    surface = go2_surface_create(display, width, height, DRM_FORMAT_RGB565);

    bytes_per_pixel = go2_drm_format_get_bpp(go2_surface_format_get(surface)) / 8;

    go2_display_backlight_set(display, (uint32_t)50);
}

void destroyGo2() {
	go2_input_destroy(input);
	go2_presenter_destroy(presenter);
	go2_display_destroy(display);
}


void go2_present() {
	go2_presenter_post(presenter, surface, 
						0, 0, width, height,
						0, 0, width, height,
						GO2_ROTATION_DEGREES_0);
	// std::cout << "drawing buffer" << std::endl;
}


void go2SetPixel(int x, int y, uint8_t c) {
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
	// std::cout << "drawing pixel og(" << x << "," << y << ") trans(" << xfinal << "," << yfinal << ")" << std::endl;
	memcpy(dst + (yfinal * go2_surface_stride_get(surface) + xfinal*bytes_per_pixel), (unsigned char*)&c, sizeof(c));

	// Take this out since it has the potential to infinite loop if not lucky like I was.
	//dirty_display = true;

	// std::cout << "drawing pixel " << c << std::endl;
}

void drawScreen()
{
    go2_present();
}


/*
int main()
{
    printf("Hello World\n\n");

    initGo2();

    int x = 0;
    int y = 0;


    while (1)
    {
        for ( x = 0; x < 100; x++)
        {
            go2SetPixel(x,10,255);

        }
        drawScreen();
    }

    destroyGo2();
}
*/