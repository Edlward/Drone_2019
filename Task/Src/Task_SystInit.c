
#define __TASK_SYSTINIT_GLOBALS

#include "Task_SystInit.h"
#include "Task_Tasks.h"
#include "Task_Test.h"
#include "Func_JudgeReceive.h"
#include "Func_StatusMachine.h"
#include "Func_BoardToBoardComm.h"
#include "Func_Imu_OB.h"
#include "Func_GYRO.h"
#include "Func_FricMotor.h"
#include "Func_JetsonComm.h"
#include "Func_RC.h"
/**
  * @brief  ϵͳ��ʼ������
  * @param  unused
  * @retval void
  * @note   ���ö�ջ70+
  */
void Task_SystemInitConfig(void *parameters)
{
    taskENTER_CRITICAL();
    Queue_CANSend = xQueueCreate(30, sizeof(CanSend_Type));
    //�¼��齨��
    //In 
    //Mode:  bit0 IMUOB�¶�  bit1 IMUOB�Ƕ�  bit2 ����λ��;
    //In Master Mode: bit0 IMUOB�¶�  bit1 IMUOB�Ƕ�  bit2 IMU����  bit3 JestonComm  bit4 BTBComm
    //EG_Wait = xEventGroupCreate();
    //xEventGroupClearBits(EG_Wait,0x1f);
    //CAN��ʼ��
    MX_CAN_Filter_Init(&hcan1);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
    //IMU��ʼ��
    mpu_device_init();
    init_quaternion();
		RC_Receive_Enable(&huart1);
		Official_GYRO_Init();
    JetsonCommUart_Config(&huart7);
    PersonalGYRO_Recevie_Enable();
		JudgeConnection_Init(&huart8);
		FricMotor_Init();
    xTaskCreate(Task_CANSend, "Task_CANSend", 200, NULL, 6, &CANSendTask_Handle);                     //CAN��������
    xTaskCreate(Task_Control_Normal, "Task_Control_Normal", 400, NULL, 4, &NormalControlTask_Handle); //��ͨ��������
    xTaskCreate(Task_StatusMachineUpdate, "SM_Update", 300, NULL, 5, &StatusMachineTask_Handle);      //״̬������
    xTaskCreate(Task_LEDBlink, "Task_LEDBlink", 100, NULL, 4, &LEDTask_Handle);                       //LED��˸����
    //xTaskCreate(Task_Imu_Ob,"Task_Imu_Ob",300,NULL,4,&ImuObTask_Handle);
    //xTaskCreate(Task_InitLED,"Task_InitLED",200,NULL,6,&InitLEDTask_Handle);     
    xTaskCreate(Task_JudgeReceive, "Task_JudgeReceive", 300, NULL, 6, &JudgeReceiveTask_Handle);
    xTaskCreate(Task_RCData_Update, "Task_RCData_Update", 100, NULL, 5, &RCUpdateTask_Handle);       //ң�������ݸ���
    xTaskCreate(Task_JetsonComm, "Task_JetsonComm", 300, NULL, 5, &JetsonCommTask_Handle);           //��JetsonͨѶ����
    xTaskCreate(Task_GYROData_Update, "Task_GYROData_Update", 150, NULL, 5, &GYROUpdateTask_Handle); //Yaw�����������ݸ���
		xTaskCreate(Task_Pitch_GYRO, "Task_Pitch_GYRO", 150, NULL, 5, &PitchGYROTask_Handle); //Pitch�����������ݸ���
    HAL_Delay(1000);

    //��������

    //xTaskCreate(Task_Test1,"Task_Test1",400,NULL,4,&Test1Task_Handle);     //ң�������ݸ���
    //    xTaskCreate(Task_Test2,"Task_Test2",400,NULL,4,&Test2Task_Handle);     //ң�������ݸ���
    
    vTaskDelete(NULL);
    taskEXIT_CRITICAL();
}
