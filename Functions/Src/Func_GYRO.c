#include "usart.h"
#include "dma.h"
#include "Func_GYRO.h"
#include "Task_SystInit.h"



GYRO_Struct OfficialGYRO;
GYRO_Struct PersonalGYRO={.num=1},PersonalGYRO2={.num=2};
uint8_t GYROBuffer[2][PersonalGYRO_rx_len],GYROBuffer2[2][PersonalGYRO_rx_len];
uint8_t GYRO_Rx_Mem=0;
uint8_t GYRO_Rx_Mem2=0;

static float CharsToFloat(uint8_t* s);

/**
  * @brief  �������������ݽ���
  * @param  void
  * @retval void
  */
extern EventGroupHandle_t EG_Wait;
void Personal_GYRO_Receive(GYRO_Struct *gyro)
{
	//5A A5 2E 00 71 D4 90 00 B0 F6 FF FB FF 0D 00 C0 06 00 0D FF F8 FE D9 48 D8 3E 3F C5 64 3F 3F 6F 01 08 43 D1 B4 18 C1 3E 26 61 67 BB 39 70 0B 3C 58 F8 6E 3F
	static uint8_t i = 1;
	//ִֻ��һ��Ϩ���Ʊ���ͨѶ����
	while(i)
	{
		HAL_GPIO_WritePin(Red_Led_GPIO_Port,Red_Led_Pin,GPIO_PIN_SET);
		--i;
	}
	if(gyro->num==1)
	{
		gyro->PitchAngle = -((int16_t)(GYROBuffer[GYRO_Rx_Mem][7] | GYROBuffer[GYRO_Rx_Mem][8] << 8)) / 100.0f; 
		gyro->RollAngle =  ((int16_t)(GYROBuffer[GYRO_Rx_Mem][9] | GYROBuffer[GYRO_Rx_Mem][10] << 8)) / 100.0f;
		gyro->YawAngle = - ((int16_t)(GYROBuffer[GYRO_Rx_Mem][11] | GYROBuffer[GYRO_Rx_Mem][12] << 8)) / 10.0f;
		gyro->Gyro_X = CharsToFloat(&GYROBuffer[GYRO_Rx_Mem][26]);
		gyro->Gyro_Y = CharsToFloat(&GYROBuffer[GYRO_Rx_Mem][30]);
		gyro->Gyro_Z = CharsToFloat(&GYROBuffer[GYRO_Rx_Mem][34]);
	}
	if(gyro->num==2)
	{
		gyro->PitchAngle = -((int16_t)(GYROBuffer2[GYRO_Rx_Mem2][7] | GYROBuffer2[GYRO_Rx_Mem2][8] << 8)) / 100.0f; 
		gyro->RollAngle =  ((int16_t)(GYROBuffer2[GYRO_Rx_Mem2][9] | GYROBuffer2[GYRO_Rx_Mem2][10] << 8)) / 100.0f;
		gyro->YawAngle = - ((int16_t)(GYROBuffer2[GYRO_Rx_Mem2][11] | GYROBuffer2[GYRO_Rx_Mem2][12] << 8)) / 10.0f;
		gyro->Gyro_X = CharsToFloat(&GYROBuffer2[GYRO_Rx_Mem2][26]);
		gyro->Gyro_Y = CharsToFloat(&GYROBuffer2[GYRO_Rx_Mem2][30]);
		gyro->Gyro_Z = CharsToFloat(&GYROBuffer2[GYRO_Rx_Mem2][34]);
	}
	

	gyro->FrameCounter++;
}

/**
  * @brief  ����6˫������������
  * @param  void
  * @retval void
  * @note   �ڴ����жϺ����е���
  */
uint8_t usart_this_time_rx_len;
void Uart_Config_In_IRQHandle(UART_HandleTypeDef *huart)
{
	BaseType_t xHigherPriorityTaskToWaken = pdFALSE;
	usart_this_time_rx_len = 0; //�˴ν��ճ���
	DMA_HandleTypeDef *hdma_uart_rx = huart->hdmarx;
	if (__HAL_UART_GET_IT_SOURCE(huart, UART_IT_IDLE) != RESET)
	{
		//clear the idle pending flag
		(void)huart->Instance->SR;
		(void)huart->Instance->DR;

		__HAL_UART_CLEAR_IDLEFLAG(huart);
		__HAL_DMA_DISABLE(hdma_uart_rx);
		usart_this_time_rx_len = PersonalGYRO_rx_len - __HAL_DMA_GET_COUNTER(hdma_uart_rx);

		if ((hdma_uart_rx->Instance->CR & DMA_SxCR_CT) != RESET)
		{
			/* Current memory buffer used is Memory 1 */
			hdma_uart_rx->Instance->CR &= (uint32_t)(~DMA_SxCR_CT);
			GYRO_Rx_Mem = MEMORY1;
		}
		else
		{
			/* Current memory buffer used is Memory 0 */
			hdma_uart_rx->Instance->CR |= (uint32_t)DMA_SxCR_CT;
			GYRO_Rx_Mem = MEMORY0;
		}
		if(usart_this_time_rx_len == PersonalGYROFrameLength\
			&& GYROBuffer[GYRO_Rx_Mem][0] == GYROFRAMEHEAD0 \
			&& GYROBuffer[GYRO_Rx_Mem][1] == GYROFRAMEHEAD1) 
			//������Ϣ֪ͨ
			vTaskNotifyGiveFromISR(GYROUpdateTask_Handle, &xHigherPriorityTaskToWaken);
		else 
			GYRO_Rx_Mem = MEMORYRESET;
	
		__HAL_DMA_SET_COUNTER(hdma_uart_rx, PersonalGYRO_rx_len);
		__HAL_DMA_ENABLE(hdma_uart_rx);

		portYIELD_FROM_ISR(xHigherPriorityTaskToWaken);
	}
}


