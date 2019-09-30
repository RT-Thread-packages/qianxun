#include <rtthread.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <rtdbg.h>
#include <board.h>
#include "qxwz_types.h"
#include "qxwz_sdk.h"
#include "qxwz_api.h"
#include <drv_lcd.h>

#define SAMPLE_UART_NAME       "uart3"

static int report_flag = 0;
static char* file = "/qxwz_report.txt";

static rt_mutex_t dynamic_mutex = RT_NULL;
static struct rt_semaphore net_ready;

static const qxwz_usr_config_t s_config;
static char s_gga[256] = {0};
static int report_flag;
static int stopflag;

static void report_gga(char *gga, qxwz_u32_t len);
static void print2lcd(char* input_buf);

static void receive_iprtcm(qxwz_void_t *rtcm, qxwz_u32_t len, qxwz_data_type_e type)
{
    static unsigned int rtcmcouter = 0;
    write_to_uart(rtcm,len);
    rtcmcouter++;
    if(rtcmcouter >300){
        stopflag = 1;
    }
}

static void receive_status(qxwz_s32_t status)
{
    rt_kprintf("got rtcm status=%d\n",status);
    if(1007 == status)
    {
        report_flag =1;
        start_uart(SAMPLE_UART_NAME,file,report_gga,print2lcd);
    }
}

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

static const qxwz_usr_config_t s_config = {
    "",
    "",
    "",
    ""
};

static void print2lcd(char* input_buf)
{
    int i = 0;
    char* p_for =NULL;
    char* p_tmp =NULL;
    char* p_lati =NULL;
    char* p_long =NULL;
    char* p_loc_accu =NULL;
    char buf[10];
    char outbuf[100];
    lcd_clear(WHITE);

    lcd_set_color(WHITE, BLACK);

    lcd_show_string(10, 69, 16, input_buf);
    p_lati = strstr(strstr(input_buf,",")+1,",")+1;
    p_tmp = strstr(p_lati,",");
    memset(buf,0,10);
    p_for = p_lati;
    for(i = 0;i<p_tmp-p_lati;i++){
        buf[i] =*p_for;
        p_for++;
    }
    memset(outbuf,0,100);
    sprintf(outbuf,"latitude:%s",buf);
    lcd_show_string(10, 69+16+8+16+24, 16, outbuf);
    p_long = strstr(p_lati,"N,")+2;
    p_tmp = strstr(p_long,",");
    memset(buf,0,10);
    p_for = p_long;
    for(i = 0;i<p_tmp-p_long;i++){
        buf[i] =*p_for;
        p_for++;
    }
    memset(outbuf,0,100);
    sprintf(outbuf,"longitude:%s",buf);
    lcd_show_string(10, 69+8+16+16+24+24, 16, outbuf);
    p_loc_accu = strstr(p_long,"E,")+2;
    p_tmp = strstr(p_loc_accu,",");
    memset(buf,0,10);
    p_for = p_loc_accu;
    for(i = 0;i<p_tmp-p_loc_accu;i++){
        buf[i] =*p_for;
        p_for++;
    }
    memset(outbuf,0,100);
    sprintf(outbuf,"qx_loca_accuracy:%s",buf);
    lcd_show_string(10, 69+8+16+16+24+24+24, 16, outbuf);
}

int qxwz_application(void)
{  
    int result = RT_EOK;
    /* 初始化文件系统 */

    /* 初始化网络连接功能 */

    // The sdk current time
    dynamic_mutex = rt_mutex_create("dmutex", RT_IPC_FLAG_FIFO);
    if (dynamic_mutex == RT_NULL)
    {
        rt_kprintf("create dynamic mutex failed.\n");
        return -1;
    }

    static int s_current_time = 0;
    qxwz_s32_t ret = 0;

    s_current_time = time(NULL);

    qxwz_setting(&s_config,QXWZ_FALSE);
    ret = qxwz_start(&data_res,&status_res);
    if(0 != ret)
        return 0;
    int j = 0;
    while(1){
        if(stopflag){
            stop_uart();
            qxwz_stop();
        }
        ret = qxwz_tick(s_current_time);
        if(ret == 0)
        { 
            break;
        }
        rt_thread_mdelay(1000);
        s_current_time += 1;
        if((report_flag)&&(j>5)){
            rt_mutex_take(dynamic_mutex, RT_WAITING_FOREVER);
            qxwz_send_data(s_gga, strlen(s_gga), UDATA_GGA);
            rt_mutex_release(dynamic_mutex);
            j = 0;
        }
        j++;
    }
    qxwz_release();
    return 0;
}


