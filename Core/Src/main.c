/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bitmaps.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

SAI_HandleTypeDef hsai_BlockA1;
DMA_HandleTypeDef hdma_sai1_a;

SD_HandleTypeDef hsd2;
DMA_HandleTypeDef hdma_sdmmc2_rx;
DMA_HandleTypeDef hdma_sdmmc2_tx;

/* Definitions for defaultTask */

/* USER CODE BEGIN PV */
osThreadId_t wavTaskHandle;
const osThreadAttr_t wavTask_attributes = { .name = "wavTask", .stack_size = 512
		* 4, .priority = (osPriority_t) osPriorityHigh, };

osThreadId_t guiTaskHandle;
const osThreadAttr_t guiTask_attributes = { .name = "guiTask", .stack_size = 512
		* 4, .priority = (osPriority_t) osPriorityNormal, };

osThreadId_t touchScreenTaskHandle;
const osThreadAttr_t touchScreenTask_attributes = { .name = "touchScreenTask",
		.stack_size = 512 * 4, .priority = (osPriority_t) osPriorityNormal, };

osThreadId_t animateTitlesTaskHandle;
const osThreadAttr_t animateTitlesTask_attributes = { .name =
		"animateTitlesTask", .stack_size = 512 * 4, .priority =
		(osPriority_t) osPriorityLow, };

uint8_t lcd_status = LCD_OK;
uint32_t ts_status = TS_OK;
TS_StateTypeDef TS_State = { 0 };

int numberOfTracks;
GUI_TrackTypeDef *tracks;
GUI_TrackControlTypeDef trackControl;
GUI_ViewControlTypeDef viewControl;
GUI_ProgressBarTypedef progressBar;
Screen *screen;

osMutexId_t wav_mutex;
const osMutexAttr_t wavMutex_attributes = { .name = "wavMutex" };
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_SDMMC2_SD_Init(void);
/* USER CODE BEGIN PFP */
void StartGuiTask(void *args);
void StartTouchScreenTask(void *args);
void StartAnimateTitlesTask(void *args);
void StartWavTask(void *args);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_SDMMC2_SD_Init();
	MX_FATFS_Init();
	/* USER CODE BEGIN 2 */
	screen = Screen_init();

	ts_status = BSP_TS_Init(screen->width, screen->height);
	while (ts_status != TS_OK)
		;

	ts_status = BSP_TS_ITConfig();
	while (ts_status != TS_OK)
		;

	trackControl.volume = WAV_DEFAULT_VOLUME;

	trackControl.v_minus.x = 500;
	trackControl.v_minus.y = 50;
	trackControl.v_minus.height = GUI_CONTROLS_RECT_HEIGHT;
	trackControl.v_minus.width = GUI_CONTROLS_RECT_WIDTH;
	trackControl.v_minus.type = BTN_TRACK_CONTROL;
	trackControl.v_minus.icon = &v_down_icon[0];

	trackControl.v_plus.x = 650;
	trackControl.v_plus.y = 50;
	trackControl.v_plus.height = GUI_CONTROLS_RECT_HEIGHT;
	trackControl.v_plus.width = GUI_CONTROLS_RECT_WIDTH;
	trackControl.v_plus.type = BTN_TRACK_CONTROL;
	trackControl.v_plus.icon = &v_up_icon[0];

	trackControl.prev.x = 500;
	trackControl.prev.y = 150;
	trackControl.prev.height = GUI_CONTROLS_RECT_HEIGHT;
	trackControl.prev.width = GUI_CONTROLS_RECT_WIDTH;
	trackControl.prev.type = BTN_TRACK_CONTROL;
	trackControl.prev.icon = &prev_icon[0];

	trackControl.play.x = 575;
	trackControl.play.y = 150;
	trackControl.play.height = GUI_CONTROLS_RECT_HEIGHT;
	trackControl.play.width = GUI_CONTROLS_RECT_WIDTH;
	trackControl.play.type = BTN_TRACK_CONTROL;
	trackControl.play.icon = &play_icon[0];

	trackControl.next.x = 650;
	trackControl.next.y = 150;
	trackControl.next.height = GUI_CONTROLS_RECT_HEIGHT;
	trackControl.next.width = GUI_CONTROLS_RECT_WIDTH;
	trackControl.next.type = BTN_TRACK_CONTROL;
	trackControl.next.icon = &next_icon[0];

	viewControl.up.x = 400;
	viewControl.up.y = 50;
	viewControl.up.height = GUI_CONTROLS_RECT_HEIGHT;
	viewControl.up.width = GUI_CONTROLS_RECT_WIDTH;
	viewControl.up.type = BTN_VIEW_CONTROL;
	viewControl.up.icon = &arrow_up_icon[0];

	viewControl.down.x = 400;
	viewControl.down.y = 150;
	viewControl.down.height = GUI_CONTROLS_RECT_HEIGHT;
	viewControl.down.width = GUI_CONTROLS_RECT_WIDTH;
	viewControl.down.type = BTN_VIEW_CONTROL;
	viewControl.down.icon = &arrow_down_icon[0];

	WAV_init();

	wav_mutex = osMutexNew(&wavMutex_attributes);

	/* USER CODE END 2 */

	/* Init scheduler */
	osKernelInitialize();

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of defaultTask */
	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	wavTaskHandle = osThreadNew(StartWavTask, NULL, &wavTask_attributes);

	guiTaskHandle = osThreadNew(StartGuiTask, NULL, &guiTask_attributes);

	touchScreenTaskHandle = osThreadNew(StartTouchScreenTask, NULL,
			&touchScreenTask_attributes);

	animateTitlesTaskHandle = osThreadNew(StartAnimateTitlesTask, NULL,
			&animateTitlesTask_attributes);
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */

	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */
	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 432;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 9;
	RCC_OscInitStruct.PLL.PLLR = 2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Activate the Over-Drive mode
	 */
	if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief SDMMC2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SDMMC2_SD_Init(void) {

	/* USER CODE BEGIN SDMMC2_Init 0 */

	/* USER CODE END SDMMC2_Init 0 */

	/* USER CODE BEGIN SDMMC2_Init 1 */

	/* USER CODE END SDMMC2_Init 1 */
	hsd2.Instance = SDMMC2;
	hsd2.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
	hsd2.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
	hsd2.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
	hsd2.Init.BusWide = SDMMC_BUS_WIDE_1B;
	hsd2.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
	hsd2.Init.ClockDiv = 0;
	/* USER CODE BEGIN SDMMC2_Init 2 */

	/* USER CODE END SDMMC2_Init 2 */

}

