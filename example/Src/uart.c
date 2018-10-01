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

void UART_SendChar(uint8_t data)
{
	xSemaphoreTake(xTxSemaphore, portMAX_DELAY);
	prv_SendChar(data);
	xSemaphoreGive(xTxSemaphore);
}

void UART_SendString(const char *str)
{
	xSemaphoreTake(xTxSemaphore, portMAX_DELAY);
//	int i = 0;
//	while (str[i] != 0)
//	{
//		prv_SendChar(str[i]);
//		i++;
//	}
	HAL_UART_Transmit_IT(&huart1, (uint8_t *) str, strlen(str));
	vTaskDelay(2);
	xSemaphoreGive(xTxSemaphore);
}

int UART_GetChar()
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
	else	{
		return (UART_NO_DATA);
	}
}

#ifdef USE_FREERTOS
int UART_GetCharBlocking()
{
	xSemaphoreTake(xRxSemaphore, portMAX_DELAY);
	return UART_GetChar();
}
#endif

void prv_SendChar(uint8_t data)
{
//	sTxRingBuf.data[sTxRingBuf.wrIdx++] = data;
//	if (sTxRingBuf.wrIdx >= uartSIZE_OF_RING_BUFFER)
//
//		sTxRingBuf.wrIdx = 0;
//	}
//	USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
//	HAL_UART_Transmit(&huart1, &data, 1, 500);
}

void USARTx_IRQHandler(void)
{
	uint8_t x;

	HAL_UART_IRQHandler(&huart1);

	if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE))
	{
		HAL_UART_Receive(&huart1, &x, 1, 500); // получаем символ
		sRxRingBuf.data[sRxRingBuf.wrIdx++] = x;
		if (sRxRingBuf.wrIdx >= uartSIZE_OF_RING_BUFFER)
		{
			sRxRingBuf.wrIdx = 0;
		}
#ifdef USE_FREERTOS
		portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(xRxSemaphore, &xHigherPriorityTaskWoken);
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portYIELD();
		}
#endif
	}
//	else if (USART_GetITStatus(USARTx,USART_IT_TXE) != RESET)
//	{
//		if (sTxRingBuf.wrIdx != sTxRingBuf.rdIdx)
//		{
//			USART_SendData(USARTx, sTxRingBuf.data[sTxRingBuf.rdIdx++]);
//			if (sTxRingBuf.rdIdx >= uartSIZE_OF_RING_BUFFER)
//			{
//				sTxRingBuf.rdIdx = 0;
//			}
//			if (sTxRingBuf.wrIdx == sTxRingBuf.rdIdx)
//			{
//				USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);
//			}
//		}
//	}
}

//******************************************************************************
//  ENF OF FILE
//******************************************************************************
