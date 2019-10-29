#define __FUNC_CHASSIS_GLOBALS

#include "Func_Chassis.h"
#include "Func_BoardToBoardComm.h"
#include "Func_StatusMachine.h"
#include "Task_Tasks.h"

#ifdef SLAVE_MODE

#define LEFT_SWITCH_STATUS HAL_GPIO_ReadPin(Left_Switch_GPIO_Port, Left_Switch_Pin)
#define MID_SWITCH_STATUS HAL_GPIO_ReadPin(Mid_Switch_GPIO_Port, Mid_Switch_Pin)
#define RIGHT_SWITCH_STATUS HAL_GPIO_ReadPin(Right_Switch_GPIO_Port, Right_Switch_Pin)

//�¼�λ��
#define imu_ob_temp_pos 0
#define imu_ob_angle_pos 1
#define chassis_pos 2

extern EventGroupHandle_t EG_Wait;

//����ʹ��
float i, d = 0;
float p = 0;
int16_t TargetSpeed = 0;

ChassisParam_Struct ChassisParam = {
    .Chassis_Mode = chassis_run,
    .Motor.PID.Kp = 15,   //0.5,
    .Motor.PID.Ki = 0.25, //0.05,
    .Motor.PID.Kd = 2,   //1,
    .Motor.Mechanical_n = 0,
    .Motor.Current_n = 0,
};

static void M3508_PID_Ctrl(ChassisParam_Struct *chassis);

/**
  * @brief  ���̵���������
  * @param  unused
  * @retval void
  */
void Chassis_Ctrl(void)
{
  M3508_PID_Ctrl(&ChassisParam);
}

/**
  * @brief  ���̵���ٶȱջ� 
  * @param  ���̽ṹ��
  * @retval void
  */
void M3508_PID_Ctrl(ChassisParam_Struct *chassis)
{
  /**************  Motor  ***************/
  chassis->Motor.PID.Last_Error = chassis->Motor.PID.Cur_Error;
  chassis->Motor.PID.Cur_Error = chassis->Motor.TargetSpeed - chassis->Motor.RealSpeed;
  chassis->Motor.PID.Sum_Error += chassis->Motor.PID.Cur_Error;
  //��������
  chassis->Motor.PID.Sum_Error = chassis->Motor.PID.Sum_Error > 15000 ? 15000 : chassis->Motor.PID.Sum_Error;
  chassis->Motor.PID.Sum_Error = chassis->Motor.PID.Sum_Error < -15000 ? -15000 : chassis->Motor.PID.Sum_Error;

  chassis->Motor.PID.Output = (int32_t)(chassis->Motor.PID.Kp * chassis->Motor.PID.Cur_Error + chassis->Motor.PID.Ki * chassis->Motor.PID.Sum_Error + chassis->Motor.PID.Kd * (chassis->Motor.PID.Cur_Error - chassis->Motor.PID.Last_Error));
  //�����������
  chassis->Motor.NeedCurrent = (chassis->Motor.PID.Output >= C620CURRENTMAX) ? C620CURRENTMAX : (int16_t)chassis->Motor.PID.Output;
  chassis->Motor.NeedCurrent = (chassis->Motor.PID.Output <= -C620CURRENTMAX) ? -C620CURRENTMAX : (int16_t)chassis->Motor.NeedCurrent;
}

/**
  * @brief  ���̵�����ݷ���  --CAN1
  * @param  ���̵������
  * @retval None
  */
void M3508_CMD_Trans(int16_t Output)
{
  static CanSend_Type CANSend;

  CANSend.CANx = CAN_NUMBER_1;

  CANSend.SendCanTxMsg.StdId = CHASSISSENDCANID;
  CANSend.SendCanTxMsg.IDE = CAN_ID_STD;
  CANSend.SendCanTxMsg.RTR = CAN_RTR_DATA;
  CANSend.SendCanTxMsg.DLC = 0x08;

  CANSend.SendCanTxMsg.Data[0] = (uint8_t)(Output >> 8);
  CANSend.SendCanTxMsg.Data[1] = (uint8_t)Output;
  CANSend.SendCanTxMsg.Data[2] = (uint8_t)0;
  CANSend.SendCanTxMsg.Data[3] = (uint8_t)0;
  CANSend.SendCanTxMsg.Data[4] = (uint8_t)0;
  CANSend.SendCanTxMsg.Data[5] = (uint8_t)0;
  CANSend.SendCanTxMsg.Data[6] = (uint8_t)0;
  CANSend.SendCanTxMsg.Data[7] = (uint8_t)0;

  xQueueSend(Queue_CANSend, &CANSend, 3 / portTICK_RATE_MS);
}

/**
  * @brief  �жϵ��ת�ӻ�е���Ƿ��0  --CAN1
  * @param  None
  * @retval ���������1�����������-1����������0
  */
int8_t If_Pass_Zero(void)
{
  if (ChassisParam.Motor.RealSpeed > 0) //�����0
  {
    if (ChassisParam.Motor.Last_Mechanical_Angle > ChassisParam.Motor.Mechanical_Angle)
      return 1;
    else
      return 0;
  }
  else if (ChassisParam.Motor.RealSpeed < 0)
  {
    if (ChassisParam.Motor.Last_Mechanical_Angle < ChassisParam.Motor.Mechanical_Angle)
      return -1;
    else
      return 0;
  }
  else
    return 0;
}


/**
  * @brief  ���̵���ʼ��λ��ʱ�Ŀ���
  * @param  None
  * @retval None
  */
void Chassis_Ctrl_Init(void)
{
  ChassisParam.Motor.TargetSpeed = 300; //��λʱ���ٶ�Ϊ300

  if (!LEFT_SWITCH_STATUS)
  {
    xEventGroupSetBits(EG_Wait, 0x01 << chassis_pos);
    ChassisParam.Motor.TargetSpeed = 0;
    ChassisParam.Motor.Current_n = 0;
    ChassisParam.Motor.Mechanical_n = 0;
    LED3_ON();
  }
  Chassis_Ctrl();
  M3508_CMD_Trans(ChassisParam.Motor.NeedCurrent);
}

/**
  * @brief  ���õ����ƶ��ٶ�
  * @param  None
  * @retval None
  */

void Chassis_Speed_Set(void)
{
  if (LEFT_SWITCH_STATUS == 0) //�ж���߽ӽ�����
    {
      if (ChassisParam.Motor.TargetSpeed > 0) //�жϽ��յ���Ŀ���ٶ�
        ChassisParam.Motor.TargetSpeed = 0;
      if (Master_Value.Tar_Speed > 0) //�жϵ�ǰ�ƶ��ٶ�
        ChassisParam.Motor.TargetSpeed = 0;
      else
        ChassisParam.Motor.TargetSpeed = Master_Value.Tar_Speed;
    }
    else if (RIGHT_SWITCH_STATUS == 0) //�ж��ұ߽ӽ�����
    {
      if (ChassisParam.Motor.TargetSpeed < 0) //�жϽ��յ���Ŀ���ٶ�
        ChassisParam.Motor.TargetSpeed = 0;
      if (Master_Value.Tar_Speed < 0) //�жϵ�ǰ�ƶ��ٶ�
        ChassisParam.Motor.TargetSpeed = 0;
      else
        ChassisParam.Motor.TargetSpeed = Master_Value.Tar_Speed;
    }
    else
      ChassisParam.Motor.TargetSpeed = Master_Value.Tar_Speed; //�������ӽ����ؾ�δ��⵽�ϰ���ֱ�Ӹ�ֵ
    
}
#endif
