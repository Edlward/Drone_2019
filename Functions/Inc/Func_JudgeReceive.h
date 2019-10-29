#ifndef __TASKJUDGERECEIVE_H
#define __TASKJUDGERECEIVE_H

#include "System_Config.h"


/*--------------CmdID(2-Byte)----------------*/
        #define GAMESTATE           0X0001 //����״̬���ݣ�1Hz ���ڷ���
        #define GAMERESULT          0X0002 //����������ݣ�������������
        #define ROBOTSURVIVORS      0X0003 //���������˴�����ݣ�1Hz ���ڷ���
        #define EVENTDATA           0X0101 //�����¼����ݣ��¼��ı����
        #define SUPPLYACTION        0X0102 //���ز���վ������ʶ���ݣ������ı����
        #define SUPPLYBOOKING       0X0103 //���󲹸�վ�������ݣ��ɲ����ӷ��ͣ����� 10Hz����RM �Կ�����δ���ţ�
        #define GAMEROBOTSTATE      0X0201 //������״̬���ݣ�10Hz ���ڷ���
        #define POWERHEATDATA       0X0202 //ʵʱ�����������ݣ�50Hz ���ڷ���
        #define GAMEROBOTPOS        0X0203 //������λ�����ݣ�10Hz ����
        #define BUFFMUSK            0X0204 //�������������ݣ�����״̬�ı����
        #define AERIALROBOTENERGY   0X0205 //���л���������״̬���ݣ�10Hz ���ڷ��ͣ�ֻ�п��л��������ط���
        #define ROBOTHURT           0X0206 //�˺�״̬���ݣ��˺���������
        #define SHOOTDATA           0X0207 //ʵʱ������ݣ��ӵ��������
        #define INTERACTIVEHEADER   0X0301 //�������ݽ�����Ϣ������Ƶ�ʣ����� 10Hz
/*--------------CmdID(2-Byte)----------------*/

/*--------------DataSize----------------*/
    #define GAMESTATE_DATA_SIZE         3
    #define GAMERESULT_DATA_SIZE        1
    #define ROBOTSURVIVORS_DATA_SIZE    2
    #define EVENTDATA_DATA_SIZE         4
    #define SUPPLYACTION_DATA_SIZE      3
    #define SUPPLYBOOKING_DATA_SIZE     2
    #define GAMEROBOTSTATE_DATA_SIZE    15
    #define POWERHEATDATA_DATA_SIZE     14
    #define GAMEROBOTPOS_DATA_SIZE      16
    #define BUFFMUSK_DATA_SIZE          1
    #define AERIALROBOTENERGY_DATA_SIZE 3
    #define ROBOTHURT_DATA_SIZE         1
    #define SHOOTDATA_DATA_SIZE         6
    #define INTERACTIVEHEADER_DATA_SIZE(n)      (n+9)
		#define JUDGE_DATA_LENGTH(n)                (n+9)
/*--------------DataSize----------------*/

/*--------------ƫ��λ��----------------*/
    #define JUDGE_SOF_OFFSET            0
    #define JUDGE_DATALENGTH_OFFSET     1
    #define JUDGE_SEQ_OFFSET            3
    #define JUDGE_CRC8_OFFSET           4
    #define JUDGE_CMDID_OFFSET          5
    #define JUDGE_DATA_OFFSET           7
    #define JUDGE_CRC16_OFFSET(n)       (n+JUDGE_DATA_OFFSET)
/*--------------ƫ��λ��----------------*/

/*-------------------------------------------CRCУ��---------------------------------------------------*/
/**
  * @brief  ����ϵͳ����У��
  * @param  __RECEIVEBUFFER__��  ���յ��Ĳ���ϵͳ����ͷ֡���ڵ�ַ
  * @param  __DATALENGTH__��     һ֡�����ڵ�������/Bytes�����ݣ�
  * @retval 1��                  У����ȷ
  * @retval 0��                  У�����
  * @note	None
  */
#define Verify_CRC_Check_Sum(__RECEIVEBUFFER__, __DATALENGTH__)      (Verify_CRC8_Check_Sum(__RECEIVEBUFFER__, JUDGE_CRC8_OFFSET+1)\
                                                    &&Verify_CRC16_Check_Sum(__RECEIVEBUFFER__, __DATALENGTH__+JUDGE_DATA_LENGTH(0)))

/**
  * @brief  ����ϵͳ���У��
  * @param  __TRANSMITBUFFER__�� ���͵�����ϵͳ��������ͷ֡���ڵ�ַ
  * @param  __DATALENGTH__��     һ֡�����ڵ�������/Bytes�����ݣ�
  * @retval None
  * @note	None
  */
#define Append_CRC_Check_Sum(__TRANSMITBUFFER__, __DATALENGTH__)                    \
do                                                                                  \
{                                                                                   \
    Append_CRC8_Check_Sum(__TRANSMITBUFFER__, JUDGE_CRC8_OFFSET+1);                 \
    Append_CRC16_Check_Sum(__TRANSMITBUFFER__, __DATALENGTH__+JUDGE_DATA_LENGTH(0));\
}while(0U)


//1.	����������״̬(0x0001)
typedef __packed struct
{
    /*
    0-3 bit����������
    ? 1��RoboMaster ���״�ʦ����
    ? 2��RoboMaster ���״�ʦ��������
    ? 3��ICRA RoboMaster �˹�������ս��
    */
    uint8_t game_type : 4;

    /*
    4-7 bit����ǰ�����׶�
    ? 0��δ��ʼ������
    ? 1��׼���׶Σ�
    ? 2���Լ�׶Σ�
    ? 3��5s ����ʱ��
    ? 4����ս�У�
    ? 5������������
    */
    uint8_t game_progress : 4;
    
    /*
    ��ǰ�׶�ʣ��ʱ�䣬��λ s
    */
    uint16_t stage_remain_time;
}ext_game_state_t;

//2.����������ݣ�0x0002������Ƶ�ʣ�������������
typedef __packed struct
{
    /*0 ƽ�� 1 �췽ʤ�� 2 ����ʤ��*/
    uint8_t winner;
} ext_game_result_t;

//3. �����˴�����ݣ�0x0003������Ƶ�ʣ�1Hz
typedef __packed struct
{
    /*
    bit 0���췽Ӣ�ۻ����ˣ�
    bit 1���췽���̻����ˣ�
    bit 2���췽���������� 1��
    bit 3���췽���������� 2��
    bit 4���췽���������� 3��
    bit 5���췽���л����ˣ�
    bit 6���췽�ڱ������ˣ�
    bit 7������
    bit 8������Ӣ�ۻ����ˣ�
    bit 9���������̻����ˣ�
    bit 10���������������� 1��
    bit 11���������������� 2��
    bit 12���������������� 3��
    bit 13���������л����ˣ�
    bit 14�������ڱ������ˣ�
    bit 15������
    ��Ӧ�� bit ��ֵ�� 1 ��������˴���ֵ�� 0 �����������������δ�ϳ���
    */
    uint16_t robot_legion;
} ext_game_robot_survivors_t;

//4.�����¼����ݣ�0x0101������Ƶ�ʣ��¼��ı����
typedef __packed struct
{
    /*
    bit 0-1������ͣ��ƺռ��״̬
    ? 0 Ϊ�޻�����ռ�죻
    ? 1 Ϊ���л�������ռ�쵫δͣ����
    ? 2 Ϊ���л�������ռ�첢ͣ��
    bit 2����������վ 1 �Ų�Ѫ��ռ��״̬ 1 Ϊ��ռ�죻
    bit 3����������վ 2 �Ų�Ѫ��ռ��״̬ 1 Ϊ��ռ�죻
    bit 4����������վ 3 �Ų�Ѫ��ռ��״̬ 1 Ϊ��ռ�죻
    bit 5-6����������������״̬��
    ? 0 Ϊ�����δռ���Ҵ���������δ���
    ? 1 Ϊ�����ռ���Ҵ���������δ���
    ? 2 Ϊ�����������Ѽ��
    ? 3 Ϊ�����������Ѽ����Ҵ���㱻ռ�죻
    bit 7�������ؿ�ռ��״̬ 1 Ϊ��ռ�죻
    bit 8�������ﱤռ��״̬ 1 Ϊ��ռ�죻
    bit 9��������Դ��ռ��״̬ 1 Ϊ��ռ�죻
    bit 10-11���������ط���״̬��
    ? 2 Ϊ���� 100%������
    ? 1 Ϊ�������ڱ�������
    ? 0 Ϊ�����޷�����
    bit 12-13��ICRA �췽�����ӳ�
    ? 0�������ӳ�δ���
    ? 1�������ӳ� 5s ���������У�
    ? 2�������ӳ��Ѽ���
    bit 14-15��ICRA ���������ӳ�
    ? 0�������ӳ�δ���
    ? 1�������ӳ� 5s ���������У�
    ? 2�������ӳ��Ѽ���
    ���ౣ��
    */
    uint32_t event_type;
} ext_event_data_t;

