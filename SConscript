import os
from building import * 

cwd     = GetCurrentDir()
path    = []#[cwd, ]
src     = []#Glob('*.c')

path += [cwd + '/MQTTPacket/src']
src += Glob('MQTTPacket/src/*.c')

path += [cwd + '/MQTTClient-C/src']
src += Glob('MQTTClient-C/src/*.c')

path += [cwd + '/MQTTClient-C/src/RTThread']
src += Glob('MQTTClient-C/src/RTThread/*.c')

CPPDEFINES = ['MQTTCLIENT_PLATFORM_HEADER=MQTTRTThread.h']
CPPDEFINES += ['MQTT_TASK']

group = DefineGroup('paho.mqtt.embedded-c', src, depend = [''], CPPPATH = path, CPPDEFINES = CPPDEFINES)
Return('group')

