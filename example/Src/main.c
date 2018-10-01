/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/

#include "conf.h"
#include "microrl_func.h"

//******************************************************************************
//  Секция определения переменных, используемых в модуле
//******************************************************************************

//------------------------------------------------------------------------------
// Глобальные
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Локальные
//------------------------------------------------------------------------------

volatile uint32_t periodBlink;
volatile uint32_t sysTickUptime = 0;

//******************************************************************************
//  Секция прототипов локальных функций
//******************************************************************************

void SystemClock_Config(void);
void Error_Handler(void);
static void MX_GPIO_Init(void);

//******************************************************************************
//  Секция описания функций (сначала глобальных, потом локальных)
//******************************************************************************

void microrl_run(void *pvParameters)
{
	microrl_terminalInit();
	while(1)
	{
		microrl_terminalProcess();
	}
}

void vLedTask (void *pvParameters)
{
    while(1)
    {

    }

    vTaskDelete(NULL);
}

portTASK_FUNCTION_PROTO(initTask, pvParameters)
{
	/* MCU Configuration----------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	UART_Init();

#ifdef DEBUG
	xTaskCreate(	DebugTask,"debug",
					100,
					NULL,
					tskIDLE_PRIORITY + 2,
					NULL);
#endif

	xTaskCreate(	microrl_run,"microrl",
					500,
					NULL,
					tskIDLE_PRIORITY + 3,
					NULL);


	/* Terminate initTask */
	vTaskDelete(NULL);
}

int main(void)
{
	//		   Task_func		       Task_name   Stack	    Param  Prio			   Handler
	xTaskCreate(initTask, (const char *) "Init",  128, (void *) NULL, 2, (xTaskHandle *) NULL);

	/* Запуск шедулера, после чего созданные задачи начнут выполняться. */
	vTaskStartScheduler();

	/* Если все хорошо, то управление в main() никогда не дойдет до этой точки,
	     и теперь шедулер будет управлять задачами. Если main() довела управление
	     до этого места, то это может означать, что не хватает памяти кучи (heap)
	     для создания специальной задачи ожидания (idle task, об этой задаче
	     далее). Часть 5 предоставляет больше информации по управлению
	     памятью. */
	for( ;; );

	return 0;
}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13|GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 PC14 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB6 PB7 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  static uint32_t delay = 0;
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
	  for(delay = 0; delay < 1000000; delay++)
		  continue;

	  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
  }
}

/*************************************************************
*  Function:       micros
*------------------------------------------------------------
*  description:    Считывает время безотказной работы системы в
*                  микросекундах (70 минут)
*  parameters:     void
*  on return:      uint32_t - системное время в микросекундах
*************************************************************/
uint32_t micros(void)
{
	register uint32_t ms, cycle_cnt;
    do
    {
        ms = sysTickUptime;
        cycle_cnt = SysTick->VAL;
    } while (ms != sysTickUptime);

    return ((float)ms * 1000.0) + (72000.0 - (float)cycle_cnt) / 72.0;
}

/*******************************************************************************
 Секция обработчиков системных событий:
  ошибка MallocFailed - не удалось выделить память, скорее всего закончилась свободная память;
  ошибка StackOverflow - переполнение стека вызовов. Бесконечная рекурсия или выделение слишком большой переменной;
  событие Idle - вызывается каждый тик ядра RTOS.

Ошибки "обрабатываются" падением в бесконечный цикл. Очевидно, это лучше чем пытаться продолжать
программу с неверными данными. На самом деле, лучше вызывать стандартный обработчик HARD_FAULT.

Событие Idle обрабатывается пустой процедурой. В этом примере нам не нужно дополнительно что-то делать каждый тик.
*******************************************************************************/
//void vApplicationMallocFailedHook( void )
//{
//	HAL_GPIO_TogglePin(LED_ERR_GPIO_Port, LED_ERR_Pin);
//	for( ;; );
//}
//
//void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
//{
//	HAL_GPIO_TogglePin(LED_ERR_GPIO_Port, LED_ERR_Pin);
//	for( ;; );
//}
//
//void vApplicationIdleHook( void )
//{
//	HAL_GPIO_TogglePin(LED_ERR_GPIO_Port, LED_ERR_Pin);
//	for( ;; );
//}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