//5. ����վ������ʶ��0x0102������Ƶ�ʣ������ı����
typedef __packed struct
{
    /*
    ����վ�� ID��
    1��1 �Ų����ڣ�
    2��2 �Ų�����
    */
    uint8_t supply_projectile_id;

    /*
    ���������� ID��0 Ϊ��ǰ�޻����˲�����1 Ϊ�췽Ӣ�ۻ����˲�����2 Ϊ�췽����
    �����˲�����3/4/5 Ϊ�췽���������˲�����11 Ϊ����Ӣ�ۻ����˲�����12 Ϊ����
    ���̻����˲�����13/14/15 Ϊ�������������˲���
    */
    uint8_t supply_robot_id;

    /*
    �����ڿ���״̬��0 Ϊ�رգ�1 Ϊ�ӵ�׼���У�2 Ϊ�ӵ�����
    */
    uint8_t supply_projectile_step;

    /*
    ����������
    50��50 ���ӵ���
    100��100 ���ӵ���
    150��150 ���ӵ���
    200��200 ���ӵ���
    */
    uint8_t supply_projectile_num;
} ext_supply_projectile_action_t;

//6.���󲹸�վ�����ӵ���cmd_id (0x0103)������Ƶ�ʣ����� 10Hz��RM �Կ�����δ����
typedef __packed struct
{
    /*
    ����վ������ ID��
    1��1 �Ų�����
    */
    uint8_t supply_projectile_id;

    /*
    ���������� ID��1 Ϊ�췽Ӣ�ۻ����˲�����2 Ϊ�췽���̻����˲�����
    3/4/5 Ϊ�췽���������˲�����11 Ϊ����Ӣ�ۻ����˲�����12 Ϊ����
    ���̻����˲�����13/14/15 Ϊ�������������˲���
    */
    uint8_t supply_robot_id;
    uint8_t supply_num;
} ext_supply_projectile_booking_t;

//7.����������״̬��0x0201������Ƶ�ʣ�10Hz
typedef __packed struct
{
    /*
    ������ ID��
    1���췽Ӣ�ۻ����ˣ�
    2���췽���̻����ˣ�
    3/4/5���췽���������ˣ�
    6���췽���л����ˣ�
    7���췽�ڱ������ˣ�
    11������Ӣ�ۻ����ˣ�
    12���������̻����ˣ�
    13/14/15���������������ˣ�
    16���������л����ˣ�
    17�������ڱ������ˡ�
    */
    uint8_t robot_id;

    /*
    �����˵ȼ���
    1��һ����2��������3��������
    */
    uint8_t robot_level;

    /*
    ������ʣ��Ѫ��
    */
    uint16_t remain_HP;

    /*
    ����������Ѫ��
    */
    uint16_t max_HP;

    /*
    ������ 17mm ǹ��ÿ����ȴֵ
    */
    uint16_t shooter_heat0_cooling_rate;

    /*
    ������ 17mm ǹ����������
    */
    uint16_t shooter_heat0_cooling_limit;

    /*
    ������ 42mm ǹ��ÿ����ȴֵ
    */
    uint16_t shooter_heat1_cooling_rate;

    /*
    ������ 42mm ǹ����������
     */
    uint16_t shooter_heat1_cooling_limit;

    /*
    ���ص�Դ��������
    0 bit��gimbal ������� 1 Ϊ�� 24V �����0 Ϊ�� 24v �����
    1 bit��chassis �������1 Ϊ�� 24V �����0 Ϊ�� 24v �����
    2 bit��shooter �������1 Ϊ�� 24V �����0 Ϊ�� 24v �����
    */
    uint8_t mains_power_gimbal_output : 1;
    uint8_t mains_power_chassis_output : 1;
    uint8_t mains_power_shooter_output : 1;
} ext_game_robot_state_t;

//8.ʵʱ�����������ݣ�0x0202������Ƶ�ʣ�50Hz
typedef __packed struct
{
    //���������ѹ ��λ ����
    uint16_t chassis_volt;
    //����������� ��λ ����
    uint16_t chassis_current;
    //����������� ��λ W ��
    float chassis_power;
    //���̹��ʻ��� ��λ J ����
    uint16_t chassis_power_buffer;
    //17mm ǹ������
    uint16_t shooter_heat0;
    //42mm ǹ������
    uint16_t shooter_heat1;
}ext_power_heat_data_t;

