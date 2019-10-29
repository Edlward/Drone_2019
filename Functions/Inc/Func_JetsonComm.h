#ifndef __FUNC_JETSONCOMM_H
#define __FUNC_JETSONCOMM_H

#include "Task_SystInit.h"
#include <arm_math.h> 


#define mat_init   arm_mat_init_f32
#define mat_add     arm_mat_add_f32 
#define mat_sub     arm_mat_sub_f32
#define mat_mult    arm_mat_mult_f32
#define mat_trans   arm_mat_trans_f32
#define mat_inv     arm_mat_inverse_f32

#define mat         arm_matrix_instance_f32

/*
    ��ͷ������תΪ��
    ��ͷ����ǰ��Ϊ��
    �ڱ������ֶα�����ӷ����˶�Ϊ��
*/

#define JetsonCommReservedFrameLEN  5

#define JETSONFLAG_LEN      16

//֡ͷ֡β
#define JetsonCommSOF     0x66
#define JetsonCommEOF     0x88
#define CommSetUp         (uint16_t)0xCCCC
#define RecordAngle       (uint16_t)0xffff
#define RequestTrans      (uint16_t)0xbbbb
//����������
#define BlueTeam          (uint16_t)0xDDDD
#define RedTeam           (uint16_t)0xEEEE
//���䷽ʽ
#define NoFire            (uint16_t)(0x00<<8)      //������
#define BurstFire         (uint16_t)(0x01<<8)      //����
#define RunningFire       (uint16_t)(0x02<<8)      //����
//�����ٶȣ����ٵ��٣�
#define HighBulletSpeed   (uint16_t)(0x01)
#define LowBulletSpeed    (uint16_t)(0x02)
//�������ģʽ
#define ManualMode        (uint8_t)(0x00)          //�ֶ�����
#define SmallBuffMode     (uint8_t)(0x01)          //С��ģʽ
#define BigBuffMode       (uint8_t)(0x02)          //���ģʽ
#define AutoShootMode     (uint8_t)(0x03)          //�Զ����
//�ڱ���̨����ģʽ
#define RotatinPatrol     (uint8_t)(0x01)          //��תѲ��
#define PatrolArmor0      (uint8_t)(0x02)          //Ѳ��װ�װ�0
#define PatrolArmor1      (uint8_t)(0x03)          //Ѳ��װ�װ�1
#define ServoMode         (uint8_t)(0x04)          //�ŷ����

typedef struct{
    uint8_t  SoF;
    uint8_t  Seq;
    uint16_t ShootMode;            //�߰�λ���䷽ʽ �Ͱ�λ�����ٶȵȼ�  (0xFFFF-��¼��ǰ�Ƕ�  0xEEEE-�췽  0xDDDD-����  0xCCCC-ͨ�Ž���  0xBBBB-�������ݷ���)
    float    TargetPitchAngle;     //PitchĿ��Ƕ�
    float    TargetYawAngle;       //YawĿ��Ƕ�
    /*  �ڱ�ר��   */
    int16_t  TargetSpeedOnRail;    //Ŀ�����ٶȣ��ڱ��ã�     
    uint8_t  SentryGimbalMode;     //�ڱ���̨����ģʽ
    uint8_t  EoF;
} JetsonToSTM_Struct;

typedef struct
{
	//��¼��ͼʱ�ĽǶ�
	float CurAngle_Pitch;
	float CurAngle_Yaw;
	//�˴���һ�ξ��ԽǶ�
	float Velocity_Pitch;
	float Velocity_Yaw;
	//�Ƿ��¼���Ƕ�
	uint8_t ChangeAngle_flag;
}JetsonFlag_Struct;

typedef struct{
    uint8_t  SoF;
    uint8_t  Seq;
    uint8_t  NeedMode;             //�������ģʽ
    uint8_t  ShootSpeed;           //����
    /*  �ڱ�ר��   */
    uint8_t  RailNum;              //����������
    uint8_t  ArmorType;            //�����װ�װ��ʶ
    uint16_t RemainHP;             //ʣ��Ѫ��
    uint8_t  Reserved[11];         //�����ֽ�
    uint8_t  EoF;
} STMToJetson_Struct;

typedef struct{
    uint16_t  team;
    uint8_t   CommSuccess;
} CommStatus_Struct;

extern JetsonFlag_Struct JetsonFlag[JETSONFLAG_LEN];
extern uint8_t Jetson_Seq;

extern  JetsonToSTM_Struct  DataRecFromJetson;
extern  STMToJetson_Struct  DataSendToJetson;
extern  CommStatus_Struct   CommStatus;
extern float Pitch_Desire, Yaw_Desire;

void JetsonCommUart_Config(UART_HandleTypeDef *huart);
void JetsonCommUart_ReConfig_In_IRQHandler(UART_HandleTypeDef *huart);
void JetsonComm_Control(UART_HandleTypeDef *huart);

extern float Pitch_Desire, Yaw_Desire;
extern float Jetson_AnglePitch;
extern float Jetson_AngleYaw;
extern float Jetson_VelocityPitch;
extern float Jetson_VelocityYaw;


typedef struct
{
    float raw_value;
    float filtered_value[2];
    mat xhat; 
    mat xhatminus; 
    mat z; 
    mat A; 
    mat H; 
    mat AT; 
    mat HT; 
    mat Q; 
    mat R; 
    mat P; 
    mat Pminus; 
    mat K;
		mat B;//
		mat u;//
}kalman_filter_t;

typedef struct
{
  float raw_value;
  float filtered_value[2];
  float xhat_data[2], xhatminus_data[2], z_data[2],Pminus_data[4], K_data[4];
  float P_data[4];
  float AT_data[4], HT_data[4];
  float A_data[4];
  float H_data[4];
  float Q_data[4];
  float R_data[4];
	float B_data[2];//
	float *u_data;//
}kalman_filter_init_t;

void kalman_filter_init(kalman_filter_t *F, kalman_filter_init_t *I);

float *amended_kalman_filter_calc(kalman_filter_t *F, float signal1, float signal2, float signal3);
float *kalman_filter_calc(kalman_filter_t *F, float signal1, float signal2, float signal3);

void KF_Init(void);
void KF_Cal_Desire(void);
void Version_Init(void);
#endif
