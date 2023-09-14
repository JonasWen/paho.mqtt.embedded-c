#if !defined(MQTTRTThread_H)
#define MQTTRTThread_H

#include <rtthread.h>

typedef struct Timer 
{
    int xTicksToWait;
    int xTimeOut;
} Timer;

typedef struct Network Network;

struct Network
{
    int my_socket;
    int (*mqttread) (Network*, unsigned char*, int, int);
    int (*mqttwrite) (Network*, unsigned char*, int, int);
    void (*disconnect) (Network*);
};

void TimerInit(Timer*);
char TimerIsExpired(Timer*);
void TimerCountdownMS(Timer*, unsigned int);
void TimerCountdown(Timer*, unsigned int);
int TimerLeftMS(Timer*);

typedef struct Mutex
{
    rt_mutex_t mutex;
} Mutex;

void MutexInit(Mutex*);
int MutexLock(Mutex*);
int MutexUnlock(Mutex*);

typedef struct Thread
{
    int task;
} Thread;

int ThreadStart(Thread*, void (*fn)(void*), void* arg);

int FreeRTOS_read(Network*, unsigned char*, int, int);
int FreeRTOS_write(Network*, unsigned char*, int, int);
void FreeRTOS_disconnect(Network*);

void NetworkInit(Network*);
int NetworkConnect(Network*, char*, int);

#endif
