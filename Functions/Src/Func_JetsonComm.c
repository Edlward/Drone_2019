#include "Func_JetsonComm.h"
#include "Func_StatusMachine.h"
#include "Func_GimbalMotor.h"
#include "Func_FricMotor.h"
#include "Func_Imu_OB.h"

kalman_filter_t KF_Gimbal_Pitch, KF_Gimbal_Yaw;
kalman_filter_init_t KF_Gimbal_Pitch_init, KF_Gimbal_Yaw_init;
float Pitch_Desire, Yaw_Desire;
float Jetson_AnglePitch = 0.0;
float Jetson_AngleYaw = 0.0;
float Jetson_VelocityPitch = 0.0;
float Jetson_VelocityYaw = 0.0;
float Jetson_AccelerationPitch = 0.0;
float Jetson_AccelerationYaw = 0.0;
float Pre_Pitch_Velocity = 0.0;
float Pre_Yaw_Velocity = 0.0;

JetsonFlag_Struct   JetsonFlag[JETSONFLAG_LEN]; //16�����������˷��ӳ٣����ڼ�¼����
uint8_t Jetson_Seq;
float Pitch_Desire, Yaw_Desire;
JetsonToSTM_Struct  DataRecFromJetson_Temp,DataRecFromJetson; //���������˷�ĳЩ���ܸ��ǵĴ���
STMToJetson_Struct  DataSendToJetson = {    //seq�Ǽ�¼�ĵڼ������� eofβ֡ soqͷ֡
  .Seq = 0,
  .SoF = JetsonCommSOF,
  .EoF = JetsonCommEOF
};

CommStatus_Struct   CommStatus = {
  .CommSuccess = 0,
  .team = 0
};


//static HAL_StatusTypeDef MY_UART_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
//��������������ǰѲ���һ�Ĵ��ڣ����ղ��������������浽������

/**
 * @brief  ͨ�Ŵ���DMA����
 * @param  huart������ṹ���ַ
 * @retval none
 */
void JetsonCommUart_Config(UART_HandleTypeDef *huart)//���������init�ĳ�ʼ���ж��壬��uart7��һ��dma�ڴ�
{
    //����
    HAL_UART_Receive_DMA(huart,(uint8_t *)&DataRecFromJetson_Temp,sizeof(JetsonToSTM_Struct)+JetsonCommReservedFrameLEN);//���Ĵ������ڴ�
    __HAL_UART_ENABLE_IT(huart,UART_IT_IDLE);  //ʹ�ܴ��ڿ����ж�
}

/**
 * @brief  ��Jetsonͨ�����ݴ���
 * @param  huart������ṹ���ַ
 * @note
 */
