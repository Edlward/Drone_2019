
#define __TASK_TEST_GLOBALS

#include "Task_Test.h"
#include "Func_Chassis.h"

#ifdef MASTER_MODE
#include "Func_GimbalMotor.h"
#include "Func_FricMotor.h"
#include "Func_GYRO.h"
#include "Func_JetsonComm.h"
#endif

#include "Func_Imu_OB.h"

/**
  * @brief  ����1��������
  * @param  unused
  * @retval void
  */
uint8_t temp = 0;
int probe1 = 0,probe2 = 0,probe3,probe4,probe5;
void Task_Test1(void *parameter)
{
    while (1)
    {
	

    }
}

int16_t test_RELAY = 0;
int16_t test_chassis = 0;

/**
  * @brief  ����2��������
  * @param  unused
  * @retval void
  */
void Task_Test2(void *parameter)
{
    while (1)
    {
	#ifdef MASTER_MODE

//        printf("ts %f\t cs %f\t op %d\t",Motor2006_Pitch.TargetSpeed,PersonalGYRO.Gyro_X,Motor2006_Pitch.NeedCurrent);        
//        printf("ta %f\t ca %f\t op %f\r\n",Motor2006_Pitch.TargetAngle,\
//                                                    PersonalGYRO.PitchAngle,\
//                                                    Motor2006_Pitch.PositionPID.Output);        
//        printf("ts %f\t cs %f\t op %d\r\n",Motor2006_Yaw.TargetSpeed,PersonalGYRO.Gyro_Z,Motor2006_Yaw.NeedCurrent);        
//        printf("ta %f\t ca %f\t op %f\r\n",Motor2006_Pitch.TargetAngle,\
//                                                    PersonalGYRO.PitchAngle,\
//                                                    Motor2006_Pitch.PositionPID.Output);        
	#endif
        vTaskDelay(5);
    }
}

#if (RTOS_DEBUG_ENABLE == 1)

/**
  * @brief  ��ȡϵͳ״̬���������ã�
  * @param  unused
  * @retval void
  */
void PrintSystemState(void)
{
    uint32_t TotalRunTime;
    UBaseType_t ArraySize, x;
    TaskStatus_t *StatusArray;

    printf("/********��һ��������uxTaskGetSystemState()��ʹ��**********/\r\n");
    ArraySize = uxTaskGetNumberOfTasks();                         //��ȡϵͳ��������
    StatusArray = pvPortMalloc(ArraySize * sizeof(TaskStatus_t)); //�����ڴ�
    if (StatusArray != NULL)                                      //�ڴ�����ɹ�
    {
        ArraySize = uxTaskGetSystemState((TaskStatus_t *)StatusArray, //������Ϣ�洢����
                                         (UBaseType_t)ArraySize,      //������Ϣ�洢�����С
                                         (uint32_t *)&TotalRunTime);  //����ϵͳ�ܵ�����ʱ��
        printf("TaskName\t\tPriority\t\tTaskNumber\t\t\r\n");
        for (x = 0; x < ArraySize; x++)
        {
            //ͨ�����ڴ�ӡ����ȡ����ϵͳ������й���Ϣ�������������ơ�
            //�������ȼ��������š�
            printf("%s\t\t%d\t\t\t%d\t\t\t\r\n",
                   StatusArray[x].pcTaskName,
                   (int)StatusArray[x].uxCurrentPriority,
                   (int)StatusArray[x].xTaskNumber);
        }
    }
    vPortFree(StatusArray); //�ͷ��ڴ�
    printf("/**************************����***************************/\r\n");
}

/**
  * @brief  ��ȡ������Ϣ���������ã�
  * @param  ������ (char [])
  * @retval void
  */
void PrintTaskInfo(char str[])
{
    TaskHandle_t TaskHandle;
    TaskStatus_t TaskStatus;

    printf("/************�ڶ���������vTaskGetInfo()��ʹ��**************/\r\n");
    TaskHandle = xTaskGetHandle(str); //������������ȡ��������
    //��ȡLED0_Task��������Ϣ
    vTaskGetInfo((TaskHandle_t)TaskHandle,    //������
                 (TaskStatus_t *)&TaskStatus, //������Ϣ�ṹ��
                 (BaseType_t)pdTRUE,          //����ͳ�������ջ��ʷ��Сʣ���С
                 (eTaskState)eInvalid);       //�����Լ���ȡ��������״̬
    //ͨ�����ڴ�ӡ��ָ��������й���Ϣ��
    printf("������:                %s\r\n", TaskStatus.pcTaskName);
    printf("������:              %d\r\n", (int)TaskStatus.xTaskNumber);
    printf("����״̬:              %d\r\n", TaskStatus.eCurrentState);
    printf("����ǰ���ȼ�:        %d\r\n", (int)TaskStatus.uxCurrentPriority);
    printf("��������ȼ�:          %d\r\n", (int)TaskStatus.uxBasePriority);
    printf("�����ջ����ַ:        %#x\r\n", (int)TaskStatus.pxStackBase);
    printf("�����ջ��ʷʣ����Сֵ:%d\r\n", TaskStatus.usStackHighWaterMark);
    printf("/**************************����***************************/\r\n");
}

/**
  * @brief  ��ȡ����״̬���������ã�
  * @param  ������ (char [])
  * @retval void
  */
void PrintTaskState(char str[])
{
    TaskHandle_t TaskHandle;
    eTaskState TaskState;
    char TaskInfo[10];

    printf("/***********������������eTaskGetState()��ʹ��*************/\r\n");
    TaskHandle = xTaskGetHandle(str);      //������������ȡ��������
    TaskState = eTaskGetState(TaskHandle); //��ȡquery_task���������״̬
    memset(TaskInfo, 0, 10);
    switch ((int)TaskState)
    {
    case 0:
        sprintf(TaskInfo, "Running");
        break;
    case 1:
        sprintf(TaskInfo, "Ready");
        break;
    case 2:
        sprintf(TaskInfo, "Suspend");
        break;
    case 3:
        sprintf(TaskInfo, "Delete");
        break;
    case 4:
        sprintf(TaskInfo, "Invalid");
        break;
    }
    printf("����״ֵ̬:%d,��Ӧ��״̬Ϊ:%s\r\n", TaskState, TaskInfo);
    printf("/**************************����**************************/\r\n");
}

/**
  * @brief  ��ӡ�����б��������ã�
  * @param  unused
  * @retval void
  * @note   �ú�����Ҫ���������ջ��500���ɣ�
  */
void PrintTaskListing(void)
{
    char InfoBuffer[1000]; //������Ϣ������

    printf("/*************������������vTaskList()��ʹ��*************/\r\n");
    vTaskList(InfoBuffer);        //��ȡ�����������Ϣ
    printf("%s\r\n", InfoBuffer); //ͨ�����ڴ�ӡ�����������Ϣ
    printf("/**************************����**************************/\r\n");
}

#endif
