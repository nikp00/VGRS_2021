#include "gui.h"
#include "fatfs.h"

int GUI_button_clicked(int x, int y, GUI_ButtonTypeDef *btn) {
	if (x >= btn->x && x <= btn->x + btn->width && y >= btn->y
			&& y <= btn->y + btn->height)
		return 0;
	return 1;
}

void GUI_render_button(GUI_ButtonTypeDef *btn, char *text) {

	if (btn->type == BTN_TRACK) {
		BSP_LCD_SetFont(&btn->font);
		BSP_LCD_SetTextColor(btn->backColor);
		BSP_LCD_SetBackColor(btn->backColor);

		BSP_LCD_FillRect(btn->x, btn->y, btn->width, btn->height);

		BSP_LCD_SetBackColor(btn->backColor);
		BSP_LCD_SetTextColor(btn->frontColor);

		BSP_LCD_DrawRect(btn->x, btn->y, btn->width, btn->height);

		BSP_LCD_DisplayStringAt(BSP_LCD_GetXSize() - btn->x,
				btn->y + btn->height / 2 - btn->font.Height / 2,
				(uint8_t*) text, LEFT_MODE);

	} else if (btn->type == BTN_TRACK_CONTROL
			|| btn->type == BTN_VIEW_CONTROL) {

		BSP_LCD_DrawBitmap(btn->x, btn->y, (uint8_t*) btn->icon); //&v_up_icon[0]); //&btn->icon);
	}

}

void GUI_render_progress_bar(GUI_ProgressBarTypedef *progressBar) {
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_SetBackColor(LCD_COLOR_BLACK);

	BSP_LCD_DrawRect(GUI_PROGRESS_X, GUI_PROGRESS_Y, GUI_PROGRESS_WIDTH,
	GUI_PROGRESS_HEIGHT);

	if (progressBar->max > 0) {

		char buff[5];
		int sec = progressBar->curr / progressBar->bitrate;
		float step = ((GUI_PROGRESS_WIDTH - 2 * GUI_PROGRESS_PADDING)
				/ (float) progressBar->max);
		int width = progressBar->curr * step;

		sprintf(buff, "%02d:%02d", sec / 60, sec % 60);

		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		BSP_LCD_SetBackColor(LCD_COLOR_WHITE);

		BSP_LCD_DisplayStringAt(width + 25, GUI_PROGRESS_Y - 25, buff,
				LEFT_MODE);

		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		BSP_LCD_SetBackColor(LCD_COLOR_BLACK);

		if (width > 0)
			BSP_LCD_FillRect(GUI_PROGRESS_X + GUI_PROGRESS_PADDING,
			GUI_PROGRESS_Y + GUI_PROGRESS_PADDING, width,
			GUI_PROGRESS_HEIGHT - 2 * GUI_PROGRESS_PADDING);
	}
}

void GUI_render_tracks(GUI_TrackTypeDef *tracks, int numberOfTracks) {
	BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	for (int i = 0; i < numberOfTracks; i++) {
		if (tracks[i].displayIndex
				>= 0&& tracks[i].displayIndex < GUI_VIEW_TRACKS) {
			GUI_render_button(&tracks[i].button, (char*) tracks[i].displayBuff);

		}
	}
}

void GUI_view_move_up(GUI_TrackTypeDef *tracks, int numberOfTracks) {

	if (tracks[GUI_VIEW_TRACKS].displayIndex < GUI_VIEW_TRACKS) {
		for (int i = 0; i < numberOfTracks; i++) {
			tracks[i].displayIndex++;
			tracks[i].button.y = 2 * tracks[i].displayIndex
					* GUI_TRACK_RECT_SPACE;
		}
	}
}

void GUI_view_move_down(GUI_TrackTypeDef *tracks, int numberOfTracks) {
	if (tracks[numberOfTracks - 1].displayIndex >= GUI_VIEW_TRACKS) {
		for (int i = 0; i < numberOfTracks; i++) {
			tracks[i].displayIndex--;
			tracks[i].button.y = 2 * tracks[i].displayIndex
					* GUI_TRACK_RECT_SPACE;
		}
	}
}

int GUI_seek(int x, int y, GUI_ProgressBarTypedef *progressBar) {
	if (x >= GUI_PROGRESS_X + GUI_PROGRESS_PADDING / 2
			&& x
					<= GUI_PROGRESS_X + GUI_PROGRESS_WIDTH
							- GUI_PROGRESS_PADDING / 2
			&& y >= GUI_PROGRESS_Y + GUI_PROGRESS_PADDING / 2
			&& y
					<= GUI_PROGRESS_Y + GUI_PROGRESS_HEIGHT
							- GUI_PROGRESS_PADDING / 2) {

		float step = ((float) progressBar->max
				/ (GUI_PROGRESS_WIDTH - 2 * GUI_PROGRESS_PADDING));
		return step * (x - GUI_PROGRESS_X - GUI_PROGRESS_PADDING / 2);
	} else {
		return -1;
	}
}
