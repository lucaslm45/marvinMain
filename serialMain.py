from enum import Enum
from re import A
import time
import serial

# device1 = "/dev/ttyUSB1"
# device2 = "/dev/ttyUSB0"
device1 = "/dev/ttyUSB0"
device2 = "/dev/ttyUSB1"
obstaculoDist = 80+50; readDistMin = 0; correcLaser2 = 5
obstaculoDistLateral = 100+30
obstaculo = 60
obstaculoLateral = 75
speedRobo = 60


ser = serial.Serial(device1, 115200)
sermotor = serial.Serial(device2, 115200)

## End System Topics
class estados(Enum):
    waitUser = 1
    buscaLocal = 2
    consulta = 3
    segueRota = 4
    calcRota = 4

estadoAtual = estados.waitUser

posicaox = 0; posicaoy = 0; objetivo1x = 10; objetivo1y = 10; objetivo2x = 10; objetivo2y = 10; objetivo3x = 10; objetivo3y = 10

ultrassom1 = 0; ultrassom2 = 0; ultrassom3 = 0; laser1 = 0; laser2 = 0; latitude = 0; longitude = 0; angle = 0; acelX = 0; acelY = 0; acelZ = 0
upU1 = False; upU2 = False; upU3 = False; upL1 = False; upL2 = False; upGl1 = False; upGl2 = False; upGa = False; upAcelX = False; upAcelY = False; upAcelZ = False; upReceive = False
mapTamX = 40; mapTamY = 40

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
    var = ser.readline()
    # print(var)
    setU1(float(var))

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

def message_to_ga():
    ser.write("ga".encode())
    setGa(float(ser.readline()))

def message_to_acelX(tempo):
    comando = "ax" + tempo
    # print(comando)
    comprimido = comando.encode()
    # print(comprimido)
    # print(comando)
    ser.write(comando.encode())
    var = ser.readline()
    # print(var)
    setAx(float(var))

def message_to_acelY():
    ser.write("ay".encode())
    var = ser.readline()
    # print(var)
    setAy(float(ser.readline()))

def message_to_gl1(): #latitude
    ser.write("g1".encode())
    var = ser.readline()
    print(var)
    setUpGl1()
    # setGl1(float(ser.readline()))

def message_to_gl2(): #longitude
    ser.write("g2".encode())
    var = ser.readline()
    print(var)
    setUpGl2()

    # setGl2(float(ser.readline()))


def message_to_motor(comando):
    sermotor.write(comando.encode())
    print(comando)
    var = sermotor.readline()
    print (var)

    setUpRecebido()
# def message_to_gl1():

# def message_to_gl2():

# def message_to_ga():

# def message_to_ax():

# def message_to_ay():
    
def current_milli_time():
    return round(time.time() * 1000)

# consulta dados dos sensores e seta se o caminho tá livre
def consultaSensores():
    global flagCaminhoLivre
    flagCaminhoLivre = True

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

# Consultando Lasers
    # message_to_l1()
    # while(not upL1):{}
    # setUpL1()
    # checkObstaculo()

    # message_to_l2()
    # while(not upL2):{}
    # setUpL2()
    # checkObstaculo()

# # Consultando Angulo
#     message_to_ga()
#     while(not upGa):{}
#     setUpGa()

    # message_to_gl1()
    # while(not upGl1):{}
    # setUpGl1()

    # message_to_gl2()
    # while(not upGl2):{}
    # setUpGl2()


    # if not flagCaminhoLivre:
    #     message_to_ga()
    #     while(not upGa):{}
    #     setUpGa()

        #Faz o robo girar, manda o angulo lido para o espMotor

        
def checkObstaculo():
    global flagCaminhoLivre
    if((ultrassom1 >= readDistMin and ultrassom1 <= obstaculoLateral) or (ultrassom2 >= readDistMin and ultrassom2 <= obstaculo) or 
        (ultrassom3 >= readDistMin and ultrassom3 <= obstaculoLateral)):# or (laser1 >= readDistMin and laser1 <= obstaculo) or (laser2 >= (readDistMin + correcLaser2) and laser2 <= obstaculo)):
        flagCaminhoLivre = False
        message_to_motor("p")
        while(not upReceive):{}
        setUpRecebido()
        
        
    # elif((ultrassom1 >= readDistMin and ultrassom1 <= obstaculoDistLateral) or (ultrassom2 >= readDistMin and ultrassom2 <= obstaculoDist) or 
    #     (ultrassom3 >= readDistMin and ultrassom3 <= obstaculoDistLateral)):

    # # elif ((ultrassom2 >= readDistMin and ultrassom2 <= obstaculoDist)):# and (laser1 >= readDistMin and laser1 <= obstaculoDist)):
    #     distancia = min(ultrassom1, ultrassom2)
    #     distancia = min(distancia, ultrassom3)
    #     distancia = distancia*speedRobo/(obstaculoDistLateral)# + laser1)/2 #media central
    #     message_to_motor("v"+str(distancia))
    #     while(not upReceive):{}
    #     setUpRecebido()

def determinaLocal():
    global flagEmRota
    #faz alguma coisa
    flagEmRota = False #or True

def determinaRota():
    global flagEmRota
    #faz alguma coisa
    flagEmRota = True #or False

def setSpeeds():
    message_to_motor("v"+str(speedRobo))
    while(not upReceive):{}
    setUpRecebido()

    value = 1.7
    message_to_motor("3"+str(value))
    while(not upReceive):{}
    setUpRecebido()

    value = 1.7
    message_to_motor("4"+str(value))
    while(not upReceive):{}
    setUpRecebido()

i = 0
setSpeeds()
while 1:
    if estadoAtual == estados.waitUser:
        # pega dados da pagina web
        # objetivo1x = 10
        # objetivo1y = 10
        # objetivo2x = 0
        # objetivo2y = 10
        # objetivo3x = 15
        # objetivo3y = 15

        # setPosicoes()
        estadoAtual = estados.buscaLocal
    elif estadoAtual == estados.buscaLocal:
        #Set flatCaminhoLivre e para robo se obstaculo detectado
        print("Consultando Sensores: " + str(i))
        inicio = current_milli_time()
        consultaSensores()
        fim = current_milli_time()
        # print("Sensores Consultados")
        i+=1

        print("Tempo para leitura de todos os sensores: " + str(fim - inicio) + " ms")
        print("CaminhoLivre: " + str(flagCaminhoLivre))
        

        if flagCaminhoLivre:
            print("enviando para motor")
            message_to_motor("f")
            while(not upReceive):{}
            setUpRecebido()
            print("resposta motor")

        print("Ultrassom1: " + str(ultrassom1))
        print("Ultrassom2: " + str(ultrassom2))
        print("Ultrassom3: " + str(ultrassom3))

        # print("Laser1: " + str(laser1))
        # print("Laser2: " + str(laser2))

        # print("Angulo: " + str(angle))

        if(not flagCaminhoLivre):
            estadoAtual = estados.calcRota


    elif estadoAtual == estados.calcRota:
        consultaSensores()
        print("Ultrassom1: " + str(ultrassom1))
        print("Ultrassom2: " + str(ultrassom2))
        print("Ultrassom3: " + str(ultrassom3))
        distancia = min(ultrassom1, ultrassom2)
        distancia = min(distancia, ultrassom3)

        if(distancia > obstaculoLateral+2):
            estadoAtual = estados.buscaLocal

        # if not flagCaminhoLivre:
        #     break
        # print("Latitude: " + str(latitude))
        # print("Longitude: " + str(longitude))

        # value = abs(600)
        # message_to_motor("d"+str(value))
        # while(not upReceive):{}
        # setUpRecebido()
        # # time.sleep(3)
        # message_to_motor("p")
        # while(not upReceive):{}
        # setUpRecebido()
        # time.sleep(2)

        # value = abs(1100)
        # message_to_motor("e"+str(value))
        # while(not upReceive):{}
        # setUpRecebido()
        # # time.sleep(3)
        # message_to_motor("p")
        # while(not upReceive):{}
        # setUpRecebido()
        # break



            # message_to_acelX("1500")
            # while not upAcelX or flagCaminhoLivre:
            #     consultaSensores()

            # setUpAcelX()
            # message_to_motor("p")
            # while(not upReceive):{}
            # setUpRecebido()

            # estadoAtual = estados.calcRota
            # print("Distancia percorrida 1: " + str(acelX))

            # print("Distancia percorrida 2: %.2f"%acelX)
            # print("Fim")
            # break

    #     if (not flagEmRota) or (not flagCaminhoLivre):
    #         estadoAtual = estados.calcRota
        
    #     elif flagEmRota:
    #         estadoAtual = estados.segueRota

    # elif estadoAtual == estados.calcRota:
    #     while (not flagCaminhoLivre):
    #         consultaSensores()
            
    #     determinaRota()

    #     if flagEmRota:
    #         estadoAtual = estados.segueRota

    # elif estadoAtual == estados.segueRota:
    #     message_to_motor("f")
    #     while(not upReceive):{}
    #     setUpRecebido()

    #     estadoAtual = estados.buscaLocal