void JetsonComm_Control(UART_HandleTypeDef *huart)
{

  static float Pre_Pitch_Desire, Pre_Yaw_Desire;
  TickType_t Cur_time, delta_time;
  static TickType_t Pre_time;
  uint8_t Seq;
//���溯���ĵڶ����������ڴ���6�Ľ����жϺ�����ֵ��
  memcpy(&DataRecFromJetson,&DataRecFromJetson_Temp,sizeof(JetsonToSTM_Struct));//�Ӳ���������n���ַ�������һ
  Seq = DataRecFromJetson.Seq % JETSONFLAG_LEN;
  //ͨ�Ž�������
  if(DataRecFromJetson.ShootMode == CommSetUp) 
  {
    //���͵�ǰ������
    if(WeAreRedTeam) 
    {
      CommStatus.team = RedTeam; 
      DataSendToJetson.Seq++;
      DataSendToJetson.NeedMode = (uint8_t)(RedTeam >> 8);
      DataSendToJetson.ShootSpeed = (uint8_t)(RedTeam);
    }
    else if(WeAreBlueTeam)
    {
      CommStatus.team = BlueTeam; 
      DataSendToJetson.Seq++;
      DataSendToJetson.NeedMode = (uint8_t)(BlueTeam >> 8);
      DataSendToJetson.ShootSpeed = (uint8_t)(BlueTeam);
    }
    HAL_UART_Transmit_DMA(huart,(uint8_t *)&DataSendToJetson,sizeof(STMToJetson_Struct));
  }
  //ͨ�ųɹ�ȷ��֡
  else if(DataRecFromJetson.ShootMode == CommStatus.team)  
  {
    CommStatus.CommSuccess = 1;
  }
  //�������ݴ���
  else if(DataRecFromJetson.ShootMode == RequestTrans)
  {
    DataSendToJetson.Seq++;
    DataSendToJetson.NeedMode = ShootStatus;
    HAL_UART_Transmit_DMA(huart,(uint8_t *)&DataSendToJetson,sizeof(STMToJetson_Struct));
  }
  //��¼��ǰ�Ƕ�
  else if(DataRecFromJetson.ShootMode == RecordAngle)
  {
    //��¼��ʱ��ȡͼƬ��Ӧ����̬
		float a = PITCH_ANGLE, b = YAW_ANGLE;
		while(a > 180)
		{
			a= a - 360;
		}
		while(b > 180)
		{
			b= b - 360;
		}
    JetsonFlag[Seq].CurAngle_Pitch = a;
    JetsonFlag[Seq].CurAngle_Yaw = b;
		
    //��¼��̬�ı�־λ�����Ѽ�¼
    JetsonFlag[Seq].ChangeAngle_flag = 1;

    // Motor6623_Pitch._RecordAngle = PersonalGYRO.PitchAngle;
    // Motor6623_Yaw._RecordAngle = PersonalGYRO.YawAngle;
  }
	else if (JetsonFlag[Seq].ChangeAngle_flag)
  {
    //��¼��ǰʱ��
    Cur_time = xTaskGetTickCount();
    //����˴ο������ϴε�ʱ���
    delta_time = Cur_time - Pre_time;
    //
    Pre_time = Cur_time;
    //��ȡ�������Ӧ�Ķ�ͼ���
    Jetson_Seq = Seq;
    //�����¼��̬�ı�־λ
    JetsonFlag[Jetson_Seq].ChangeAngle_flag = 0;
//��������������������������������������������������������������������������������������������������������������������������������������������
    //Pitch���Ŀ��Ƕ�
    Pitch_Desire = JetsonFlag[Jetson_Seq].CurAngle_Pitch - DataRecFromJetson.TargetPitchAngle;//����ϵ�෴
		Jetson_AnglePitch = Pitch_Desire;
		//Yaw���Ŀ��Ƕ�
    Yaw_Desire = JetsonFlag[Jetson_Seq].CurAngle_Yaw - DataRecFromJetson.TargetYawAngle;
		Jetson_AngleYaw = Yaw_Desire;
    //Pitch��Ľ��ٶȣ��ǶȲ����ʱ���ı�ֵ��
		if(delta_time!=0)
    {
      JetsonFlag[Jetson_Seq].Velocity_Pitch = (Pitch_Desire - Pre_Pitch_Desire) * 1000 / delta_time;
    }
    else
    {
      JetsonFlag[Jetson_Seq].Velocity_Pitch = 0;
    }
    Pre_Pitch_Desire = Pitch_Desire;
		Jetson_AccelerationPitch=0;//(JetsonFlag[Jetson_Seq].Velocity_Pitch - Jetson_VelocityPitch) * 1000 / delta_time;

//��������������������������������������������������������������������������������������������������������������������������������������������������
		
		JetsonFlag[Jetson_Seq].Velocity_Yaw = (Yaw_Desire - Pre_Yaw_Desire) * 1000 / delta_time;
		if(delta_time!=0)
    {
      JetsonFlag[Jetson_Seq].Velocity_Yaw = (Yaw_Desire - Pre_Yaw_Desire) * 1000 / delta_time;
    }
    else
    {
      JetsonFlag[Jetson_Seq].Velocity_Yaw = 0;
    }
		Pre_Yaw_Desire = Yaw_Desire;
		Jetson_AccelerationPitch=0;//(JetsonFlag[Jetson_Seq].Velocity_Yaw - Jetson_VelocityYaw) * 1000 / delta_time;
    
		Jetson_VelocityPitch = JetsonFlag[Jetson_Seq].Velocity_Pitch;
    Jetson_VelocityYaw = JetsonFlag[Jetson_Seq].Velocity_Yaw;
//��������������������������������������������������������������������������������������������������������������������������������������������
		
		if((DataRecFromJetson.ShootMode>>8) == (NoFire >> 8))
		{
			Laser_On(); 
		}
		if((DataRecFromJetson.ShootMode>>8) == (RunningFire >> 8))
		{
			Laser_Off(); 
		}
  }
}


//��������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������
/**
  * @brief  �Ӿ�ʶ�𿨶����˲��ĳ�ʼ��
  * @param  None
  * @retval None
  * @note	None
  */
void KF_Init()
{
	//matrix A init
	KF_Gimbal_Pitch_init.A_data[0] = 1;	    KF_Gimbal_Pitch_init.A_data[1] = 0.002;//��̨��������2ms
	KF_Gimbal_Pitch_init.A_data[2] = 0;	    KF_Gimbal_Pitch_init.A_data[3] = 1;
	//matrix B init
	KF_Gimbal_Pitch_init.B_data[0] = 0.0002;
	KF_Gimbal_Pitch_init.B_data[1] = 0.002;
	//matrix H init
	KF_Gimbal_Pitch_init.H_data[0] = 1;	    KF_Gimbal_Pitch_init.H_data[1] = 0;
	KF_Gimbal_Pitch_init.H_data[2] = 0;	    KF_Gimbal_Pitch_init.H_data[3] = 1;
	//matrix Q init
	KF_Gimbal_Pitch_init.Q_data[0] = 1;	    KF_Gimbal_Pitch_init.Q_data[1] = 0;
	KF_Gimbal_Pitch_init.Q_data[2] = 0;	    KF_Gimbal_Pitch_init.Q_data[3] = 1;
	//matrix R init
	KF_Gimbal_Pitch_init.R_data[0] = 5000;		KF_Gimbal_Pitch_init.R_data[1] = 0;
	KF_Gimbal_Pitch_init.R_data[2] = 0;	    KF_Gimbal_Pitch_init.R_data[3] = 1;
	
//	KF_Gimbal_Pitch_init.xhat_data[0] = GimbalPitch.Position.Attitude;
//	KF_Gimbal_Pitch_init.xhat_data[1] = 0;
//	KF_Gimbal_Pitch_init.P_data[0] = 100;
//	KF_Gimbal_Pitch_init.P_data[1] = 0;
//	KF_Gimbal_Pitch_init.P_data[2] = 0;
//	KF_Gimbal_Pitch_init.P_data[3] = 100;

  kalman_filter_init(&KF_Gimbal_Pitch, &KF_Gimbal_Pitch_init);

  //matrix A init
	KF_Gimbal_Yaw_init.A_data[0] = 1;	      KF_Gimbal_Yaw_init.A_data[1] = 0.002;//��̨��������2ms
	KF_Gimbal_Yaw_init.A_data[2] = 0;	      KF_Gimbal_Yaw_init.A_data[3] = 1;
	//matrix B init
	KF_Gimbal_Yaw_init.B_data[0] = 0.0002;
	KF_Gimbal_Yaw_init.B_data[1] = 0.002;
	//matrix H init
	KF_Gimbal_Yaw_init.H_data[0] = 1;	      KF_Gimbal_Yaw_init.H_data[1] = 0;
	KF_Gimbal_Yaw_init.H_data[2] = 0;	      KF_Gimbal_Yaw_init.H_data[3] = 1;
//	//matrix Q init
//	KF_Gimbal_Yaw_init.Q_data[0] = 0.4;	      KF_Gimbal_Yaw_init.Q_data[1] = 0;
//	KF_Gimbal_Yaw_init.Q_data[2] = 0;	      KF_Gimbal_Yaw_init.Q_data[3] = 0.5;
//	//matrix R init
//	KF_Gimbal_Yaw_init.R_data[0] = 55;	  		KF_Gimbal_Yaw_init.R_data[1] = 0;
//	KF_Gimbal_Yaw_init.R_data[2] = 0;	      KF_Gimbal_Yaw_init.R_data[3] = 100;
	
		//matrix Q init
	KF_Gimbal_Yaw_init.Q_data[0] = 1;	      KF_Gimbal_Yaw_init.Q_data[1] = 0;
	KF_Gimbal_Yaw_init.Q_data[2] = 0;	      KF_Gimbal_Yaw_init.Q_data[3] = 1;
	//matrix R init
	KF_Gimbal_Yaw_init.R_data[0] = 800;	  		KF_Gimbal_Yaw_init.R_data[1] = 0;
	KF_Gimbal_Yaw_init.R_data[2] = 0;	      KF_Gimbal_Yaw_init.R_data[3] = 5000;
	
//	KF_Gimbal_Yaw_init.xhat_data[0] = GimbalYaw.Position.Attitude;
//	KF_Gimbal_Yaw_init.xhat_data[1] = 0;
//	KF_Gimbal_Yaw_init.P_data[0] = 100;
//	KF_Gimbal_Yaw_init.P_data[1] = 0;
//	KF_Gimbal_Yaw_init.P_data[2] = 0;
//	KF_Gimbal_Yaw_init.P_data[3] = 100;

  kalman_filter_init(&KF_Gimbal_Yaw, &KF_Gimbal_Yaw_init);
}

