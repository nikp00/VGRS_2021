#ifndef __SCREEN_H
#define __SCREEN_H

#include "stm32f769i_discovery_lcd.h"

typedef struct {
	uint32_t addr[2];
	uint32_t width;
	uint32_t height;
	uint32_t front;
} Screen;

Screen* Screen_init();
void Screen_flip_buffers(Screen *screen);
uint32_t* Screen_backbuffer_ptr(Screen *screen);

uint32_t Screen_backbuffer_id(Screen *screen);

#endif