//9.������λ�ã�0x0203������Ƶ�ʣ�10Hz
typedef __packed struct
{
   
    float x;//λ�� x ���꣬��λ m
    float y;//λ�� y ���꣬��λ m
    float z;//λ�� z ���꣬��λ m
    float yaw;//λ��ǹ�ڣ���λ��
} ext_game_robot_pos_t;

//10. ���������棺0x0204������Ƶ�ʣ�״̬�ı����
typedef __packed struct
{
    /*
    bit 0��������Ѫ����Ѫ״̬
    bit 1��ǹ��������ȴ����
    bit 2�������˷����ӳ�
    bit 3�������˹����ӳ�
    ���� bit ����
    */
    uint8_t power_rune_buff;
}ext_buff_musk_t;

//11. ���л���������״̬��0x0205������Ƶ�ʣ�10Hz
typedef __packed struct
{

    uint8_t energy_point;//���۵�������
    uint8_t attack_time; //�ɹ���ʱ�� ��λ s��50s �ݼ��� 0
} aerial_robot_energy_t;

//12. �˺�״̬��0x0206������Ƶ�ʣ��˺���������
typedef __packed struct
{
    /*
    bit 0-3����Ѫ���仯����Ϊװ���˺�������װ�� ID��������ֵΪ 0-4 �Ŵ��������
    �����װ��Ƭ������Ѫ���仯���ͣ��ñ�����ֵΪ 0��
    */
    uint8_t armor_id : 4;
    /*bit 4-7��Ѫ���仯����
    0x0 װ���˺���Ѫ��
    0x1 ģ����߿�Ѫ��
    0x2 ��ǹ��������Ѫ��
    0x3 �����̹��ʿ�Ѫ��
    */
    uint8_t hurt_type : 4;
} ext_robot_hurt_t;
//13. ʵʱ�����Ϣ��0x0207������Ƶ�ʣ��������
typedef __packed struct
{
    uint8_t bullet_type;//�ӵ�����: 1��17mm ���� 2��42mm ����
    uint8_t bullet_freq;//�ӵ���Ƶ ��λ Hz
    float bullet_speed; //�ӵ����� ��λ m/s
} ext_shoot_data_t;

//�������ݽ�����Ϣ��0x0301������Ƶ�ʣ����� 10Hz
typedef __packed struct
{

    uint16_t data_cmd_id;
    uint16_t send_ID;
    uint16_t receiver_ID;
}ext_student_interactive_header_data_t;


extern ext_game_state_t                ext_game_state;
extern ext_game_result_t               ext_game_result;
extern ext_game_robot_survivors_t      ext_game_robot_survivors;
extern ext_event_data_t                ext_event_data;
extern ext_supply_projectile_action_t  ext_supply_projectile_action;
extern ext_supply_projectile_booking_t ext_supply_projectile_booking;
extern ext_game_robot_state_t          ext_game_robot_state;
extern ext_power_heat_data_t           ext_power_heat_data;
extern ext_game_robot_pos_t            ext_game_robot_pos;
extern ext_buff_musk_t                 ext_buff_musk;
extern aerial_robot_energy_t           aerial_robot_energy;
extern ext_robot_hurt_t                ext_robot_hurt;
extern ext_shoot_data_t                ext_shoot_data;
extern ext_student_interactive_header_data_t			      ext_student_interactive_header_data;



void JudgeConnection_Init(UART_HandleTypeDef *huart);
void Judge_IDLECallback(UART_HandleTypeDef *huart);
void JudgeTransmit(void);
void JudgeReceive(void);
void Judge_Receive_Data_Processing(uint8_t SOF, uint16_t CmdID);

/*--------------------------------------------------У�麯��--------------------------------------------------*/
unsigned char Get_CRC8_Check_Sum(unsigned char *pchMessage,unsigned int dwLength,unsigned char ucCRC8);
unsigned int Verify_CRC8_Check_Sum(unsigned char *pchMessage, unsigned int dwLength);
void Append_CRC8_Check_Sum(unsigned char *pchMessage, unsigned int dwLength);
uint16_t Get_CRC16_Check_Sum(uint8_t *pchMessage,uint32_t dwLength,uint16_t wCRC);
uint32_t Verify_CRC16_Check_Sum(uint8_t *pchMessage, uint32_t dwLength);
void Append_CRC16_Check_Sum(uint8_t * pchMessage,uint32_t dwLength);
/*--------------------------------------------------У�麯��--------------------------------------------------*/

#endif