/* USER CODE BEGIN 4 */

/*
 * Render GUI
 */
void StartGuiTask(void *argument) {
	tracks = WAV_get_files(tracks, &numberOfTracks);

	for (;;) {
		Screen_flip_buffers(screen);
		BSP_LCD_Clear(LCD_COLOR_WHITE);

		GUI_render_tracks(tracks, numberOfTracks);
		GUI_render_button(&trackControl.v_minus, "");
		GUI_render_button(&trackControl.v_plus, "");
		GUI_render_button(&trackControl.play, "");
		GUI_render_button(&trackControl.prev, "");
		GUI_render_button(&trackControl.next, "");
		GUI_render_button(&viewControl.up, "");
		GUI_render_button(&viewControl.down, "");

		GUI_render_progress_bar(&progressBar);

		BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		char volBuff[3];
		sprintf(volBuff, "%d", trackControl.volume);
		BSP_LCD_DisplayStringAt(
				BSP_LCD_GetXSize() - trackControl.v_minus.x
						- trackControl.v_minus.width * 2,
				trackControl.v_minus.y + 16, (uint8_t*) volBuff, RIGHT_MODE);
		osDelay(10);
	}
}

/*
 * Handle touch screen clicks
 */
void StartTouchScreenTask(void *argument) {
	for (;;) {
		BSP_TS_GetState(&TS_State);
		if (TS_State.touchDetected > 0) {
			int x = TS_State.touchX[0];
			int y = TS_State.touchY[0];

			// Track clicked
			for (int i = 0; i < numberOfTracks; i++) {
				if (GUI_button_clicked(x, y, &tracks[i].button)
						== 0&& tracks[i].displayIndex < GUI_VIEW_TRACKS) {
					if (osMutexWait(wav_mutex, 10) == osOK) {
						WAV_select_file(i, tracks, numberOfTracks,
								&progressBar);
						WAV_play(&trackControl);
						osMutexRelease(wav_mutex);
					}
					break;
				}
			}

			// Volume down
			if (GUI_button_clicked(x, y, &trackControl.v_minus) == 0) {
				if (trackControl.volume > 0
						&& trackControl.v_minus.state == BTN_NOT_PRESSED) {
					trackControl.volume -= 5;
					if (WAV_is_playing() == 0) {
						//WAV_pause();
						BSP_AUDIO_OUT_SetVolume((uint8_t) trackControl.volume);
						//WAV_resume();
					} else {
						BSP_AUDIO_OUT_SetVolume((uint8_t) trackControl.volume);
					}
					trackControl.v_minus.state = BTN_PRESSED;
				}
			}

			// Volume up
			if (GUI_button_clicked(x, y, &trackControl.v_plus) == 0) {
				if (trackControl.volume < 100
						&& trackControl.v_plus.state == BTN_NOT_PRESSED) {

					trackControl.volume += 5;
					if (WAV_is_playing() == 0) {
						//WAV_pause();
						BSP_AUDIO_OUT_SetVolume((uint8_t) trackControl.volume);
						//WAV_resume();
					} else {
						BSP_AUDIO_OUT_SetVolume((uint8_t) trackControl.volume);
					}
					trackControl.v_plus.state = BTN_PRESSED;
				}
			}

			// Play/pause
			if (GUI_button_clicked(x, y, &trackControl.play) == 0) {
				if (WAV_is_playing() == 0
						&& trackControl.play.state == BTN_NOT_PRESSED) {
					WAV_pause();
					trackControl.play.state = BTN_PRESSED;
				} else if (WAV_is_playing() == 1
						&& trackControl.play.state == BTN_NOT_PRESSED) {
					WAV_resume();
					trackControl.play.state = BTN_PRESSED;
				}
			}

			// Next
			if (GUI_button_clicked(x, y, &trackControl.next) == 0) {
				if (trackControl.next.state == BTN_NOT_PRESSED) {
					if (osMutexWait(wav_mutex, 10) == osOK) {
						WAV_next(tracks, &trackControl, numberOfTracks,
								&progressBar);
						trackControl.next.state = BTN_PRESSED;
						osMutexRelease(wav_mutex);
					}
				}
			}

			// Prev
			if (GUI_button_clicked(x, y, &trackControl.prev) == 0) {
				if (trackControl.prev.state == BTN_NOT_PRESSED) {
					if (osMutexWait(wav_mutex, 10) == osOK) {
						WAV_prev(tracks, &trackControl, numberOfTracks,
								&progressBar);
						trackControl.prev.state = BTN_PRESSED;
						osMutexRelease(wav_mutex);
					}
				}
			}

			// View Up
			if (GUI_button_clicked(x, y, &viewControl.up) == 0) {
				if (viewControl.up.state == BTN_NOT_PRESSED) {
					GUI_view_move_up(tracks, numberOfTracks);
					viewControl.up.state = BTN_PRESSED;
				}
			}

			// View down
			if (GUI_button_clicked(x, y, &viewControl.down) == 0) {
				if (viewControl.down.state == BTN_NOT_PRESSED) {
					GUI_view_move_down(tracks, numberOfTracks);
					viewControl.down.state = BTN_PRESSED;
				}
			}

			// Seek to
			int seekTo = GUI_seek(x, y, &progressBar);
			if (seekTo > 0 && progressBar.state == BTN_NOT_PRESSED) {
				WAV_seek(seekTo);
				progressBar.state = BTN_PRESSED;
			}

		} else {
			// Set all buttons to not pressed
			progressBar.state = BTN_NOT_PRESSED;
			viewControl.down.state = BTN_NOT_PRESSED;
			viewControl.up.state = BTN_NOT_PRESSED;
			trackControl.next.state = BTN_NOT_PRESSED;
			trackControl.prev.state = BTN_NOT_PRESSED;
			trackControl.play.state = BTN_NOT_PRESSED;
			trackControl.v_minus.state = BTN_NOT_PRESSED;
			trackControl.v_plus.state = BTN_NOT_PRESSED;
		}
		osDelay(5);
	}
}

/*
 * Animate scrolling titles
 */
void StartAnimateTitlesTask(void *args) {
	for (;;) {
		for (int i = 0; i < numberOfTracks; i++) {
			if (tracks[i].displayBuffPos
					< tracks[i].titleLen + GUI_VIEW_NUM_CHARS) {

				memset(tracks[i].displayBuff, 0, sizeof(tracks[i].displayBuff));

				strncpy(tracks[i].displayBuff,
						(char*) &tracks[i].fname[tracks[i].displayBuffPos],
						sizeof(tracks[i].displayBuff) - 1);

				tracks[i].displayBuffPos = (tracks[i].displayBuffPos + 1)
						% GUI_VIEW_NUM_CHARS;
			}
		}
		osDelay(500);
	}
}

/*
 * Process the WAV file
 */
void StartWavTask(void *args) {
	for (;;) {
		if (osMutexWait(wav_mutex, 1) == osOK) {
			WAV_process(tracks, &trackControl, numberOfTracks, &progressBar);
			osMutexRelease(wav_mutex);
		}
		osDelay(2);
	}
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/* USER CODE END Header_StartDefaultTask */

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM6 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM6) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

