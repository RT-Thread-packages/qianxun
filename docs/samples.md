#  QXWZ_SDK 示例程序

## 示例代码讲解

下面讲解 RT-Thread 提供的  QXWZ_SDK 示例代码，功能示例代码如下：

```c
#include <rtthread.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <rtdbg.h>
#include <board.h>
#include "qxwz_types.h"
#include "qxwz_sdk.h"
#include "qxwz_api.h"

/*连接M110M串口信息*/
#define SAMPLE_UART_NAME       "uart3"

/*输出gga信息文件路径*/
static char* file = "/qxwz_report.txt";

/*上报SDK GGA标志*/
static int report_flag = 0;


static rt_mutex_t dynamic_mutex = RT_NULL;
static struct rt_semaphore net_ready;

static const qxwz_usr_config_t s_config;
static char s_gga[256] = {0};
static int report_flag;
static int stopflag;

static void report_gga(char *gga, qxwz_u32_t len);

/*注册到SDK的数据回调函数，sdk会将差分rtcm数据通过该回调函数上报*/
static void receive_iprtcm(qxwz_void_t *rtcm, qxwz_u32_t len, qxwz_data_type_e type)
{
    static unsigned int rtcmcouter = 0;
    /*将上报的差分rtcm数据通过该函数写入M110M芯片进行差分纠偏解算*/
    write_to_uart(rtcm,len);
    rtcmcouter++;
    if(rtcmcouter >300){
        stopflag = 1;
    }
}

/*注册到SDK的状态码回调函数，sdk会将运行状态或错误信息通过该回调函数上报*/
static void receive_status(qxwz_s32_t status)
{
    rt_kprintf("got rtcm status=%d\n",status);
    /*收到1007状态码，证明鉴权通过，可以上报GGA启动差分数据下发*/
    if(1007 == status)
    {
        report_flag =1;
        /*读取串口数据*/
        start_uart(SAMPLE_UART_NAME,file,report_gga);
    }
}

/*GGA上报回调函数，会将串口读取的GGA数据保存在本地*/
static void report_gga(char *gga, qxwz_u32_t len)
{
    rt_mutex_take(dynamic_mutex, RT_WAITING_FOREVER);
    memcpy(s_gga,gga,len);
    rt_mutex_release(dynamic_mutex);
}

qxwz_data_response_t data_res = {
    receive_iprtcm,
    NULL
};

qxwz_status_response_t status_res = {
    receive_status
};

/*用户账号信息*/
static const qxwz_usr_config_t s_config = {
    "",
    "",
    "",
    ""
};

/*应用入口函数*/
int qxwz_appliction(void)
{  
    int result = RT_EOK;
    /* 初始化文件系统 */

    /* 初始化网络连接功能 */

    dynamic_mutex = rt_mutex_create("dmutex", RT_IPC_FLAG_FIFO);
    if (dynamic_mutex == RT_NULL)
    {
        rt_kprintf("create dynamic mutex failed.\n");
        return -1;
    }

    static int s_current_time = 0;
    qxwz_s32_t ret = 0;

    /*获取当前系统时间*/
    s_current_time = time(NULL);

    /*配置用户账号信息*/
    qxwz_setting(&s_config,QXWZ_FALSE);
    /*启动SDK服务，注册数据回调，状态码回调函数到sdk*/
    ret = qxwz_start(&data_res,&status_res);
    if(0 != ret)
        return 0;
    int j = 0;
    while(1){
        if(stopflag){
            /*停止读取串口信息，关闭串口*/
            stop_uart();
            /*停止/释放SDK资源*/
            qxwz_stop();
        }
        /*时间节拍函数，驱动sdk运行*/
        ret = qxwz_tick(s_current_time);
        /*时间节拍函数返回为0，表示sdk处于空转状态，可以退出*/
        if(ret == 0)
        { 
            break;
        }
        rt_thread_mdelay(1000);
        s_current_time += 1;
        if((report_flag)&&(j>5)){
            rt_mutex_take(dynamic_mutex, RT_WAITING_FOREVER);
            /*通过此函数向SDK注入GGA数据*/
            qxwz_send_data(s_gga, strlen(s_gga), UDATA_GGA);
            rt_mutex_release(dynamic_mutex);
            j = 0;
        }
        j++;
    }
    /*释放用户账号信息*/
    qxwz_release();
    return 0;
}

```