/**
  * @brief  ���Ӿ�ʶ������ݽ��п������˲�
  * @param  None
  * @retval None
  * @note	None
  */
void  KF_Cal_Desire()
{
  float *result;

  //�õ��˲����Ŀ���
  result = kalman_filter_calc(&KF_Gimbal_Pitch,
                              Jetson_AnglePitch,
																//+ JetsonFlag[Jetson_Seq].Velocity_Pitch*JetsonFlag[Jetson_Seq].Cal_time/1000,
                             Jetson_VelocityPitch, Jetson_AccelerationPitch);
  //������˲�֮���Ŀ��Ǳ仯��
  Pitch_Desire = result[0];// + 0.18*result[1];// + result[1]*JetsonFlag[Jetson_Seq].Cal_time/1000;

  //�õ��˲����Ŀ���
  result = kalman_filter_calc(&KF_Gimbal_Yaw,
                              Jetson_AngleYaw,
                              //  + JetsonFlag[Jetson_Seq].Velocity_Yaw*JetsonFlag[Jetson_Seq].Cal_time/1000,
                              Jetson_VelocityYaw, Jetson_AccelerationYaw);

  //������˲�֮���Ŀ��Ǳ仯��
  Yaw_Desire = result[0] + 0.18f*result[1];// + result[1]*JetsonFlag[Jetson_Seq].Cal_time/1000;

}


void Version_Init()
{
  uint8_t i = 0;
  //DataRecFromJetson.TargetPitchAngle = 0; //PitchĿ��Ƕ�
  //DataRecFromJetson.TargetYawAngle = 0;   //YawĿ��Ƕ�
  for (; i < 10; i++)
  {
    JetsonFlag[i].CurAngle_Pitch = PITCH_ANGLE;
    JetsonFlag[i].CurAngle_Yaw = YAW_ANGLE;
    JetsonFlag[i].Velocity_Pitch = 0;
    JetsonFlag[i].Velocity_Yaw = 0;
    JetsonFlag[i].ChangeAngle_flag = 0;
  }
  KF_Init();
}

void kalman_filter_init(kalman_filter_t *F, kalman_filter_init_t *I)
{
  mat_init(&F->xhat,2,1,(float *)I->xhat_data);
  mat_init(&F->xhatminus,2,1,(float *)I->xhatminus_data);
  mat_init(&F->z,2,1,(float *)I->z_data);
  mat_init(&F->A,2,2,(float *)I->A_data);
  mat_init(&F->H,2,2,(float *)I->H_data);
  mat_init(&F->AT,2,2,(float *)I->AT_data);
  mat_trans(&F->A, &F->AT);
  mat_init(&F->Q,2,2,(float *)I->Q_data);
  mat_init(&F->R,2,2,(float *)I->R_data);
  mat_init(&F->P,2,2,(float *)I->P_data);
  mat_init(&F->Pminus,2,2,(float *)I->Pminus_data);
  mat_init(&F->K,2,2,(float *)I->K_data);
  mat_init(&F->HT,2,2,(float *)I->HT_data);
  mat_trans(&F->H, &F->HT);
	mat_init(&F->B,2,1,(float *)I->B_data);//
	mat_init(&F->u,1,1,(float *)I->u_data);//
}

float *kalman_filter_calc(kalman_filter_t *F, float signal1, float signal2, float signal3)
{
  float TEMP_data[4] = {0, 0, 0, 0};
  float TEMP_data21[2] = {0, 0};
  mat TEMP,TEMP21,TEMP211;
	
//	static float last_signal1 = 0.0;
//	static float last_signal2 = 0.0;

  mat_init(&TEMP,2,2,(float *)TEMP_data);
  mat_init(&TEMP21,2,1,(float *)TEMP_data21);
	//mat_init(&TEMP211,2,1,(float *)TEMP_data21);

  F->z.pData[0] = signal1;
  F->z.pData[1] = signal2;
	F->u.pData[0] = signal3;

  //1. xhat'(k)= A xhat(k-1) + B U
  mat_mult(&F->A, &F->xhat, &F->xhatminus);
	//mat_mult(&F->B, &F->u, &TEMP211);
	//mat_add(&TEMP21, &TEMP211, &F->xhatminus);
	

//	if(signal1 != last_signal1)
//	{
//		last_signal1 = signal1;
//		last_signal2 = signal2;
//		uint8_t flag = 0;
//		float gamma1 = sqrt(F->R.pData[0]) / F->R.pData[0];
//		float gamma2 = sqrt(F->R.pData[3]) / F->R.pData[3];
//		if(abs(TEMP211.pData[0])>0.5*sqrt(F->R.pData[0]))
//		{
//			F->xhat.pData[0] = F->xhat.pData[0] + TEMP211.pData[0]*gamma1*0.9;
//			flag = 1;
//		}
//		if(abs(TEMP211.pData[1])>0.5*sqrt(F->R.pData[3]))
//		{
//			F->xhat.pData[1] = F->xhat.pData[1] + TEMP211.pData[1]*gamma2*10;
//			flag = 1;
//		}
//		if(flag == 1)
//		{
//			//1. xhat'(k)= A xhat(k-1) + B U
//			mat_mult(&F->A, &F->xhat, &TEMP21);
//			mat_mult(&F->B, &F->u, &TEMP211);
//			mat_add(&TEMP21, &TEMP211, &F->xhatminus);

//			//4. xhat(k) = xhat'(k) + K(k) (z(k) - H xhat'(k))
//			mat_mult(&F->H, &F->xhatminus, &TEMP21);
//			mat_sub(&F->z, &TEMP21, &TEMP211);//&F->xhat==r	
//		}
//	}
  //2. P'(k) = A P(k-1) AT + Q
  mat_mult(&F->A, &F->P, &F->Pminus);
  mat_mult(&F->Pminus, &F->AT, &TEMP);
  mat_add(&TEMP, &F->Q, &F->Pminus);
	
	

  //3. K(k) = P'(k) HT / (H P'(k) HT + R)
  mat_mult(&F->H, &F->Pminus, &F->K);
  mat_mult(&F->K, &F->HT, &TEMP);
  mat_add(&TEMP, &F->R, &F->K);//======|

  mat_inv(&F->K, &F->P);
  mat_mult(&F->Pminus, &F->HT, &TEMP);
  mat_mult(&TEMP, &F->P, &F->K);
	


	//4. xhat(k) = xhat'(k) + K(k) (z(k) - H xhat'(k))
  mat_mult(&F->H, &F->xhatminus, &TEMP21);
  mat_sub(&F->z, &TEMP21, &F->xhat);//&F->xhat==r	
  mat_mult(&F->K, &F->xhat, &TEMP21);
  mat_add(&F->xhatminus, &TEMP21, &F->xhat);

  //5. P(k) = (1-K(k)H)P'(k)
  mat_mult(&F->K, &F->H, &F->P);
  mat_sub(&F->Q, &F->P, &TEMP);
  mat_mult(&TEMP, &F->Pminus, &F->P);

  F->filtered_value[0] = F->xhat.pData[0];
  F->filtered_value[1] = F->xhat.pData[1];

  return F->filtered_value;
}


