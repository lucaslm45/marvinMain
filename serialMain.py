from enum import Enum
from os import PRIO_USER
import time
import serial

ser = serial.Serial("/dev/ttyUSB0", 115200)
# sermotor = serial.Serial("/dev/ttyUSB1", 115200)

## End System Topics
class estados(Enum):
    waitUser = 1
    buscaLocal = 2
    consulta = 3
    segueRota = 4
    calcRota = 4

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

def message_to_u1():
    ser.write("u1".encode())
    setU1(float(ser.readline()))

def message_to_u2():
    ser.write("u2".encode())
    setU2(float(ser.readline()))

def message_to_u3():
    ser.write("u3".encode())
    setU3(float(ser.readline()))

def message_to_l1():
    ser.write("l1".encode())
    setL1(float(ser.readline()))

def message_to_l2():
    ser.write("l2".encode())
    setL2(float(ser.readline()))

# def message_to_gl1():

# def message_to_gl2():

# def message_to_ga():

# def message_to_ax():

# def message_to_ay():
    
def current_milli_time():
    return round(time.time() * 1000)

# consulta dados dos sensores e seta se o caminho tá livre
def consultaSensores():
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
        # client.publish("esp32m/rasp", "p") #angulo de rotação do robô
        # while(not upReceive):{}
        setUpRecebido()

while 1:
    if estadoAtual == estados.waitUser:
        # pega dados da pagina web
        objetivo1x = 10
        estadoAtual = estados.buscaLocal
    elif estadoAtual == estados.buscaLocal:
        # Consultando Ultrassons
        message_to_u1()
        while(not upU1):{}
        setUpU1()
        checkObstaculo()
        
        message_to_u2()
        while(not upU2):{}
        setUpU2()
        checkObstaculo()

        message_to_u3()
        while(not upU3):{}
        setUpU3()
        checkObstaculo()

        message_to_l1()
        while(not upL1):{}
        setUpL1()
        checkObstaculo()

        message_to_l2()
        while(not upL2):{}
        setUpL2()
        checkObstaculo()
        # client.publish("esp32/rasp", "u2") #ultrassom3
        # while(not upU2):{}
        # setUpU2()
        # checkObstaculo()

        # client.publish("esp32/rasp", "u3") #ultrassom3
        # while(not upU3):{}
        # setUpU3()
        # checkObstaculo()

        # client.publish("esp32/rasp", "l1") #laser1
        # while(not upL1):{}
        # setUpL1()
        # checkObstaculo()

        # client.publish("esp32/rasp", "l2") #laser2
        # while(not upL2):{}
        # setUpL2()
        # checkObstaculo()


        # if(flagCaminhoLivre):
        #     client.publish("esp32m/rasp", "f") #angulo de rotação do robô
        #     while(not upReceive):{}
        #     setUpRecebido()
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
        time.sleep(.20)
        print(laser1)
        print(laser2)
        print("")

        # print(angle)

         
            # estadoAtual = estados.calcRota
            # continue


    #Em rota é a localizacao atual com o angulo aponta para o objetivo?

        # print(flagCaminhoLivre)

        # if not flagEmRota or not flagCaminhoLivre:
        #     estadoAtual = estados.calcRota
        # elif flagEmRota:
        #     estadoAtual = estados.segueRota
        # elif estadoAtual == estadoAtual.calcRota:


