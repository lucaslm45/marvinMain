from enum import Enum
import paho.mqtt.client as mqtt
import time
from struct import *
# from string import *
broker = "192.168.137.32"
# broker="test.mosquitto.org"
# publish Integers and floats
port = 1883

b = "b'"

## End System Topics
class estados(Enum):
    waitUser = 1
    buscaLocal = 2
    consulta = 3
    segueRota = 4
    calcRota = 4

msgLida = "lucas"
ultimoEstado = estados.waitUser
estadoAtual = estados.waitUser

posicaox = 0; posicaoy = 0; objetivo1x = 10; objetivo1y = 10; objetivo2x = 10; objetivo2y = 10; objetivo3x = 10; objetivo3y = 10; obstaculoDist = 40; readDistMin = 4

ultrassom1 = 0; ultrassom2 = 0; ultrassom3 = 0; laser1 = 10; laser2 = 20; latitude = 0; longitude = 0; angle = 0; flagCaminhoLivre = False; flagEmRota = False; notBreak = True

def setU1(valor):
    global ultrassom1 
    ultrassom1 = valor

def setU2(valor):
    global ultrassom2
    ultrassom2 = valor
    
def setU3(valor):
    global ultrassom3 
    ultrassom3 = valor

def setL1(valor):
    global laser1
    laser1 = valor

def setL2(valor):
    global laser2 
    laser2 = valor

def setGl1(valor):
    global latitude
    latitude = valor

def setGl2(valor):
    global longitude
    longitude = valor


def on_log(client, userdata, level, buf):
    print(buf)

def on_message(client, userdata, message):
    print(message.topic+" "+str(message.qos)+" "+str(message.payload))

def on_message_u1(client, userdata, message):
    msgLida = str(message.payload)
    for i in range(0,len(b)):
        msgLida = msgLida.replace(b[i],"")
    
    setU1(float(msgLida))

def on_message_u2(client, userdata, message):
    msgLida = str(message.payload)
    for i in range(0,len(b)):
        msgLida = msgLida.replace(b[i],"")

    setU2(float(msgLida))

def on_message_u3(client, userdata, message):
    msgLida = str(message.payload)
    for i in range(0,len(b)):
        msgLida = msgLida.replace(b[i],"")
    
    setU3(float(msgLida))

def on_message_l1(client, userdata, message):
    msgLida = str(message.payload)
    for i in range(0,len(b)):
        msgLida = msgLida.replace(b[i],"")
    
    setL1(float(msgLida))

def on_message_l2(client, userdata, message):
    msgLida = str(message.payload)
    for i in range(0,len(b)):
        msgLida = msgLida.replace(b[i],"")
    
    setL2(float(msgLida))

def on_message_gl1(client, userdata, message):
    msgLida = str(message.payload)
    for i in range(0,len(b)):
        msgLida = msgLida.replace(b[i],"")
    
    setGl1(float(msgLida))

def on_message_gl2(client, userdata, message):
    msgLida = str(message.payload)
    for i in range(0,len(b)):
        msgLida = msgLida.replace(b[i],"")
    
    setGl2(float(msgLida))

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        client.connected_flag = True  # set flag
        print("connected OK")
        client.subscribe(topic)
    else:
        print("Bad connection Returned code=", rc)
        client.loop_stop()

def on_disconnect(client, userdata, rc):
    print("client disconnected ok")

def on_publish(client, userdata, mid):
    # time.sleep(1)
    print("In on_pub callback mid= "  ,mid)

mqtt.Client.connected_flag = False  # create flag in class
client = mqtt.Client("rasp")  # create new instance
# set up callbacks
# client.on_log=on_log #this gives getailed logging
client.on_connect = on_connect
client.on_disconnect = on_disconnect
# client.on_publish = on_publish
# client.on_message = on_message
client.message_callback_add('esp32/u1', on_message_u1)
client.message_callback_add('esp32/u2', on_message_u2)
client.message_callback_add('esp32/u3', on_message_u3)
client.message_callback_add('esp32/l1', on_message_l1)
client.message_callback_add('esp32/l2', on_message_l2)
client.message_callback_add('esp32/gl1', on_message_gl1)
client.message_callback_add('esp32/gl2', on_message_gl2)

topic="esp32/#"

print("connecting to broker ", broker)
client.connect(broker, port)  # establish connection
client.loop_start()  # start loop
# client.loop_forever()
while not client.connected_flag:  # wait in loop until connected
    print("In wait loop")
    time.sleep(1)

# consulta dados dos sensores e seta se o caminho tá livre
def consultaSensores():
    waitTime = 0.150
    # Consultando Ultrassons
    client.publish("esp32/rasp", "u1") #ultrassom1
    time.sleep(waitTime)
    client.publish("esp32/rasp", "u2") #ultrassom2
    time.sleep(waitTime)
    client.publish("esp32/rasp", "u3") #ultrassom3
    time.sleep(waitTime)

    # # Consultando Lasers
    client.publish("esp32/rasp", "l1") #laser1
    time.sleep(waitTime)
    client.publish("esp32/rasp", "l2") #laser2
    time.sleep(waitTime)
    # # Consultando GPS
    client.publish("esp32/rasp", "gl1") #gpsLatitude
    time.sleep(waitTime)
    client.publish("esp32/rasp", "gl2") #gpsLongitude
    time.sleep(waitTime)

    #Set flatCaminhoLivre
    flagCaminhoLivre = True
    if (ultrassom1 > readDistMin and ultrassom1 < obstaculoDist) or (ultrassom2 > readDistMin and ultrassom2 < obstaculoDist) or (ultrassom3 > readDistMin and ultrassom3 < obstaculoDist):
        flagCaminhoLivre = False 
    elif (laser1 > readDistMin and laser1 < obstaculoDist) or (laser2 > readDistMin and laser2 < obstaculoDist):
        flagCaminhoLivre = False 

while 1:
    if estadoAtual == estados.waitUser:
        # pega dados da pagina web
        objetivo1x = 10
        estadoAtual = estados.buscaLocal
    elif estadoAtual == estados.buscaLocal:
        consultaSensores()
        # print("Ultrassom 1")
        print(ultrassom1)
        # print("\t\t")
        # print("Ultrassom 2")
        print(ultrassom2)
        # print("\t\t")
        # print("Ultrassom 3")
        print(ultrassom3)

        # if not flagEmRota or not flagCaminhoLivre:
        #     estadoAtual = estados.calcRota
        # elif flagEmRota:
        #     estadoAtual = estados.segueRota
        # elif estadoAtual == estadoAtual.calcRota:


