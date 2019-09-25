#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h>
#include <rtthread.h>
#include <rthw.h>
#include <stdarg.h>
#include <sys/socket.h>
#include "qxwz_socket.h"
#include "qxwz_types.h"

#include <dfs.h>
#include <dfs_fs.h>
#include <dfs_posix.h>

#include <dfs_poll.h>
#include <dfs_select.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       2048
#define BUFFER_SIZE             2048
#define THREAD_TIMESLICE        5

typedef enum {
    SOC_FAIL = -1,
    SOC_OK = 0,
    SOC_BLOCK
}SOC_STATUS;

static soc_cb_t g_soc_funtion;
static host_info_t g_host;
static qxwz_s8_t g_soc_buf[BUFFER_SIZE] = {0};
static qxwz_s32_t g_soc;

static rt_thread_t pid_recv = RT_NULL;
static rt_thread_t pid_con = RT_NULL;

static void soc_connect_cb(void *param)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    qxwz_s32_t ret = -1;
    qxwz_s32_t soc = g_soc;
    qxwz_s8_t port[10] = {0};

    sprintf(port,"%d",g_host.port);
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;    /* Allow IPv4*/
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    rt_kprintf("soc=%d, host = %s\n",soc,g_host.hostName);

    ret = getaddrinfo(g_host.hostName, port, &hints, &result);
    if (ret == -1) {
        rt_kprintf("dns fail\n");
        perror("dns");
        g_soc_funtion.cb_status(soc,SOC_FAIL);
        g_soc_funtion.cb_connect(soc);
        freeaddrinfo(result);       
        return;
    }
    
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        rt_kprintf("rp = %p\n",rp);
        if (connect(soc, rp->ai_addr, rp->ai_addrlen) != -1){
            g_soc_funtion.cb_status(soc,SOC_OK);
            g_soc_funtion.cb_connect(soc);
            freeaddrinfo(result);
            return;
         }
    }
    
    perror("connect");

    if(NULL == rp){
        rt_kprintf("no invalid host\n");
        g_soc_funtion.cb_status(soc,SOC_FAIL);
    }
    freeaddrinfo(result);
    return;
}

static void soc_recv_cb_1(void *param)
{
    qxwz_s32_t ret = 0;
    qxwz_s32_t soc = g_soc;
    struct timeval timeout = {2,0};

    setsockopt(soc,SOL_SOCKET,SO_RCVTIMEO, (char *)&timeout,sizeof(struct timeval)); 
    rt_kprintf("%s,%d,soc=%d\n",__func__,__LINE__,soc);
    while(1){
        memset(g_soc_buf,0,sizeof(g_soc_buf));
        ret = recv(soc, g_soc_buf, sizeof(g_soc_buf), MSG_WAITALL);
        if(ret < 0){
            rt_kprintf("%s,%d\n",__func__,__LINE__);
            perror("recv");
            g_soc_funtion.cb_status(soc,SOC_FAIL);
            break;
        }else if(ret == 0){
            rt_kprintf("FIN received[%s,%d]\n",__func__,__LINE__);
            g_soc_funtion.cb_status(soc,SOC_FAIL);
            break;
        }else{
            g_soc_funtion.cb_recv(soc,g_soc_buf, ret);
            return;
        }
    }
}

static void soc_recv_cb_2(void *param)
{
    struct timeval timeout = {2,0};
    qxwz_s32_t counter,ret = 0;
    qxwz_s32_t soc = g_soc;
    setsockopt(soc,SOL_SOCKET,SO_RCVTIMEO, (char *)&timeout,sizeof(struct timeval));
    rt_kprintf("%s,%d,soc=%d\n",__func__,__LINE__,soc);
    while(1){
        memset(g_soc_buf,0,sizeof(g_soc_buf));
        ret = recv(soc, g_soc_buf, sizeof(g_soc_buf), MSG_WAITALL);
        if(ret < 0){
            counter++;
            if(counter>5){
                g_soc_funtion.cb_status(soc,SOC_FAIL);
                return;
            }
            continue;
        }else if(ret == 0){
            rt_kprintf("FIN received[%s,%d]\n",__func__,__LINE__);
            g_soc_funtion.cb_status(soc,SOC_FAIL);
            return;
        }else{
            g_soc_funtion.cb_recv(soc,g_soc_buf, ret);
        }
    }
}

qxwz_s32_t qxwz_soc_create(qxwz_u32_t nwk_id)
{
    qxwz_s32_t sock_fd;
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd<= 0){
        return -1;
    }
    rt_kprintf("qxwz_soc_create=%d\n",sock_fd);
    g_soc = sock_fd;

    rt_kprintf("%s,%d\n",__func__,__LINE__);
     
   return sock_fd;
}


/**
 * @param[in]  soc: socket number.
 * @param[in]  host: the server info @see host_info_t.
 * @param[in]  cbs: the callbacks for async @see soc_cb_t.
 *
 * @return:
 *   0 if success
 *  -1 if fail
 *  -2 if async occurs
 */
qxwz_s32_t qxwz_soc_connect(qxwz_s32_t soc, host_info_t *host, soc_cb_t *cbs)
{
    g_host = *host;
    g_soc_funtion = *cbs;
    if(8000 == host->port){
        pid_recv = rt_thread_create("soc_recv1",
                            soc_recv_cb_1, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
        }else {
                pid_recv = rt_thread_create("soc_recv2",
                            soc_recv_cb_2, RT_NULL,
                            THREAD_STACK_SIZE,
                            24, THREAD_TIMESLICE);
    }
    if (pid_recv != RT_NULL){
        rt_kprintf("start recv\n");
        rt_thread_startup(pid_recv);
    }
    rt_kprintf("qxwz_soc_connect=%d\n",soc);
    pid_con = rt_thread_create("soc_connect",
                            soc_connect_cb, (void *)&soc,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
                            
   if (pid_con != RT_NULL)
        rt_thread_startup(pid_con);
 
    return -2;
}


/**
 * @param[in]  soc: socket number.
 * @param[in]  buf: the data buffer which will be sent.
 * @param[in]  len: the data buffer length.
 *
 * @return:
 *  >=0 if success
 *   -1 if fail for any reason
 *   -2 if buffer not available, async occurs
 */
qxwz_s32_t qxwz_soc_send(qxwz_s32_t soc, const qxwz_s8_t *buf, qxwz_s32_t len)
{
    return send(soc, buf, len, 0);
}

/**
 * @param[in]  soc: socket number.
 * @param[out] buf: the data buffer which will be filled by received data.
 * @param[in]  len: the maxium lenght of data buffer.
 *
 * @return:
 *  >0 if success
 *  -1 if fail for any reason, or receive the FIN from the server
 *  -2 if no data available, async occurs
 */
qxwz_s32_t qxwz_soc_recv(qxwz_s32_t soc, qxwz_s8_t *buf, qxwz_s32_t len)
{
    return 0;
}

/**
 * @param[in]  soc: socket number.
 *
 * @return:
 *   0 if success
 *  -1 if fail
 *  -2 if async occurs
 */
qxwz_s32_t qxwz_soc_close(qxwz_s32_t soc)
{
    return closesocket(soc);
}

