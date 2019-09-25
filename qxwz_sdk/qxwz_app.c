#include <rtthread.h>
#include <drv_lcd.h>
#include <dfs_fs.h>
#include <dfs_posix.h>
#include "qxwz_types.h"
#include "qxwz_sdk.h"
#include "qxwz_api.h"

static struct rt_semaphore rx_sem;
static rt_device_t serial;
static int qxwz_stop_flag = 0;

static rt_mutex_t dynamic_mutex = RT_NULL;

static char report_gga[256] = {0};
static char gga[256] = {0};

static uart_cb s_uart_rsp = NULL;

typedef enum  {
    NONE,
    SYNC,
    SYNC1,
    SYNC2,
    SYNC3,
    SYNC4,
    GET_DATA,
    END_CR,
    END_NL
} qxState;

static int print2lcd(char* input_buf);

static int filter_report_gga(char s_gga[],int len)
{
   int i = 0;
   int ret = -1;
   static int j = 0;
   char data = 0;
   static qxState cur_state =NONE;
    for (i = 0; i < len; i++) {
        data = s_gga[i];
        switch (cur_state) {
        case NONE:
             if('$' == data){
                cur_state = SYNC;
                report_gga[j] = data;
                j++;
             }
             break;
        case SYNC:
             if('G' == data){
                cur_state = SYNC1;
                report_gga[j] = data;
                j++;
             }
             break;
        case SYNC1:
             if('N' == data){
                cur_state = SYNC2; 
                report_gga[j] = data;
                j++;
             }
             break;
        case SYNC2:
             if('G' == data){
                cur_state = SYNC3; 
                report_gga[j] = data;
                j++;
             }
             break;
        case SYNC3:
             if('G' == data){
                cur_state = SYNC4; 
                report_gga[j] = data;
                j++;
             }
             break;
        case SYNC4:
             if('A' == data){
                cur_state = GET_DATA; 
                report_gga[j] = data;
                j++;
             }
             break;
        case GET_DATA:
             if('\r' != data){ 
                report_gga[j] = data;
                j++;
                break;
             }
             else{
                cur_state = END_CR; 
             }
        case END_CR:
             if('\r' == data){
                cur_state = END_NL; 
                report_gga[j] = data;
                j++;
             }
             break;
        case END_NL:
             if('\n' == data){
                cur_state = NONE; 
                report_gga[j] = data;
                j=0;
                ret = 0;
             }
             break;
        }
    }
    return ret;
}

static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(&rx_sem);
    return RT_EOK;
}

static void serial_thread_entry(void *parameter)
{
    int  recv,ret = -1;
    int  fd;
    int  ret_w = 0;
    char buffer[512];
    char* before = NULL;
    char* after = NULL;
    char* file = (char*)parameter;
    rt_kprintf("serial_thread_entry\n");
    fd = open(file, O_WRONLY | O_CREAT);
    rt_kprintf("open fd %d\n",fd);
    while (1)
    {
        if(qxwz_stop_flag){
            break;            
        }
        while ((recv = rt_device_read(serial, -1, buffer,sizeof(buffer))) > 0)
        {
            ret = filter_report_gga(buffer,recv);
            if(0 == ret){
                before = strstr(report_gga, "E,");
                after = strstr(before+2, ",");
                if(1 == after-(before+2)){
                    rt_mutex_take(dynamic_mutex, RT_WAITING_FOREVER);
                    memcpy(gga,report_gga,256);
                    rt_mutex_release(dynamic_mutex);
                    s_uart_rsp(gga,strlen(gga));
                    ret_w = write(fd, gga,strlen(gga));
                    print2lcd(gga);
                    rt_sem_take(&rx_sem, RT_WAITING_FOREVER);                            
                }
                ret = -1;
            }
            memset(buffer,0,sizeof(buffer));
        }
    }
    close(fd);
}

static int print2lcd(char* input_buf)
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
    p_lati = strstr(strstr(gga,",")+1,",")+1;
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
    return 0;
}

int start_uart(char* uart,char* file,uart_cb uart_rsp)
{
    rt_err_t ret = RT_EOK;
    if ((NULL == uart)||(NULL == file)||(NULL == uart_rsp))
    {
        rt_kprintf("start_uart para uart or file or uart_rsp is NULL\n");
        return RT_ERROR;
    }
    char* uart_name = uart;
    rt_kprintf("find uart_name %s \n", uart_name);
    serial = rt_device_find(uart_name);
    if (!serial)
    {
        rt_kprintf("find %s failed!\n", uart_name);
        return RT_ERROR;
    }
    s_uart_rsp = uart_rsp;
    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);

    rt_device_open(serial, RT_DEVICE_FLAG_INT_RX);

    rt_device_set_rx_indicate(serial, uart_input);
        
    dynamic_mutex = rt_mutex_create("dmutex", RT_IPC_FLAG_FIFO);
    if (dynamic_mutex == RT_NULL)
    {
        rt_kprintf("create dynamic mutex failed.\n");
        return -1;
    }

    rt_thread_t thread = rt_thread_create("serial", serial_thread_entry, file, 2048, 24, 10);

    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        ret = RT_ERROR;
    }
    return ret;
}

int stop_uart(void)
{
    qxwz_stop_flag = 0;
    return 0;
}

int write_to_uart(void* data, int len)
{
    rt_device_write(serial, 0, data, len);
    return 0;
}

int qxwz_printf(const char *fmt, ...)
{
    char buf[1024] = {0};

    va_list val;
    va_start(val, fmt);
    _vsnprintf(buf, 1024, fmt, val);
    va_end(val);

    rt_kprintf("%s\n", buf);
    return 0;
}
