#include "wav_player.h"
#include "fatfs.h"

typedef enum {
	AUDIO_FILE_NOT_AVAILABLE = 0,
	AUDIO_FILE_AVAILABLE,
	AUDIO_FILE_PLAYING,
	AUDIO_FILE_DONE,
	AUDIO_FILE_PAUSE
} WAV_FILE_StateTypeDef;

typedef enum {
	AUDIO_BUFFER_IDLE = 0,
	AUDIO_BUFFER_TRANSFER_COMPLETE_HALF,
	AUDIO_BUFFER_TRANSFER_COMPLETE_FULL
} WAV_BUFFER_StateTypeDef;

typedef struct {
	uint32_t ChunkID; /* 0 */
	uint32_t FileSize; /* 4 */
	uint32_t FileFormat; /* 8 */
	uint32_t SubChunk1ID; /* 12 */
	uint32_t SubChunk1Size; /* 16*/
	uint16_t AudioFormat; /* 20 */
	uint16_t NbrChannels; /* 22 */
	uint32_t SampleRate; /* 24 */

	uint32_t ByteRate; /* 28 */
	uint16_t BlockAlign; /* 32 */
	uint16_t BitPerSample; /* 34 */
	uint32_t SubChunk2ID; /* 36 */
	uint32_t SubChunk2Size; /* 40 */

} WAV_HeaderTypeDef;

typedef struct {
	uint8_t buff[WAV_BUFFER_SIZE];
	FSIZE_t fileSize;
	FIL file;
	FILINFO fileInfo;
	UINT bytesRead;
	UINT bytesRemaining;
	WAV_HeaderTypeDef header;
	WAV_FILE_StateTypeDef state;
	WAV_BUFFER_StateTypeDef buffState;
	GUI_TrackTypeDef *track;
} WAV_FileTypeDef;

extern char SDPath[4]; /* SD logical drive path */
extern FATFS SDFatFS; /* File system object for SD logical drive */
FRESULT res; /* FatFs function common result code */
DIR dir;
FILINFO fno;
char path[] = "/";
static WAV_FileTypeDef audioFile;

/*
 * Load the mount the file selected on the GUI
 */
void WAV_select_file(int index, GUI_TrackTypeDef *tracks, int numberOfTracks,
		GUI_ProgressBarTypedef *progressBar) {

	if (audioFile.state == AUDIO_FILE_PLAYING) {
		BSP_AUDIO_OUT_Pause();
	}

	audioFile.state = AUDIO_FILE_NOT_AVAILABLE;
	audioFile.buffState = AUDIO_BUFFER_IDLE;

	for (int i = 0; i < numberOfTracks; i++) {
		tracks[i].button.backColor = LCD_COLOR_WHITE;
		tracks[i].button.frontColor = LCD_COLOR_BLACK;
	}

	// Try to close the previously opened file
	f_close(&audioFile.file);
	if (f_open(&audioFile.file, tracks[index].fname, FA_READ) == FR_OK) {
		audioFile.fileSize = f_size(&audioFile.file);
		f_stat((TCHAR*) path, &audioFile.fileInfo);
		f_read(&audioFile.file, &audioFile.header, sizeof(WAV_HeaderTypeDef),
				&audioFile.bytesRead);
		audioFile.bytesRemaining = audioFile.fileSize - audioFile.bytesRead;
		audioFile.state = AUDIO_FILE_AVAILABLE;

		tracks[index].button.frontColor = LCD_COLOR_BLACK;
		tracks[index].button.backColor = LCD_COLOR_GREEN;
		audioFile.track = &tracks[index];

		progressBar->max = (int) audioFile.fileSize;
		progressBar->bitrate = audioFile.header.ByteRate;
		progressBar->curr = 0;
	}

}

// Init the SAI (BSP_AUDIO_OUT)
void WAV_init() {
	BSP_AUDIO_OUT_DeInit();
	res = BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, WAV_DEFAULT_VOLUME,
	BSP_AUDIO_FREQUENCY_48K);
	BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
	BSP_AUDIO_OUT_Play((uint16_t*) &audioFile.buff, WAV_BUFFER_SIZE);
	BSP_AUDIO_OUT_Pause();
}

// Start playing the selected file
void WAV_play(GUI_TrackControlTypeDef *volumeControl) {
	f_lseek(&audioFile.file, 0);
	f_read(&audioFile.file, &audioFile.buff[0], WAV_BUFFER_SIZE,
			&audioFile.bytesRead);
	audioFile.bytesRemaining = audioFile.fileSize - audioFile.bytesRead;
	BSP_AUDIO_OUT_Resume();
	audioFile.state = AUDIO_FILE_PLAYING;
}

// Process the playing file
void WAV_process(GUI_TrackTypeDef *tracks, GUI_TrackControlTypeDef *controls,
		int numberOfTracks, GUI_ProgressBarTypedef *progressBar) {

	if (audioFile.state == AUDIO_FILE_PLAYING) {
		if (audioFile.bytesRemaining > 0) {
			// If the DMA finished the transfer of the first half of the buffer, add new data to the first half
			if (audioFile.buffState == AUDIO_BUFFER_TRANSFER_COMPLETE_HALF) {
				f_read(&audioFile.file, &audioFile.buff[0],
				WAV_BUFFER_SIZE / 2, &audioFile.bytesRead);
				audioFile.bytesRemaining -= audioFile.bytesRead;
				audioFile.buffState = AUDIO_BUFFER_IDLE;
				progressBar->curr = (int) audioFile.fileSize
						- audioFile.bytesRemaining;
			}
			// if the DMA finished the transfer of the second half of the buffer, add new data to the second half
			else if (audioFile.buffState
					== AUDIO_BUFFER_TRANSFER_COMPLETE_FULL) {
				f_read(&audioFile.file, &audioFile.buff[WAV_BUFFER_SIZE / 2],
				WAV_BUFFER_SIZE / 2, &audioFile.bytesRead);
				audioFile.bytesRemaining -= audioFile.bytesRead;
				audioFile.buffState = AUDIO_BUFFER_IDLE;
				progressBar->curr = (int) audioFile.fileSize
						- audioFile.bytesRemaining;
			}
		} else {
			BSP_AUDIO_OUT_Pause();
			audioFile.state = AUDIO_FILE_DONE;
			audioFile.buffState = AUDIO_BUFFER_IDLE;
			WAV_next(tracks, controls, numberOfTracks, progressBar);
		}
	}
}

