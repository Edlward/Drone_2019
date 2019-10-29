#ifndef __FUNC_GimbalMOTOR_H
#define __FUNC_GimbalMOTOR_H

#include "System_Config.h"

#ifdef __FUNC_GimbalMOTOR_GLOBALS
#define __FUNC_GimbalMOTOR_EXT
#else
#define __FUNC_GimbalMOTOR_EXT extern
#endif


//Yaw�ᳯ��ǰ������1285,Pitch����ǰ����е��1429
//Yaw���������1360����С����1200
//Pitch�������+13�㣬����1400������ӽ�-89�㣬����1600
//��̨�������ID
#define GimbalMOTORSENDCANID 0x1FF
//��̨�������ID
#define YawMotorReceiveID 0x201
#define PitchMotorReceiveID 0x208

//�Ƕ����Ӳ���ֵ
#define PhysicalAngleAddStep 0.96

#define YawSpeedMax   200
#define YawAngleMax   360

//M2006 ������������
#define M2006CURRENTMAX 16000

//GM3510 �������ѹ
#define GM3510VOLTAGEMAX 29000

//GM6020 �������ѹ
#define GM6020CURRENTMAX 29000


//M2006����ṹ��
typedef struct
{
    uint16_t FrameCounter;
    float TargetAngle;
		float RealAngle;
    float TargetSpeed;
		float LastTargetSpeed;
    float RealSpeed;
    uint16_t MechanicalAngle;
		uint16_t last_MechanicalAngle;
		int16_t RotationalSpeed;
    int16_t NeedCurrent;
    int16_t RealTorque;     //uint or int
    struct
    {
        float Kp, Ki, Kd;
        float Cur_Error, Last_Error, Sum_Error;
				float DeadBand;
        float Output;
    } SpeedPID;
    struct
    {
        float Kp, Ki, Kd;
        float Cur_Error, Last_Error, Sum_Error;
				float DeadBand;
        float Output;
    } PositionPID;
} M2006;

//GM6020����ṹ��
typedef struct
{
		uint16_t FrameCounter;
    float TargetAngle;
    float RealAngle;
		float last_RealAngle;
    float TargetSpeed;
		
    float RealSpeed;
    int16_t NeedCurrent;
	  int16_t RotationalSpeed;
		uint16_t MechanicalAngle;
		uint16_t last_MechanicalAngle;
    int16_t RealTorque;     
		struct
    {
        float Kp, Ki, Kd;
        float Cur_Error, Last_Error, Sum_Error;
				float DeadBand;
        float Output;
    } SpeedPID;
		struct
    {
        float Kp, Ki, Kd;
        float Cur_Error, Last_Error, Sum_Error;
				float DeadBand;
        float Output;
    } PositionPID;
}GM6020;

extern float PITCH_ANGLE;
extern float YAW_ANGLE;

extern GM6020 Motor6020_Pitch;
extern GM6020 Motor6020_Yaw;
extern M2006 Motor2006_Yaw;
extern int monitor_Pitch_TargetSpeed;
extern int monitor_Pitch_RealSpeed;
extern int monitor_Pitch_TargetAngle;
extern int monitor_Pitch_RealAngle;
extern int monitor_Yaw_TargetSpeed;
extern int monitor_Yaw_RealSpeed;
extern int monitor_Yaw_TargetAngle;
extern int monitor_Yaw_RealAngle;
void Gimbal_Init(void);
void Gimbal_Ctrl(void);
void Gimbal_CMD_Trans(int16_t Current_Yaw, int16_t Current_Pitch,int16_t Current_StirMotor);
float SmoothFilter(float data);
#endif