float *amended_kalman_filter_calc(kalman_filter_t *F, float signal1, float signal2, float signal3)
{
  float TEMP_data[4] = {0, 0, 0, 0};
  float TEMP_data21[2] = {0, 0};
  mat TEMP,TEMP21,TEMP211,TEMP2,TEMP3;

  mat_init(&TEMP,2,2,(float *)TEMP_data);
	mat_init(&TEMP2,2,2,(float *)TEMP_data);
	mat_init(&TEMP3,2,2,(float *)TEMP_data);
  mat_init(&TEMP21,2,1,(float *)TEMP_data21);
	mat_init(&TEMP211,2,1,(float *)TEMP_data21);

  F->z.pData[0] = signal1;
  F->z.pData[1] = signal2;
	F->u.pData[0] = signal3;
	
  //Predicting
  //1. xhat'(k)= A xhat(k-1) + B U
  mat_mult(&F->A, &F->xhat, &F->xhatminus);
//	mat_mult(&F->B, &F->u, &TEMP211);
//	mat_add(&TEMP21, &TEMP211, &F->xhatminus);
	
	//(z(k) - H xhat'(k))
  //mat_mult(&F->H, &F->xhatminus, &TEMP21);
  //mat_sub(&F->z, &TEMP21, &TEMP211);//&F->xhat==r	
	mat_sub(&F->z, &F->xhatminus, &TEMP211);//&F->xhat==r	

  //2. P'(k) = A P(k-1) AT + Q
  mat_mult(&F->A, &F->P, &F->Pminus);
  mat_mult(&F->Pminus, &F->AT, &TEMP);
  mat_add(&TEMP, &F->Q, &F->Pminus);

  //(H P'(k) HT + R)
//  mat_mult(&F->H, &F->Pminus, &F->K);
//  mat_mult(&F->K, &F->HT, &TEMP);
//  mat_add(&TEMP, &F->R, &F->K);//======K-->S
	mat_add(&F->Pminus, &F->R, &F->K);//======K-->S
	
	//Amending
	//P-->Ck-1   Ck=A-1 S-1 (H P'(k) HT)
	mat_mult(&F->K, &F->A, &F->P);
	mat_inv(&F->P, &TEMP);
	mat_mult(&TEMP, &F->Pminus, &F->P);//P-->Ck-1
	
	//xhat(k-1)new = xhat(k-1) + Ck-1*TEMP211
	mat_mult(&F->P, &TEMP211, &TEMP21);
	mat_add(&F->xhat,&TEMP21,&TEMP211);
	F->xhat.pData[0] = TEMP211.pData[0];
	F->xhat.pData[1] = TEMP211.pData[1];
	
	//p(k-1)new = Pminus + (A Ck-1)S(A Ck-1)T - Pminus(A Ck-1)T - (A Ck-1)Pminus
	mat_mult(&F->A, &F->P, &TEMP);//(A Ck-1)
	mat_trans(&TEMP, &TEMP2);//(A Ck-1)T
	
	mat_mult(&TEMP,&F->K, &TEMP3);
	mat_mult(&TEMP3,&TEMP2,&F->P);//(A Ck-1)S(A Ck-1)T
	
	mat_mult(&F->Pminus,&TEMP2,&TEMP3);//Pminus(A Ck-1)T 
	mat_sub(&F->P,&TEMP3,&TEMP2);//(A Ck-1)S(A Ck-1)T - Pminus(A Ck-1)T
	
	mat_mult(&TEMP,&F->Pminus,&F->P);//(A Ck-1)Pminus
	mat_sub(&TEMP2,&F->P,&TEMP3);//(A Ck-1)S(A Ck-1)T - Pminus(A Ck-1)T - (A Ck-1)Pminus
	
	mat_add(&F->Pminus, &TEMP3, &F->P);//p(k-1)new 
	//=========Amending finished========
	
	//Recalculating
	//1. xhat'(k)= A xhat(k-1) + B U
  mat_mult(&F->A, &F->xhat, &F->xhatminus);
//	mat_mult(&F->B, &F->u, &TEMP211);
//	mat_add(&TEMP21, &TEMP211, &F->xhatminus);
	
	//2. P'(k) = A P(k-1) AT + Q
  mat_mult(&F->A, &F->P, &F->Pminus);
  mat_mult(&F->Pminus, &F->AT, &TEMP);
  mat_add(&TEMP, &F->Q, &F->Pminus);
	
  //3. K(k) = P'(k) HT / (H P'(k) HT + R)
	mat_mult(&F->H, &F->Pminus, &F->K);
  mat_mult(&F->K, &F->HT, &TEMP);
  mat_add(&TEMP, &F->R, &F->K);//======K-->S
	
  mat_inv(&F->K, &F->P);
  mat_mult(&F->Pminus, &F->HT, &TEMP);
  mat_mult(&TEMP, &F->P, &F->K);
	
	//4. xhat(k) = xhat'(k) + K(k) (z(k) - H xhat'(k))
	mat_mult(&F->H, &F->xhatminus, &TEMP21);
  mat_sub(&F->z, &TEMP21, &TEMP211);//&F->xhat==r	
	
  mat_mult(&F->K, &TEMP211, &TEMP21);
  mat_add(&F->xhatminus, &TEMP21, &F->xhat);

  //5. P(k) = (1-K(k)H)P'(k)
  mat_mult(&F->K, &F->H, &F->P);
  mat_sub(&F->Q, &F->P, &TEMP);
  mat_mult(&TEMP, &F->Pminus, &F->P);

  F->filtered_value[0] = F->xhat.pData[0];
  F->filtered_value[1] = F->xhat.pData[1];

  return F->filtered_value;
}

