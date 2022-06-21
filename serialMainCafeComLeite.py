import time
import serial

# Configura a serial e a velocidade de transmissao
ser = serial.Serial("/dev/ttyUSB0", 115200)
# serMotor = serial.Serial("/dev/ttyUSB1", 115200)

var = True

def current_milli_time():
    return round(time.time() * 1000)

while(1):

    # Verifica se o botao foi pressionado
    if var:
        # Envia o caracter L pela serial
        # value = str("u1%d" % 20)
        value = "u1"
        enviado = current_milli_time()
        ser.write("u1".encode())
        # Aguarda reposta
        resposta = ser.readline()
        recebido = current_milli_time()
        print(recebido - enviado)
        # Mostra na tela a resposta enviada
        # pelo Arduino
        resposta = resposta.decode()
        print(resposta)
        # Aguarda 0,5 segundos e reinicia o processo
        time.sleep(0.5)

    else:
        var = not var
