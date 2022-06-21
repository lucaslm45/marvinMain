from enum import Enum
import this
import paho.mqtt.client as mqtt
import time
from struct import *
# from string import *
broker = "192.168.3.22"
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

posicaox = 0; posicaoy = 0; objetivo1x = 10; objetivo1y = 10; objetivo2x = 10; objetivo2y = 10; objetivo3x = 10; objetivo3y = 10; obstaculoDist = 60; readDistMin = 0

ultrassom1 = 0; ultrassom2 = 0; ultrassom3 = 0; laser1 = 10; laser2 = 20; latitude = 0; longitude = 0; angle = 0; acelX = 0; acelY = 0; acelZ = 0
upU1 = False; upU2 = False; upU3 = False; upL1 = False; upL2 = False; upGl1 = False; upGl2 = False; upGa = False; upAcelX = False; upAcelY = False; upAcelZ = False; upReceive = False

flagCaminhoLivre = False; flagEmRota = False; notBreak = True

def setUpU1():
    global upU1
    upU1 = not upU1

def setUpU2():
    global upU2
    upU2 = not upU2

def setUpU3():
    global upU3
    upU3 = not upU3

def setUpL1():
    global upL1
    upL1 = not upL1

def setUpL2():
    global upL2
    upL2 = not upL2

def setUpGl1():
    global upGl1
    upGl1 = not upGl1

def setUpGl2():
    global upGl2
    upGl2 = not upGl2

def setUpGa():
    global upGa
    upGa = not upGa

def setUpAcelX():
    global upAcelX
    upAcelX = not upAcelX

def setUpAcelY():
    global upAcelY
    upAcelY = not upAcelY

def setUpRecebido():
    global upReceive
    upReceive = not upReceive

def setU1(valor):
    global ultrassom1
    ultrassom1 = valor
    setUpU1()

def setU2(valor):
    global ultrassom2
    ultrassom2 = valor
    setUpU2()
    
def setU3(valor):
    global ultrassom3
    ultrassom3 = valor
    setUpU3()

def setL1(valor):
    global laser1
    laser1 = valor
    setUpL1()

def setL2(valor):
    global laser2
    laser2 = valor
    setUpL2()

def setGl1(valor):
    global latitude
    latitude = valor
    setUpGl1()

def setGl2(valor):
    global longitude
    longitude = valor
    setUpGl2()

def setGa(valor):
    global angle
    angle = valor
    setUpGa()

def setAx(valor):
    global acelX
    acelX = valor
    setUpAcelX()

#AceleracaoY
def setAy(valor):
    global acelY
    acelY = valor
    setUpAcelY()

# def setAz(valor):
#     global longitude
#     longitude = valor

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

def on_message_ga(client, userdata, message):
    msgLida = str(message.payload)
    for i in range(0,len(b)):
        msgLida = msgLida.replace(b[i],"")
    
    setGa(float(msgLida))

def on_message_ax(client, userdata, message):
    msgLida = str(message.payload)
    for i in range(0,len(b)):
        msgLida = msgLida.replace(b[i],"")
    
    setAx(float(msgLida))

def on_message_ay(client, userdata, message):
    msgLida = str(message.payload)
    for i in range(0,len(b)):
        msgLida = msgLida.replace(b[i],"")
    
    setAy(float(msgLida))

def on_message_motor(client, userdata, message):
    setUpRecebido()

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

