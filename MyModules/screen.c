#include <screen.h>
#include <stdlib.h>

/*
 * Used to remove the flickering of the display.
 * Uses to buffers instead of one.
 *
 * Credits https://gist.github.com/postspectacular/61f17333c17b0206a73e4591cd5ce59b
 */

Screen* Screen_init() {
	BSP_LCD_Init();
	Screen *screen = (Screen*) malloc(sizeof(Screen));
	screen->width = BSP_LCD_GetXSize();
	screen->height = BSP_LCD_GetYSize();
	screen->addr[0] = LCD_FB_START_ADDRESS;
	screen->addr[1] = LCD_FB_START_ADDRESS + screen->width * screen->height * 4;
	screen->front = 1;
	BSP_LCD_LayerDefaultInit(0, screen->addr[0]);
	BSP_LCD_LayerDefaultInit(1, screen->addr[1]);
	BSP_LCD_SetLayerVisible(0, DISABLE);
	BSP_LCD_SetLayerVisible(1, ENABLE);
	BSP_LCD_SelectLayer(0);
	return screen;
}

uint32_t Screen_backbuffer_id(Screen *screen) {
	return 1 - screen->front;
}

void Screen_flip_buffers(Screen *screen) {
	// wait for VSYNC
	while (!(LTDC->CDSR & LTDC_CDSR_VSYNCS))
		;
	BSP_LCD_SetLayerVisible(screen->front, DISABLE);
	screen->front ^= 1;
	BSP_LCD_SetLayerVisible(screen->front, ENABLE);
	BSP_LCD_SelectLayer(Screen_backbuffer_id(screen));
}

uint32_t* Screen_backbuffer_ptr(Screen *screen) {
	return (uint32_t*) (screen->addr[Screen_backbuffer_id(screen)]);
}
