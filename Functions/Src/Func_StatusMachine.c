#include "Func_JetsonComm.h"
#include "Func_StatusMachine.h"
#include "Func_RC.h"
#include "Func_FricMotor.h"
#include "Task_Tasks.h"
#include "Func_JudgeReceive.h"


BoardToBoardComm_Enum BoardToBoardComm = Comm_Off;


ControlMode_Enum ControlMode = ControlMode_Protect;
FricStatus_Enum FricStatus = FricStatus_Stop;
StirStatus_Enum StirStatus = StirStatus_Stop;
uint8_t ShootStatus;

/**
  * @brief  ״̬������
  * @param  void
  * @retval void
  * @note   �Ҳ��뿪���ϣ�����ģʽ�����ϵ��ȫ���ر�
            �Ҳ��뿪���У�ң�������ƣ��������������Ͽ���Ħ���֣����·���
            �Ҳ��뿪���£��Զ����ƣ������ϲ����
  */
uint8_t AutoShootEnable = pdFALSE;
void StatusMachine_Update(void)
{

    static uint8_t FricCtrlRelease = 1;
//    static uint8_t StirKBCtrlRelease = 1;
    //֡�ʹ��ͱ���
    //����ģʽ�л�
    switch (Get_Switch_Val(&RC_ReceiveData, RC_SW_Right))
    {
    case RC_SW_MID:
    {
        ControlMode = ControlMode_RC;
			
        break;
    }
    case RC_SW_DOWN:
    {
        ControlMode = ControlMode_Auto;
        break;
    }
    default:
    {
        ControlMode = ControlMode_Protect;
        break;
    }
    }

    //��ģʽ�л�
    switch (ControlMode)
    {
    case ControlMode_RC: //ң����ģʽ
    {
        /*******************************************  ��  Ħ����  ��  *******************************************/
//				if(aerial_robot_energy.energy_point>0&&aerial_robot_energy.attack_time>0)
//				{
				
        if (Get_Switch_Val(&RC_ReceiveData, RC_SW_Left) == RC_SW_UP)
        {
            if (FricCtrlRelease)
            {
								if(FricStatus == FricStatus_Stop)
								{
									FricStatus = FricStatus_Working_High;
								}
                else if (FricStatus == FricStatus_Working_High)
                {
                    FricStatus = FricStatus_Stop;
                }
                FricCtrlRelease = 0;
            }
        }
        else
        {
            FricCtrlRelease = 1;
        }
        /*******************************************  ��  Ħ����  ��  *******************************************/
        /*******************************************  ��   ����   ��  *******************************************/
        if ((Get_Switch_Val(&RC_ReceiveData, RC_SW_Left) == RC_SW_DOWN))
				{
            StirStatus = StirStatus_SpeedControl;
        }
        else
        {
            StirStatus = StirStatus_Stop;
        }
        /*******************************************  ��   ����   ��  *******************************************/
				break;
			}
        
    case ControlMode_Auto: //����ģʽ
    {
				
				//	if(aerial_robot_energy.attack_time==0||aerial_robot_energy.energy_point==0)
//		FricStatus=FricStatus_Stop;

				if(Get_Keyboard_Val(&RC_ReceiveData,KEY_G)&&!Get_Keyboard_Val(&LastRC_ReceiveData,KEY_G))
				{
					FricStatus=FricStatus_Stop;
					FricCtrlRelease = 1;
				}
				if(Get_Keyboard_Val(&RC_ReceiveData,KEY_F)&&!Get_Keyboard_Val(&LastRC_ReceiveData,KEY_F))
        {
          
						FricStatus = FricStatus_Working_High;
            FricCtrlRelease = 0;
          
        }
//				else
//				{
//						FricStatus = FricStatus_Stop;
//						FricCtrlRelease = 1;
//				}
            

        if(Get_Mouse_Pressed(&RC_ReceiveData,MOUSE_LEFT)&&(FricCtrlRelease == 0))
            StirStatus = StirStatus_SpeedControl;
        else
            StirStatus = StirStatus_Stop;
        break;
//				 
    }
    case ControlMode_Protect: //����ģʽ
    {
        
       FricStatus = FricStatus_Stop;
       StirStatus = StirStatus_Stop;
        
        break;
    }
    }

}
