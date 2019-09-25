# QXWZ_SDK API 介绍

头文件qxwz_api.h

   int qxwz_printf(const char *fmt, ...);
   -打印千寻log函数

   int start_uart(char* uart,char* file,uart_cb uart_rsp)
   -启动读取串口数据
   -其参数uart，表示串口信息，采用哪个串口和MC110M连接，该参数就采用哪个串口，例如"uart3"
   -参数file表示高精度定位输出数据文件，高精度定位输出除了在屏幕显示以外，还保存在该文件中，确保该文件可读可写，例如"/qxwz_report.txt"
   -参数uart_rsp表示串口输出回调函数指针，SDK将通过该回调接口上报原始GGA数据给用户。用户通过该接口获取到原始GGA后，需要将该数据上报给千寻服务器，获取差分数据下发。格式参考qxwz_api.h头文件。

   int stop_uart(void);
   -停止读取串口数据
	
   int write_to_uart(void* data,int len);
   -写入串口数据
   -其参数data，表示需要写入串口数据，即为将差分RTCM二进制数据流通过串口写入M110M，进行差分纠偏处理
   -参数len表示需要写入串口数据长度

头文件qxwz_sdk.h

   qxwz_s32_t qxwz_setting(const qxwz_usr_config_t* config, qxwz_bool_t isRealtime)	
   -配置用户账号信息
   -其参数qxwz_usr_config_t* config，希望指向一个静态存储区，生命周期与程序同始终
   -参数isRealtime表示时间模式，如果可以提供系统时间也就是绝对时间，参数isRealtime设置为TRUE，
   如果不能提供系统时间，可以提供相对时间给SDK，参数isRealtime设置为FALSE。

   qxwz_s32_t qxwz_start(qxwz_data_response_t * data_rsp, qxwz_status_response_t * status_rsp)
   -用户启动SDK服务
   -data_rsp中，描述了不同的数据类型qxwz_data_type_e，差分数据用户关心RTCM_TYPE_RAW即可

   qxwz_s32_t qxwz_tick(qxwz_u32_t system_time)
   -SDK驱动函数，用户可将之置于线程环境或while(1)中
   -其参数system_time可以是UTC时间（计于1970年1月1号0零时），也可以是相对时间节拍，一节拍对应1秒时间。

   qxwz_s32_t qxwz_send_data(const void *data, qxwz_u32_t size, qxwz_udata_type_e type)
   -客户通过此函数向SDK注入数据，比如GGA
   -其支持数据类型为qxwz_udata_type_e，一般客户仅支持UDATA_GGA

   qxwz_void_t qxwz_stop(void)
   -停止/释放SDK资源
   qxwz_void_t qxwz_release(void)
   -释放用户账号信息
   
   getqxwzAccount(void)
   获取用户账户信息

   数据回调函数static void receive_iprtcm(qxwz_void_t *rtcm, qxwz_u32_t len, qxwz_data_type_e type)中的qxwz_data_type_e 
   建议选用RTCM_TYPE_RAW
