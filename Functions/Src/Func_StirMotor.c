#define __FUNC_STIRMOTOR_GLOBALS

#include "Func_StirMotor.h"
#include "Func_StatusMachine.h"



/*
��������Ϊ�������Ƕ����ӡ������ٶ�Ϊ��������ʱ�루���ῴ��
�����⣺
  ��תʱ���������++����ת���������--
Ϊ��֤���ݾ���׼ȷ������ر�ʱ����
*/

void Stir_Motor_Speed_Control(ESC_820R* motor);
void StirMotor_Blocked_Detect(ESC_820R* motor);


ESC_820R  RM2006_StirMotor = {
  .PID.Kp = 5.0f,
  .PID.Ki = 0.25f, 
  .PID.Kd = 0.0f,
  .TargetSpeed = 0
};

/**
  * @brief  ���̵������
  * @param  void
  * @retval void
  * @note   �˺���Ҫ����Stir_Motor_Speed_Control����
  */
uint16_t stirspeed = 5500;
uint16_t itemp=1000;

void StirMotor_Control(void)
{

  if(StirStatus == StirStatus_SpeedControl)
    RM2006_StirMotor.TargetSpeed = -1*stirspeed;//5300
  if(StirStatus == StirStatus_Stop)
    RM2006_StirMotor.TargetSpeed = 0;
  
  StirMotor_Blocked_Detect(&RM2006_StirMotor);
  
  //�����⵽��ת��ת3����������
  if(RM2006_StirMotor.BlockedWarningTimes > 0)
  {
    RM2006_StirMotor.TargetSpeed = 2000;
    RM2006_StirMotor.BlockedWarningTimes--;
  }
  
  Stir_Motor_Speed_Control(&RM2006_StirMotor);
	
	if(ControlMode == ControlMode_Protect)
		RM2006_StirMotor.NeedCurrent = 0;
//  if(StirStatus == StirStatus_SpeedControl)
//    RM2006_StirMotor.NeedCurrent = itemp;//5300
//  if(StirStatus == StirStatus_Stop)
//    RM2006_StirMotor.NeedCurrent = 0;
}

/**
  * @brief  ���̵���ٶȱջ�
  * @param  RM�����̵���ṹ��
  * @retval void
  */
void Stir_Motor_Speed_Control(ESC_820R* motor)
{
  motor->PID.Last_Error = motor->PID.Cur_Error;
  motor->PID.Cur_Error = motor->TargetSpeed - motor->RealSpeed;

  motor->PID.Sum_Error += motor->PID.Cur_Error;
  //��������
  motor->PID.Sum_Error = motor->PID.Sum_Error > 10000 ? 10000 : motor->PID.Sum_Error;
  motor->PID.Sum_Error = motor->PID.Sum_Error < -10000 ? -10000 : motor->PID.Sum_Error;

  motor->PID.Output = (motor->PID.Kp * motor->PID.Cur_Error\
                                    + motor->PID.Ki * motor->PID.Sum_Error\
                                    + motor->PID.Kd * (motor->PID.Cur_Error - motor->PID.Last_Error));
  //�����������
  motor->NeedCurrent = motor->PID.Output > C610CURRENTMAX ? C610CURRENTMAX : motor->PID.Output;
  motor->NeedCurrent = motor->PID.Output < -C610CURRENTMAX ? -C610CURRENTMAX : motor->NeedCurrent;
}

/**
  * @brief  ���̵����ת���
  * @param  RM�����̵���ṹ��
  * @retval void
  * @note   100ms��� 200ms��ת
  */
void StirMotor_Blocked_Detect(ESC_820R* motor)
{
  static uint8_t BlockedTimes = 0;
  //��ת������ɺ��ٴμ��
  if(motor->BlockedWarningTimes <= 0)
  {
    if(abs(motor->RealSpeed) < 100 && abs(motor->PID.Sum_Error) == 10000) 
      BlockedTimes++;
    else
      BlockedTimes = 0;
    //����100ms��⵽��ת
    if(BlockedTimes >= 20)
    {
      motor->BlockedWarningTimes = 40;
      BlockedTimes = 0;
    }
  }
}



/**
  * @brief  ��ȡ���ٺ���ת���ĽǶȣ�96:1��
  * @param  RM�����̵���ṹ��
  * @retval none
  * @note   ��Ҫ���㴦������ �Ƕ�ֵ���offset
	* @note		temp��ZreoCounter���������!!!!!!
  */
void StirMotor_Get_Angle(StirMotor_Struct* RM)
{
  static int64_t temp = 0;
  temp = (int64_t)(RM->RealAngle + (RM->CrossZero.ZreoCounter * ONECIRCLECOUNTVALUEORIGINAL) - RM->AngleOffset);
  if(temp > ONECIRCLECOUNTVALUEAFTERDECELERATE || temp <- ONECIRCLECOUNTVALUEAFTERDECELERATE)
  {
    while(temp > ONECIRCLECOUNTVALUEAFTERDECELERATE)  temp -= ONECIRCLECOUNTVALUEAFTERDECELERATE;
    while(temp < -ONECIRCLECOUNTVALUEAFTERDECELERATE) temp += ONECIRCLECOUNTVALUEAFTERDECELERATE;
  }
  RM->RealAngle_Decelerate = StirMotorChangeToAbsAngle(temp);
}
/**
  * @brief  �ǶȵĹ��㴦��
  * @param  RM�����̵���ṹ��
  * @retval void
  * @note   ֻ���ڲ��̽��нǶȿ���ʱ�Ž��й��㴦��
  */
void  Cross_Zero_Settle(StirMotor_Struct* RM)
{
  static  uint8_t OffsetValueSetNumberOfTimes = 0;

	if(StirStatus == StirStatus_AngleControl)  
  {
    //ֻ��ֵһ��
    if(OffsetValueSetNumberOfTimes == 0)
		{
			RM->AngleOffset = RM->RealAngle;
			OffsetValueSetNumberOfTimes++;
		}
    //�жϹ���
    if(RM->RealSpeed > 10)           //�������
    {
      //if(RM->LastRealAngle < RM->RealAngle) RM->CrossZero.ZreoCounter++;
      if(RM->RealAngle > CROSSZEROUPTHRESHOLD)  RM->CrossZero.InUpHalfCircle = 1;
      if(RM->CrossZero.InUpHalfCircle && RM->RealAngle < CROSSZERODOWNTHRESHOLD)
      {
        RM->CrossZero.ZreoCounter++;
        RM->CrossZero.InUpHalfCircle = 0;
      }
    }
    else if(RM->RealSpeed < -10)     //�������
    {
      //if(RM->RealAngle > RM->LastRealAngle) RM->CrossZero.ZreoCounter--;
      if(RM->RealAngle < CROSSZERODOWNTHRESHOLD)  RM->CrossZero.InDownHalfCircle = 1;
      if(RM->CrossZero.InDownHalfCircle && RM->RealAngle > CROSSZEROUPTHRESHOLD)
      {
        RM->CrossZero.ZreoCounter--;
        RM->CrossZero.InDownHalfCircle = 0;
      }
    }
  }
  else 
  {
    RM->CrossZero.ZreoCounter = 0;
    OffsetValueSetNumberOfTimes = 0;
  }
}

