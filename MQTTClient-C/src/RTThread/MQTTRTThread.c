/*******************************************************************************
 * Copyright (c) 2014, 2015 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *    Ian Craggs - convert to FreeRTOS
 *******************************************************************************/

#include "MQTTRTThread.h"

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sal_socket.h>
#include <netdb.h>
#include <sal_netdb.h>
#include <string.h>
#include <sys/errno.h>

int ThreadStart(Thread* thread, void (*fn)(void*), void* arg)
{
    int rc = 0;
    
    rt_thread_t tid = RT_NULL;
    
    tid = rt_thread_create("MQTTTask", fn, arg, 2048, 20, 20);
    
    if(tid != RT_NULL)
    {
        rc = rt_thread_startup(tid);
    }
    
    return rc;
}


void MutexInit(Mutex* mutex)
{
    mutex->mutex = rt_mutex_create("mqtt", RT_IPC_FLAG_FIFO);
}

int MutexLock(Mutex* mutex)
{
    return rt_mutex_take(mutex->mutex, RT_WAITING_FOREVER);
}

int MutexUnlock(Mutex* mutex)
{
    return rt_mutex_release(mutex->mutex);
}


void TimerCountdownMS(Timer* timer, unsigned int timeout_ms)
{
    timer->xTicksToWait = timeout_ms;   /* convert milliseconds to ticks */
    timer->xTimeOut = rt_tick_get();    /* Record the time at which this function was entered. */
}


void TimerCountdown(Timer* timer, unsigned int timeout) 
{
    TimerCountdownMS(timer, timeout * 1000);
}


int TimerLeftMS(Timer* timer) 
{
    timer->xTicksToWait -= rt_tick_get() - timer->xTimeOut;   /* convert milliseconds to ticks */
    timer->xTimeOut = rt_tick_get();    /* Record the time at which this function was entered. */
    
    return (timer->xTicksToWait <= 0) ? 0 : (timer->xTicksToWait);
}


char TimerIsExpired(Timer* timer)
{
    timer->xTicksToWait -= rt_tick_get() - timer->xTimeOut;   /* convert milliseconds to ticks */
    timer->xTimeOut = rt_tick_get();    /* Record the time at which this function was entered. */
    
    return (timer->xTicksToWait <= 0) ? RT_TRUE : RT_FALSE;
}


void TimerInit(Timer* timer)
{
    timer->xTimeOut = 0;
    timer->xTicksToWait = 0;
}


int RTThread_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
    int nread;
    int nleft = len;
    int fd = n->my_socket;
    unsigned char *ptr; 
    ptr = buffer;
    
    struct timeval tv = {
        timeout_ms / 1000, 
        (timeout_ms % 1000) * 1000
    };
    
    if (tv.tv_sec < 0 || (tv.tv_sec == 0 && tv.tv_usec <= 0)) {
        tv.tv_sec = 0;
        tv.tv_usec = 100;
    }
    
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
    
    while (nleft > 0) {
        nread = recv(fd, ptr, nleft, 0);

        if ((nread == -1) && (errno == EAGAIN))
        {
            nread = 0;
        }

        if (nread < 0) {
            return -1;
        } else if (nread == 0) {
            break;
        }

        nleft -= nread;
        ptr += nread;
    }
    return len - nleft;
}


int RTThread_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
    int fd = n->my_socket;
    
    struct timeval tv = {
        timeout_ms / 1000, 
        (timeout_ms % 1000) * 1000
    };
    
    if (tv.tv_sec < 0 || (tv.tv_sec == 0 && tv.tv_usec <= 0)) {
        tv.tv_sec = 0;
        tv.tv_usec = 100;
    }
    
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(struct timeval));
    
    return send(fd, buffer, len, 0);
}


void RTThread_disconnect(Network* n)
{
    closesocket(n->my_socket);
}


void NetworkInit(Network* n)
{
    n->my_socket = -1;
    n->disconnect = RTThread_disconnect;
    n->mqttread = RTThread_read;
    n->mqttwrite = RTThread_write;
    
}


int NetworkConnect(Network* n, char* host, int port)
{
    int fd, ret = -1;
    struct addrinfo hints, *addr_list, *cur;
    char servname[6];
    
    /* Do name resolution with both IPv6 and IPv4 */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    rt_snprintf(servname, sizeof(servname), "%d", port);
    
    if (getaddrinfo(host, servname, &hints, &addr_list) != 0) {
        return ret;
    }
    
    for (cur = addr_list; cur != NULL; cur = cur->ai_next) {
        n->my_socket = fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (fd < 0) {
            ret = -1;
            continue;
        }

        if (connect(fd, cur->ai_addr, cur->ai_addrlen) == 0) {
            ret = fd;
            break;
        }
        
        closesocket(fd);
        
        ret = -1;
    }
    
    freeaddrinfo(addr_list);
    
    return ret;
}


#if 0
int NetworkConnectTLS(Network *n, char* addr, int port, SlSockSecureFiles_t* certificates, unsigned char sec_method, unsigned int cipher, char server_verify)
{
    return 0;
}
#endif