/**
  * @brief  ����2˫������������
  * @param  void
  * @retval void
  * @note   �ڴ����жϺ����е���
  */
uint8_t usart_this_time_rx_len2;
void Uart_Config_In_IRQHandle2(UART_HandleTypeDef *huart)
{
	BaseType_t xHigherPriorityTaskToWaken = pdFALSE;
	usart_this_time_rx_len2 = 0; //�˴ν��ճ���
	DMA_HandleTypeDef *hdma_uart_rx = huart->hdmarx;
	if (__HAL_UART_GET_IT_SOURCE(huart, UART_IT_IDLE) != RESET)
	{
		//clear the idle pending flag
		(void)huart->Instance->SR;
		(void)huart->Instance->DR;
		__HAL_UART_CLEAR_IDLEFLAG(huart);
		__HAL_DMA_DISABLE(hdma_uart_rx);
		usart_this_time_rx_len2 = PersonalGYRO_rx_len - __HAL_DMA_GET_COUNTER(hdma_uart_rx);

		if ((hdma_uart_rx->Instance->CR & DMA_SxCR_CT) != RESET)
		{
			/* Current memory buffer used is Memory 1 */
			hdma_uart_rx->Instance->CR &= (uint32_t)(~DMA_SxCR_CT);
			GYRO_Rx_Mem2 = MEMORY1;
		}
		else
		{
			/* Current memory buffer used is Memory 0 */
			hdma_uart_rx->Instance->CR |= (uint32_t)DMA_SxCR_CT;
			GYRO_Rx_Mem2 = MEMORY0;
		}
		if(usart_this_time_rx_len2 == PersonalGYROFrameLength\
			&& GYROBuffer2[GYRO_Rx_Mem2][0] == GYROFRAMEHEAD0 \
			&& GYROBuffer2[GYRO_Rx_Mem2][1] == GYROFRAMEHEAD1) 
			//������Ϣ֪ͨ
			vTaskNotifyGiveFromISR(PitchGYROTask_Handle, &xHigherPriorityTaskToWaken);
		else 
			GYRO_Rx_Mem2 = MEMORYRESET;
	
		__HAL_DMA_SET_COUNTER(hdma_uart_rx, PersonalGYRO_rx_len);
		__HAL_DMA_ENABLE(hdma_uart_rx);

		portYIELD_FROM_ISR(xHigherPriorityTaskToWaken);
	}
}

/**
 *  @brief  �ֽ�ת����
 *  @param  �ֽ����׵�ַ
 *  @retval float
 *  @note   �˺������ڽ���IMU����������
 */
float CharsToFloat(uint8_t* s)
{
	union{
		uint8_t c[4];
		float   f;
	} temp;
	temp.c[0] = s[0];
	temp.c[1] = s[1];
	temp.c[2] = s[2];
	temp.c[3] = s[3];

	return temp.f;
}

/**
  * @brief  �ٷ������ǳ�ʼ�� --CAN2
  * @param  void
  * @retval void
  * @note   ��λʱ����ȷ�������Ǿ�ֹ����
  * @note   δʹ�÷��Ͷ�������Ϊ�˺��������ڴ�������֮ǰ
  */
void Official_GYRO_Init(void)
{
//	CAN_TxHeaderTypeDef TX_Message;
//	
//	TX_Message.StdId = OFFICIALZGYRORESETCANID;
//	TX_Message.IDE = CAN_ID_STD;
//	TX_Message.RTR = CAN_RTR_DATA;
//	TX_Message.DLC = 0x08;

//	TX_Message.Data[0] = 0x00;
//	TX_Message.Data[1] = 0x01;
//	TX_Message.Data[2] = 0x02;
//	TX_Message.Data[3] = 0x03;
//	TX_Message.Data[4] = 0x04;
//	TX_Message.Data[5] = 0x05;
//	TX_Message.Data[6] = 0x06;
//	TX_Message.Data[7] = 0x07;

//	hcan2.pTxMsg = &TX_Message;
//	HAL_CAN_Transmit_IT(&hcan2);
}

void PersonalGYRO_Recevie_Enable(void)
{																																																																																																				\
					DMA_MultiBuffer_Transfer_Config(&huart6, (uint32_t)&GYROBuffer[0][0], (uint32_t)&GYROBuffer[1][0], PersonalGYRO_rx_len); 
				__HAL_UART_ENABLE_IT(&huart6, UART_IT_IDLE); 
					DMA_MultiBuffer_Transfer_Config(&huart2, (uint32_t)&GYROBuffer2[0][0], (uint32_t)&GYROBuffer2[1][0], PersonalGYRO_rx_len);
        __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);                                                                            
    
}