/*
 * Get all the WAV file on the SD card (only stats are read, not data)
 */
GUI_TrackTypeDef* WAV_get_files(GUI_TrackTypeDef *tracks, int *numberOfTracks) {
	res = f_mount(&SDFatFS, (const TCHAR*) SDPath, 0);
	res = f_opendir(&dir, path);
	int count = 0;
	while (1) {
		res = f_readdir(&dir, &fno);

		//if (fno.fsize == 0 || fno.fname[0] == '\0')
		if (fno.fname[0] == '\0')
			break;

		if (WAV_is_wav(fno.fname) == 0) {
			count++;
		}
	}

	*numberOfTracks = count;
	tracks = (GUI_TrackTypeDef*) malloc(sizeof(GUI_TrackTypeDef) * count);

	res = f_opendir(&dir, path);
	count = 0;
	while (count < *numberOfTracks) {
		res = f_readdir(&dir, &fno);
		if (WAV_is_wav((char*) &fno.fname) == 0) {
			strncpy((char*) &tracks[count].fname, (char*) &fno.fname, 255);
			tracks[count].button.x = 0;
			tracks[count].button.y = 2 * count * GUI_TRACK_RECT_SPACE;
			tracks[count].button.width = GUI_TRACK_RECT_WIDTH;
			tracks[count].button.height = GUI_TRACK_RECT_HEIGHT;
			tracks[count].button.font = GUI_TRACK_FONT;
			tracks[count].button.border = 1;
			tracks[count].button.type = BTN_TRACK;
			tracks[count].button.backColor = LCD_COLOR_WHITE;
			tracks[count].button.frontColor = LCD_COLOR_BLACK;
			tracks[count].button.frontColor = LCD_COLOR_BLACK;
			tracks[count].index = count;
			tracks[count].displayIndex = count;
			int len = 0;
			while (tracks[count].fname[len++] != '\0') {
			}

			tracks[count].titleLen = len;
			tracks[count].displayBuffPos = 0;

			count++;
		}
	}

	return tracks;
}

/*
 * Check if a file is .WAV
 */
int WAV_is_wav(char *file) {
	int len = 0;
	int i = 0;
	int dot = 0;
	char wav[] = ".wav";

	while (file[i] != '\0') {
		if (file[i] == '.')
			dot = i;
		i++;
	}
	len = i;
	i = 0;

	char *ftype = (char*) malloc((len - dot + 1) * sizeof(char));
	while (dot < len) {
		ftype[i++] = file[dot++];
	}
	ftype[i] = 0;
	int res = strcmp(ftype, wav);
	free(ftype);
	return res;
}

int WAV_is_playing() {
	return audioFile.state == AUDIO_FILE_PLAYING ? 0 : 1;
}

void WAV_pause() {
	audioFile.state = AUDIO_FILE_PAUSE;
	audioFile.buffState = AUDIO_BUFFER_IDLE;
	BSP_AUDIO_OUT_Pause();
	audioFile.track->button.backColor = LCD_COLOR_YELLOW;
}

void WAV_resume() {
	audioFile.state = AUDIO_FILE_PLAYING;
	audioFile.buffState = AUDIO_BUFFER_IDLE;
	BSP_AUDIO_OUT_Resume();
	audioFile.track->button.backColor = LCD_COLOR_GREEN;
}

void WAV_next(GUI_TrackTypeDef *tracks, GUI_TrackControlTypeDef *controls,
		int numberOfTracks, GUI_ProgressBarTypedef *progressBar) {
	int index = audioFile.track->index;
	if (index < numberOfTracks - 1)
		index++;
	WAV_select_file(index, tracks, numberOfTracks, progressBar);
	WAV_play(controls);
}

void WAV_prev(GUI_TrackTypeDef *tracks, GUI_TrackControlTypeDef *controls,
		int numberOfTracks, GUI_ProgressBarTypedef *progressBar) {
	int index = audioFile.track->index;
	if (index > 0)
		index--;
	WAV_select_file(index, tracks, numberOfTracks, progressBar);
	WAV_play(controls);
}

void WAV_seek(int seekTo) {
	if (audioFile.state != AUDIO_FILE_NOT_AVAILABLE) {

		f_lseek(&audioFile.file, (seekTo / WAV_BUFFER_SIZE) * WAV_BUFFER_SIZE);
		audioFile.bytesRemaining = audioFile.fileSize
				- (seekTo / WAV_BUFFER_SIZE) * WAV_BUFFER_SIZE;
	}
}

void BSP_AUDIO_OUT_TransferComplete_CallBack(void) {
	audioFile.buffState = AUDIO_BUFFER_TRANSFER_COMPLETE_FULL;
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void) {
	audioFile.buffState = AUDIO_BUFFER_TRANSFER_COMPLETE_HALF;
}

