#ifndef __FUNC_STATUSMACHINE_H
#define __FUNC_STATUSMACHINE_H

#include "System_Config.h"

typedef enum
{
    chassis_stop = 0,
    chassis_run
}chassis_mode_Enum;
 




//�����û�����״̬
typedef enum{
    ButtonPressedOnce = 0,
    ButtonTaskFinish
} UserButtonStatus_Enum;

//Ħ����״̬
typedef enum
{
    FricStatus_Stop,
    FricStatus_Working_Low,
    FricStatus_Working_High,
    FricStatus_Working_Dynamic,
}FricStatus_Enum;

//����״̬
typedef enum
{
    StirStatus_Stop,
    StirStatus_SpeedControl,
    StirStatus_AngleControl,
}StirStatus_Enum;

//����ģʽ
typedef enum
{
    ControlMode_Protect,            //����ģʽ��ȫ��ֹͣ 
    ControlMode_RC,                 //ң�������ƣ�����ʽ��
    ControlMode_Auto,               //�Զ��������
}ControlMode_Enum;

//��׼ģʽ
typedef enum
{
    NormalAim, 
    AutoAim
}AimModeStatus_Enum;



//״̬��
extern UserButtonStatus_Enum    UserButtonStatus;
extern ControlMode_Enum         ControlMode;            //����ģʽ
extern FricStatus_Enum FricStatus;    //Ħ����״̬
extern StirStatus_Enum          StirStatus;             //����״̬
extern AimModeStatus_Enum   AimStatus;//��׼״̬
extern uint8_t ShootStatus;

typedef enum
{
    Comm_Off = 0,
    Comm_On
}BoardToBoardComm_Enum;
extern BoardToBoardComm_Enum BoardToBoardComm;

void StatusMachine_Init(void);
void StatusMachine_Update(void);

#endif
