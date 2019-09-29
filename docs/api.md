# QXWZ_SDK API 介绍

##  int start_uart(char* uart,char* file,uart_cb uart_rsp)
|**参数**                           |**描述**|
|:-----                             |:----|
|uart                               |串口信息，采用哪个串口和MC110M连接，该参数就采用哪个串口，例如"uart3"|
|file                               |表示高精度定位输出数据文件，高精度定位输出除了在屏幕显示以外，还保存在该文件中，确保该文件可读可写，例如"/qxwz_report.txt"|
|uart_rsp                           |表示串口输出回调函数指针，SDK将通过该回调接口上报原始GGA数据给用户。用户通过该接口获取到原始GGA后，需要将该数据上报给千寻服务器，获取差分数据下发。格式参考qxwz_api.h头文件|
|return                             |0 : 成功; 其他 : 失败|



函数功能：启动读取串口数据

   

##  int stop_uart(void)
|**参数**                           |**描述**|
|:-----                             |:----|
|无                                 |     |
|return                             |0 : 成功; 其他 : 失败|


函数功能：停止读取串口数据


   
##  int write_to_uart(void* data,int len)
|**参数**                           |**描述**|
|:-----                             |:----|
|data                               |表示需要写入串口数据，即为将差分RTCM二进制数据流通过串口写入M110M，进行差分纠偏处理|
|len                                |表示需要写入串口数据长度|
|return                             |0 : 成功; 其他 : 失败|


函数功能：写入串口数据



   
##  int qxwz_printf(const char *fmt, ...)
|**参数**                           |**描述**|
|:-----                             |:----|
|fmt                                |表示格式化字符串|
|return                             |0 : 成功; -1 : 失败|


函数功能：打印千寻log函数


   
##  qxwz_s32_t qxwz_setting(const qxwz_usr_config_t* config, qxwz_bool_t isRealtime)
|**参数**                           |**描述**|
|:-----                             |:----|
|config                             |qxwz_usr_config_t结构体指针，用来保存用户账号信息，应指向一个静态存储区，生命周期与程序同始终|
|isRealtime                         |表示时间模式，如果可以提供系统时间也就是绝对时间，参数isRealtime设置为TRUE，
   如果不能提供系统时间，可以提供相对时间给SDK，参数isRealtime设置为FALSE|
|return                             |0 : 成功; -1 : 失败|

 
函数功能：配置用户账号信息

   
##  qxwz_s32_t qxwz_start(qxwz_data_response_t * data_rsp, qxwz_status_response_t * status_rsp)
|**参数**                           |**描述**|
|:-----                             |:----|
|data_rsp                           |表示数据回调指针结构体|
|status_rsp                         |表示状态码回调指针结构体|
|return                             |0 : 成功; -1 : 失败|


函数功能：用户启动SDK服务



   
##  qxwz_s32_t qxwz_tick(qxwz_u32_t system_time)
|**参数**                           |**描述**|
|:-----                             |:----|
|system_time                        |可以是UTC时间（计于1970年1月1号0零时），也可以是相对时间节拍，一节拍对应1秒时间|
|return                             | >0 : 运行中; 0 : 状态机停止运行|


函数功能：SDK驱动函数，用户可将之置于线程环境或while(1)中



   
##  qxwz_s32_t qxwz_send_data(const void *data, qxwz_u32_t size, qxwz_udata_type_e type)
|**参数**                           |**描述**|
|:-----                             |:----|
|data                               |无符号指针，指向需要传递数据的首地址|
|len                                |表示传递数据长度|
|type                               |type表示数据类型，一般客户仅支持UDATA_GGA|
|return                             |0 : 成功; -1 : 失败|

函数功能： 传递数据给千寻服务器
   
##  qxwz_void_t qxwz_stop(void)
|**参数**                           |**描述**|
|:-----                             |:----|
|无                                 |     |
|return                             |无|


函数功能：停止/释放SDK资源


   
##  qxwz_void_t qxwz_release(void)
|**参数**                           |**描述**|
|:-----                             |:----|
|无                                 |     |
|return                             |无|


函数功能：释放用户账号信息


   
##  const qxwz_account_info* getqxwzAccount(void)
|**参数**                           |**描述**|
|:-----                             |:----|
|无                                 |     |
|return                             |返回qxwz_account_info结构体指针，指向sdk保存的用户账号信息|


函数功能：获取用户账户信息

   
