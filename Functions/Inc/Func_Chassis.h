#ifndef __FUNC_CHASSIS_H
#define __FUNC_CHASSIS_H

#include "System_Config.h"
#include "Func_StatusMachine.h"

#ifdef __FUNC_CHASSIS_GLOBALS
#define __FUNC_CHASSIS_EXT
#else
#define __FUNC_CHASSIS_EXT extern
#endif

//�������ݷ���CANID
#define CHASSISCANID 0x205
//�������ݽ���CANID
#define CHASSISSENDCANID 0x1FF
//M3508�������ٶ�
#define M3508SPEEDMAX 8800.0
//C620����������ֵ
#define C620CURRENTMAX 16000
//�������ģʽ
#define CHASSISS_STOP 0
#define CHASSISS_RUN 1

//�ٶ�����ö��
typedef enum
{
    ChassisSpeedLevel_Hight = 0,
    ChassisSpeedLevel_Low
} ChassisSpeedLevel_Enum;

//M3508 PID�ṹ��
typedef struct
{
    float Kp, Ki, Kd;
    int16_t Cur_Error, Last_Error, Sum_Error;
    int32_t Output;
} M3508_PID_Struct;

//�����������
typedef struct
{
    
    int16_t Current_n;              //���Ȧ��
    int16_t Mechanical_n;           //ת��Ȧ��
    uint16_t Mechanical_Angle;      //��ǰ��е��
    uint16_t Last_Mechanical_Angle; //ԭ�Ȼ�е��
    int16_t TargetSpeed;
    int16_t RealSpeed;
    int16_t NeedCurrent;
    int16_t RealCurrent;
    uint16_t FrameCounter; //֡�ʼ�����
    M3508_PID_Struct PID;
} OneMotorParam_Struct;

//���̲���
typedef struct
{
    chassis_mode_Enum Chassis_Mode;
    OneMotorParam_Struct Motor;
    ChassisSpeedLevel_Enum SpeedLevel;
    // int16_t TargetABSAngle;
} ChassisParam_Struct;

//����״̬
extern ChassisParam_Struct ChassisParam;
void M3508_CMD_Trans(int16_t Output);
void Chassis_Ctrl(void);
int8_t If_Pass_Zero(void);
void Chassis_Ctrl_Init(void);
void Chassis_Speed_Set(void);

#endif