//��������������������������������������������������������������������������������������������������������������������������������������������������������������������-



/**
 * @brief  �ڿ����ж��е�������
 * @param  huart�����ڽṹ��ָ��
 * @retval none
 */
uint8_t debug_len = 0;
void JetsonCommUart_ReConfig_In_IRQHandler(UART_HandleTypeDef *huart)//ÿ�������־λ��Ϊ���ܹ��ٽ����ж�
{
  BaseType_t xHigherPriorityTaskToWaken = pdFALSE;//�ں����֪ͨ��֪ͨĳ����Ϣ��Jetson����
  uint8_t usart_this_time_rx_len = 0; //�˴ν��ճ���
  DMA_HandleTypeDef *hdma_uart_rx = huart->hdmarx;// ������ָ���˴��ڰѽ��յ��Ķ����ŵ���

  if (__HAL_UART_GET_IT_SOURCE(huart, UART_IT_IDLE) != RESET)//�жϴ��ڽ�����ĳ���ж�
  {
    //clear the idle pending flag ������й����ʶ
    (void)huart->Instance->SR; 
    (void)huart->Instance->DR;
  
		__HAL_UART_CLEAR_IDLEFLAG(huart);
		__HAL_DMA_DISABLE(hdma_uart_rx);//�ر�dma�ж�
		
    usart_this_time_rx_len = sizeof(JetsonToSTM_Struct)+JetsonCommReservedFrameLEN - __HAL_DMA_GET_COUNTER(hdma_uart_rx);//dma��ʣ���ٿռ�
		
		debug_len = usart_this_time_rx_len;
    __HAL_DMA_SET_COUNTER(hdma_uart_rx, (sizeof(JetsonToSTM_Struct)+JetsonCommReservedFrameLEN));//Ϊʲô ���������dma�ռ䣿
    __HAL_DMA_ENABLE(hdma_uart_rx);

    if(usart_this_time_rx_len > 0)//�жϿռ��ж�����
    {
      if(DataRecFromJetson_Temp.SoF == JetsonCommSOF && DataRecFromJetson_Temp.EoF ==JetsonCommEOF)
      //������Ϣ֪ͨ
			{
      vTaskNotifyGiveFromISR(JetsonCommTask_Handle, &xHigherPriorityTaskToWaken);
      portYIELD_FROM_ISR(xHigherPriorityTaskToWaken);
			}
    }
  }

}


