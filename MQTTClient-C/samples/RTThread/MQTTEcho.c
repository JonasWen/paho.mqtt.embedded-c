/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* RT-Thread includes. */


/* RT-Thread+TCP includes. */
#include <at_socket.h>
#include <netdev.h>

#define MQTT_TASK 1
#include "MQTTClient.h"

static rt_bool_t _check_network(void)
{
#ifdef RT_USING_NETDEV
    struct netdev *netdev = netdev_default;
    return (netdev && netdev_is_link_up(netdev));
#else
    return RT_TRUE;
#endif
}

void messageArrived(MessageData* data)
{
    printf("Message arrived on topic %.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
        data->message->payloadlen, (char *)data->message->payload);
}

static void prvMQTTEchoTask(void *pvParameters)
{
    /* connect to m2m.eclipse.org, subscribe to a topic, send and receive messages regularly every 1 sec */
    MQTTClient client;
    Network network;
    unsigned char sendbuf[80], readbuf[80];
    int rc = 0, 
        count = 0;

    while(!_check_network())
    {
        rt_thread_mdelay(1000);
    }

    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

    pvParameters = 0;
    NetworkInit(&network);
    MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

    char* address = "iot.eclipse.org";
    if ((rc = NetworkConnect(&network, address, 1883)) != 0)
        printf("Return code from network connect is %d\n", rc);

#if defined(MQTT_TASK)
    if ((rc = MQTTStartTask(&client)) != RT_EOK)
        printf("Return code from start tasks is %d\n", rc);
#endif

    connectData.MQTTVersion = 3;
    connectData.clientID.cstring = "FreeRTOS_sample";

    if ((rc = MQTTConnect(&client, &connectData)) != 0)
        printf("Return code from MQTT connect is %d\n", rc);
    else
        printf("MQTT Connected\n");

    if ((rc = MQTTSubscribe(&client, "FreeRTOS/sample/#", 2, messageArrived)) != 0)
        printf("Return code from MQTT subscribe is %d\n", rc);

    while (++count)
    {
        MQTTMessage message;
        char payload[30];

        message.qos = 1;
        message.retained = 0;
        message.payload = payload;
        sprintf(payload, "message number %d", count);
        message.payloadlen = strlen(payload);

        if ((rc = MQTTPublish(&client, "FreeRTOS/sample/a", &message)) != 0)
            printf("Return code from MQTT publish is %d\n", rc);
#if !defined(MQTT_TASK)
        if ((rc = MQTTYield(&client, 1000)) != 0)
            printf("Return code from yield is %d\n", rc);
#endif
    }

    /* do not return */
}


int vStartMQTTTasks(void)
{
    rt_err_t result = RT_EOK;
    rt_thread_t tid;

    tid = rt_thread_create("mqtt_test", prvMQTTEchoTask, RT_NULL, 2048, 20, 20);

    if(tid != RT_NULL)
    {
        result = rt_thread_startup(tid);
    }

    return result;
}
INIT_APP_EXPORT(vStartMQTTTasks);
/*-----------------------------------------------------------*/
