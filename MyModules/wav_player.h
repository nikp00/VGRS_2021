#ifndef __WAV_PLAYER_H
#define __WAV_PLAYER_H

#include "stdlib.h"
#include "gui.h"
#include "stm32f769i_discovery_audio.h"

#define WAV_BUFFER_SIZE 2048
#define WAV_DEFAULT_VOLUME    10

void WAV_init();
void WAV_select_file(int index, GUI_TrackTypeDef *tracks, int numberOfTracks,
		GUI_ProgressBarTypedef *progressBar);
void WAV_play(GUI_TrackControlTypeDef *volumeControl);
void WAV_process(GUI_TrackTypeDef *tracks, GUI_TrackControlTypeDef *controls,
		int numberOfTracks, GUI_ProgressBarTypedef *progressBar);
GUI_TrackTypeDef* WAV_get_files();
int WAV_is_wav(char *file);
int WAV_is_playing();
void WAV_pause();
void WAV_resume();
void WAV_next(GUI_TrackTypeDef *tracks, GUI_TrackControlTypeDef *controls,
		int numberOfTracks, GUI_ProgressBarTypedef *progressBar);
void WAV_prev(GUI_TrackTypeDef *tracks, GUI_TrackControlTypeDef *controls,
		int numberOfTracks, GUI_ProgressBarTypedef *progressBar);
void WAV_seek(int seekTo);
#endif