/**
  * @brief  ���ڷ������������ݣ��ر�����DMA�жϣ�
  * @param  huart: ���ڽṹ��ָ��
  * @param  �ֽ�buffer��ַ
  * @param  �����ֽ���
  * @retval HAL status
  */
//HAL_StatusTypeDef MY_UART_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
//{  
//  uint32_t *tmp;
//  
//  /* Check that a Rx process is not already ongoing */
//  if(huart->RxState == HAL_UART_STATE_READY) 
//  {
//    if((pData == NULL ) || (Size == 0)) 
//    {
//      return HAL_ERROR;
//    }
//    
//    /* Process Locked */
//    __HAL_LOCK(huart);
//    
//    huart->pRxBuffPtr = pData;
//    huart->RxXferSize = Size;
//    
//    huart->ErrorCode = HAL_UART_ERROR_NONE;
//    huart->RxState = HAL_UART_STATE_BUSY_RX;
//        
//    /* Enable the DMA Stream and disable the interrupts*/
//    tmp = (uint32_t*)&pData;
//    HAL_DMA_Start(huart->hdmarx, (uint32_t)&huart->Instance->DR, *(uint32_t*)tmp, Size);

//    /* Clear the Overrun flag just before enabling the DMA Rx request: can be mandatory for the second transfer */
//    __HAL_UART_CLEAR_OREFLAG(huart);

//    /* Process Unlocked */
//    __HAL_UNLOCK(huart);

//    /* Enable the UART Parity Error Interrupt */
//    SET_BIT(huart->Instance->CR1, USART_CR1_PEIE);

//    /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
//    SET_BIT(huart->Instance->CR3, USART_CR3_EIE);
//    
//    /* Enable the DMA transfer for the receiver request by setting the DMAR bit 
//    in the UART CR3 register */
//    SET_BIT(huart->Instance->CR3, USART_CR3_DMAR);

//    return HAL_OK;
//  }
//  else
//  {
//    return HAL_BUSY; 
//  }
//}

#if 0

static HAL_StatusTypeDef MY_UART_Transmit_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);

/**
  * @brief  ������ģʽ�������ݣ��ر�����DMA�жϣ�
  * @param  huart: ���ڽṹ��ָ��
  * @param  �ֽ�buffer��ַ
  * @param  �����ֽ���
  * @retval HAL status
  * @note   �˺����޷�ʵ������ ��ѯ��state����������ô�������
  */
HAL_StatusTypeDef MY_UART_Transmit_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
  uint32_t *tmp;
  
  /* Check that a Tx process is not already ongoing */
  if(huart->gState == HAL_UART_STATE_READY)
  {
    if((pData == NULL ) || (Size == 0))
    {
      return HAL_ERROR;
    }

    /* Process Locked */
    __HAL_LOCK(huart);

    huart->pTxBuffPtr = pData;
    huart->TxXferSize = Size;
    huart->TxXferCount = Size;

    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->gState = HAL_UART_STATE_BUSY_TX;

    /* Enable the UART transmit DMA Stream */
    tmp = (uint32_t*)&pData;
    HAL_DMA_Start(huart->hdmatx, *(uint32_t*)tmp, (uint32_t)&huart->Instance->DR, Size);
    
    /* Clear the TC flag in the SR register by writing 0 to it */
    __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_TC);
    
    /* Process Unlocked */
    __HAL_UNLOCK(huart);
    
    /* Enable the DMA transfer for transmit request by setting the DMAT bit
       in the UART CR3 register */
    SET_BIT(huart->Instance->CR3, USART_CR3_DMAT);
    
    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}
#endif
 