#Esp32 Sensores
client.message_callback_add('esp32/u1', on_message_u1)
client.message_callback_add('esp32/u2', on_message_u2)
client.message_callback_add('esp32/u3', on_message_u3)
client.message_callback_add('esp32/l1', on_message_l1)
client.message_callback_add('esp32/l2', on_message_l2)
client.message_callback_add('esp32/gl1', on_message_gl1)
client.message_callback_add('esp32/gl2', on_message_gl2)
client.message_callback_add('esp32/ga', on_message_ga)
client.message_callback_add('esp32/ax', on_message_ax)
client.message_callback_add('esp32/ay', on_message_ay)
client.message_callback_add('esp32/motor', on_message_motor)

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
    # waitTime = 0.150
    # Consultando Ultrassons
    client.publish("esp32/rasp", "u1") #ultrassom1
    while(not upU1):{}
    setUpU1()
    client.publish("esp32/rasp", "u3") #ultrassom3
    while(not upU3):{}
    setUpU3()
    
    # # Consultando Lasers
    # client.publish("esp32/rasp", "l1") #laser1
    # time.sleep(waitTime)
    # client.publish("esp32/rasp", "l2") #laser2
    # time.sleep(waitTime)
    # # Consultando GPS
    # client.publish("esp32/rasp", "gl1") #gpsLatitude
    # time.sleep(waitTime)
    # client.publish("esp32/rasp", "gl2") #gpsLongitude
    # time.sleep(waitTime)
    # client.publish("esp32/rasp", "ga") #angulo de rotação do robô
    # time.sleep(waitTime)
    # client.publish("esp32/rasp", "ax") #aceleração em X
    # time.sleep(waitTime)
    # client.publish("esp32/rasp", "ay") #aceleração em Y
    # time.sleep(waitTime)
    # client.publish("esp32/rasp", "az") #aceleração em Z
    # time.sleep(waitTime)

    #Set flatCaminhoLivre
    flagCaminhoLivre = True
    if (ultrassom1 > readDistMin and ultrassom1 < obstaculoDist) or (ultrassom2 > readDistMin and ultrassom2 < obstaculoDist) or (ultrassom3 > readDistMin and ultrassom3 < obstaculoDist):
        flagCaminhoLivre = False 
    elif ((laser1 > readDistMin and laser1 < obstaculoDist) or (laser2 > readDistMin and laser2 < obstaculoDist)):
        flagCaminhoLivre = False 

def checkObstaculo():
    global flagCaminhoLivre
    if not ((ultrassom1 > readDistMin and ultrassom1 < obstaculoDist) or (ultrassom2 > readDistMin and ultrassom2 < obstaculoDist) or 
        (ultrassom3 > readDistMin and ultrassom3 < obstaculoDist) or (laser1 > readDistMin and laser1 < obstaculoDist) or (laser2 > readDistMin and laser2 < obstaculoDist)):
            flagCaminhoLivre = True
    else:
        flagCaminhoLivre = False


    if not flagCaminhoLivre:
        client.publish("esp32m/rasp", "p") #angulo de rotação do robô
        while(not upReceive):{}
        setUpRecebido()

while 1:
    if estadoAtual == estados.waitUser:
        # pega dados da pagina web
        objetivo1x = 10
        estadoAtual = estados.buscaLocal
    elif estadoAtual == estados.buscaLocal:
        # Consultando Ultrassons
        client.publish("esp32/rasp", "u1") #ultrassom1
        while(not upU1):{}
        setUpU1()
        checkObstaculo()
        
        client.publish("esp32/rasp", "u2") #ultrassom3
        while(not upU2):{}
        setUpU2()
        checkObstaculo()

        client.publish("esp32/rasp", "u3") #ultrassom3
        while(not upU3):{}
        setUpU3()
        checkObstaculo()

        client.publish("esp32/rasp", "l1") #laser1
        while(not upL1):{}
        setUpL1()
        checkObstaculo()

        client.publish("esp32/rasp", "l2") #laser2
        while(not upL2):{}
        setUpL2()
        checkObstaculo()


        if(flagCaminhoLivre):
            client.publish("esp32m/rasp", "f") #angulo de rotação do robô
            while(not upReceive):{}
            setUpRecebido()
        #Determinando localização
        # client.publish("esp32/rasp", "gl1") #gpsLatitude
        # while(not upGl1):{}
        # setUpGl1()

        # client.publish("esp32/rasp", "gl2") #gpsLongitude
        # while(not upGl2):{}
        # setUpGl2()

        # #Determinando Angulo
        # client.publish("esp32/rasp", "ga") #angulo de rotação do robô
        # while(not upGa):{}
        # setUpGa()

        # print(ultrassom1)
        # print(ultrassom2)
        # print(ultrassom3)
        print(laser1)
        print(laser2)
        # print(angle)

         
            # estadoAtual = estados.calcRota
            # continue


    #Em rota é a localizacao atual com o angulo aponta para o objetivo?

        print(flagCaminhoLivre)

        # if not flagEmRota or not flagCaminhoLivre:
        #     estadoAtual = estados.calcRota
        # elif flagEmRota:
        #     estadoAtual = estados.segueRota
        # elif estadoAtual == estadoAtual.calcRota:


