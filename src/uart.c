//******************************************************************************
//  Секция include: здесь подключается заголовочный файл к модулю
//******************************************************************************
#include "uart.h"

#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#endif

//******************************************************************************
//  Секция определения переменных, используемых в модуле
//******************************************************************************

//------------------------------------------------------------------------------
// Глобальные
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Локальные
//------------------------------------------------------------------------------

UART_HandleTypeDef huart1;

#ifdef USE_FREERTOS
xSemaphoreHandle xRxSemaphore;
xSemaphoreHandle xTxSemaphore;
#endif

sRingBuf_t sTxRingBuf, sRxRingBuf;

//******************************************************************************
//  Секция прототипов локальных функций
//******************************************************************************

void prv_SendChar(uint8_t data);

//******************************************************************************
//  Секция описания функций (сначала глобальных, потом локальных)
//******************************************************************************

/*******************************************************************************
*  Function:       UART_Init
*-------------------------------------------------------------------------------
*  description:    Конфигурирует модуль UART
*  parameters:     void
*  on return:      void
*******************************************************************************/
void UART_Init(void)
{
	huart1.Instance          = USARTx;
	huart1.Init.BaudRate     = UART_BOUD_RATE;
	huart1.Init.WordLength   = UART_WORDLENGTH_8B;
	huart1.Init.StopBits     = UART_STOPBITS_1;
	huart1.Init.Parity       = UART_PARITY_NONE;
	huart1.Init.Mode         = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&huart1);

	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);

#ifdef USE_FREERTOS
	xRxSemaphore = xSemaphoreCreateCounting(10,0);
	xTxSemaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(xTxSemaphore);
#endif
}

/*******************************************************************************
*  Function:       HAL_UART_MspInit
*-------------------------------------------------------------------------------
*  description:
*  parameters:     void
*  on return:      void
*******************************************************************************/
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(huart->Instance==USART1)
  {
    /* Peripheral clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  }

}

/*******************************************************************************
*  Function:       HAL_UART_MspDeInit
*-------------------------------------------------------------------------------
*  description:
*  parameters:     void
*  on return:      void
*******************************************************************************/
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{

  if(huart->Instance==USART1)
  {
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 interrupt DeInit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  }

}

/*******************************************************************************
*  Function:       UART_SendChar
*-------------------------------------------------------------------------------
*  description:    Отправляет байт данных
*  parameters:     data - байт данных
*  on return:      void
*******************************************************************************/
void UART_SendChar(uint8_t data)
{
	xSemaphoreTake(xTxSemaphore, portMAX_DELAY);
	prv_SendChar(data);
	xSemaphoreGive(xTxSemaphore);
}

/*******************************************************************************
*  Function:       UART_SendString
*-------------------------------------------------------------------------------
*  description:    Отправляет строку символов
*  parameters:     str - строка символов
*  on return:      void
*******************************************************************************/
void UART_SendString(const char *str)
{
	xSemaphoreTake(xTxSemaphore, portMAX_DELAY);
	int i = 0;
	while (str[i] != 0)
	{
		prv_SendChar(str[i]);
		i++;
	}
	xSemaphoreGive(xTxSemaphore);
}

/*******************************************************************************
*  Function:       UART_GetChar
*-------------------------------------------------------------------------------
*  description:    Принимает байт данных
*  parameters:     void
*  on return:      int - байт данных, если -1, то нет данных
*******************************************************************************/
int UART_GetChar(void)
{
	uint8_t data;

	if (sRxRingBuf.wrIdx != sRxRingBuf.rdIdx)
	{
		data = sRxRingBuf.data[sRxRingBuf.rdIdx++];
		if (sRxRingBuf.rdIdx >= uartSIZE_OF_RING_BUFFER)
		{
			sRxRingBuf.rdIdx = 0;
		}
		return (int)data;
	}
	else
	{
		return (UART_NO_DATA);
	}
}

/*******************************************************************************
*  Function:       UART_GetCharBlocking
*-------------------------------------------------------------------------------
*  description:
*  parameters:     void
*  on return:      void
*******************************************************************************/
#ifdef USE_FREERTOS
int UART_GetCharBlocking(void)
{
	xSemaphoreTake(xRxSemaphore, portMAX_DELAY);
	return UART_GetChar();
}
#endif

/*******************************************************************************
*  Function:       prv_SendChar
*-------------------------------------------------------------------------------
*  description:    Отправка байта
*  parameters:     void
*  on return:      void
*******************************************************************************/
void prv_SendChar(uint8_t data)
{
	sTxRingBuf.data[sTxRingBuf.wrIdx++] = data;
	if (sTxRingBuf.wrIdx >= uartSIZE_OF_RING_BUFFER)
	{
		sTxRingBuf.wrIdx = 0;
	}
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_TXE);
}

/*******************************************************************************
*  Function:       USARTx_IRQHandler
*-------------------------------------------------------------------------------
*  description:    Прерывания модуля UART
*  parameters:     void
*  on return:      void
*******************************************************************************/
void USARTx_IRQHandler(void)
{
	if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET)
	{
		sRxRingBuf.data[sRxRingBuf.wrIdx++] = (uint8_t)(huart1.Instance->DR & 0x00FF); // принимаем байт
		if (sRxRingBuf.wrIdx >= uartSIZE_OF_RING_BUFFER)
		{
			sRxRingBuf.wrIdx = 0;
		}
		__HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_RXNE);
#ifdef USE_FREERTOS
		portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(xRxSemaphore, &xHigherPriorityTaskWoken);
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD();
		}
#endif
	}
	else if (__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_TXE) != RESET)
	{
		if (sTxRingBuf.wrIdx != sTxRingBuf.rdIdx)
		{
			huart1.Instance->DR = (uint8_t)sTxRingBuf.data[sTxRingBuf.rdIdx++]; // передаём байт

			if (sTxRingBuf.rdIdx >= uartSIZE_OF_RING_BUFFER)
			{
				sTxRingBuf.rdIdx = 0;
			}

			if (sTxRingBuf.wrIdx == sTxRingBuf.rdIdx)
			{
	            __HAL_UART_DISABLE_IT(&huart1, UART_IT_TXE);
//	            __HAL_UART_ENABLE_IT(&huart1, UART_IT_TC);
			}
		}
	}

//	HAL_UART_IRQHandler(&huart1);
}

//******************************************************************************
//  ENF OF FILE
//******************************************************************************
