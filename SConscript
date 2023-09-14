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

group = DefineGroup('paho.mqtt', src, depend = [''], CPPPATH = path)
Return('group')

