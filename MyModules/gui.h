#ifndef __GUI_H
#define __GUI_H

#include "stdlib.h"
#include "stm32f769i_discovery_lcd.h"

#define GUI_TRACK_RECT_HEIGHT  50
#define GUI_TRACK_RECT_WIDTH 350
#define GUI_TRACK_RECT_SPACE 25
#define GUI_TRACK_FONT Font24

#define GUI_CONTROLS_RECT_WIDTH 64
#define GUI_CONTROLS_RECT_HEIGHT 64

#define GUI_VIEW_NUM_CHARS 20
#define GUI_VIEW_TRACKS	5

#define GUI_PROGRESS_WIDTH 675
#define GUI_PROGRESS_HEIGHT 100
#define GUI_PROGRESS_X 50
#define GUI_PROGRESS_Y 300
#define GUI_PROGRESS_PADDING 10

typedef enum {
	BTN_TRACK = 0, BTN_TRACK_CONTROL, BTN_VIEW_CONTROL
} BTN_TypeTypeDef;

typedef enum {
	BTN_NOT_PRESSED = 0, BTN_PRESSED
} BTN_StateTypeDef;

typedef struct {
	int x;
	int y;
	int width;
	int height;
	int border;
	sFONT font;
	BTN_TypeTypeDef type;
	BTN_StateTypeDef state;
	uint32_t backColor;
	uint32_t frontColor;
	unsigned char *icon;

} GUI_ButtonTypeDef;

typedef struct {
	int index;
	int displayIndex;
	GUI_ButtonTypeDef button;
	char fname[255];
	char displayBuff[GUI_VIEW_NUM_CHARS];
	int displayBuffPos;
	int titleLen;
} GUI_TrackTypeDef;

typedef struct {
	int volume;
	GUI_ButtonTypeDef v_plus;
	GUI_ButtonTypeDef v_minus;
	GUI_ButtonTypeDef play;
	GUI_ButtonTypeDef next;
	GUI_ButtonTypeDef prev;
} GUI_TrackControlTypeDef;

typedef struct {
	int pos;
	GUI_ButtonTypeDef up;
	GUI_ButtonTypeDef down;
} GUI_ViewControlTypeDef;

typedef struct {
	int max;
	int curr;
	int bitrate;
	BTN_StateTypeDef state;
} GUI_ProgressBarTypedef;

int GUI_button_clicked(int x, int y, GUI_ButtonTypeDef *btn);
int GUI_seek(int x, int y, GUI_ProgressBarTypedef *progressBar);
void GUI_render_button(GUI_ButtonTypeDef *btn, char *text);
void GUI_render_tracks(GUI_TrackTypeDef *tracks, int numberOfTracks);
void GUI_view_move_up(GUI_TrackTypeDef *tracks, int numberOfTracks);
void GUI_view_move_down(GUI_TrackTypeDef *tracks, int numberOfTracks);
void GUI_render_progress_bar(GUI_ProgressBarTypedef *proressBar);
#endif
