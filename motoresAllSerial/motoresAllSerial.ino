#include <Arduino.h>

// Definição dos Pinos
#define S1M1 14
#define S2M1 27 // S1 = CW (Horario), S2 = CCW (AntiHorario)

#define S1M2 23
#define S2M2 22

#define S1M3 21
#define S2M3 19

#define S1M4 2
#define S2M4 4

// PWM
#define PWM1 32
#define PWM2 33
#define PWM3 25
#define PWM4 26

#define BRAKE 0
#define CW 1  // Horario
#define CCW 2 // AntiHorario

// Motores
#define MOTOR_1 1
#define MOTOR_2 2
#define MOTOR_3 3
#define MOTOR_4 4

#define delaySen 800

// --- Variáveis Globais ---
short usSpeed = 90;      // default motor speed
short usSpeedGiro = 185; // default motor speed Giro
short magicWheel1 = 1;
short magicWheel2 = 1;
short magicWheel3 = 1.8;
short magicWheel4 = 1.8;
float distancia = 0;
String messageTemp = "";
char c;

unsigned short usMotor_Status = BRAKE;

// --- Protótipo das Funções Auxiliares ---
void robot_forward();                                     // Função para movimentar robô para frente
void robot_backward();                                    // Função para movimentar robô para trás
void robot_stop();                                        // Função para parar o robô
void motorGo(uint8_t motor, uint8_t direct, uint8_t pwm); // Function that controls the variables: motor(0 ou 1), direction (cw ou ccw) e pwm (entra 0 e 255);
void stopSeguro();
void robot_right(float angulo);
void robot_left(float angulo);
void setup_wifi();
void reconnect();
void moverRobo(String funcao);
int listaFuncao(String comando);
float tempoDeCurva(float angulo);
long timeStandy = 50000;

unsigned char velocidade = 0x00; // armazena a velocidade dos motores (8 bits)
long inicioLaco;
void setup()
{
    Serial.begin(115200); // Initiates the serial to do the monitoring

    while (!Serial)
    {
    }

    pinMode(PWM1, OUTPUT); // PWM1
    pinMode(PWM2, OUTPUT); // PWM2
    pinMode(PWM3, OUTPUT); // PWM3
    pinMode(PWM4, OUTPUT); // PWM4

    pinMode(S1M1, OUTPUT); // Sentido 1 M1
    pinMode(S2M1, OUTPUT); // Sentido 2 M1

    pinMode(S1M2, OUTPUT); // Sentido 1 M2
    pinMode(S2M2, OUTPUT); // Sentido 2 M2

    pinMode(S1M3, OUTPUT); // Sentido 1 M3
    pinMode(S2M3, OUTPUT); // Sentido 2 M3

    pinMode(S1M4, OUTPUT); // Sentido 1 M4
    pinMode(S2M4, OUTPUT); // Sentido 2 M4
    inicioLaco = millis();
}
void loop()
{
    while(Serial.available() > 0)
    {
      c = Serial.read();
      messageTemp += c;
    }
    if(messageTemp.length() > 0)
    {
      inicioLaco = millis();
      moverRobo(messageTemp);
      messageTemp = "";
    }
    if(millis() - inicioLaco > timeStandy)
    {
      robot_stop();
    }
}

void moverRobo(String funcao)
{
    String retorno = "Erro para " + funcao;
    bool setUp = true;

    switch (listaFuncao(funcao))
    { // decide o comando de acordo com a requisição
    case 1:
        robot_stop();
        break;
    case 2:
        robot_forward();
        break;
    case 3:
        robot_backward();
        break;
    case 4:
        robot_right(funcao.substring(1).toFloat());
        break;
    case 5:
        robot_left(funcao.substring(1).toFloat());
        break;
    default:
        Serial.println("Error motor" + funcao);
        return;
    }
    Serial.println("OK");
}
int listaFuncao(String comando)
{
    String funcao = comando.substring(0, 1);
    if (funcao == "p")
    { // parado
        return 1;
    }
    else if (funcao == "f")
    { // frente
        return 2;
    }
    else if (funcao == "t")
    { // trás
        return 3;
    }
    else if (funcao == "d")
    { // direita
        return 4;
    }
    else if (funcao == "e")
    { // esquerda
        return 5;
    }
    
    return 0; // requisição inválida
}
void robot_forward()
{
    if (usMotor_Status != CW)
    {
        usMotor_Status = CW;
        motorGo(MOTOR_1, usMotor_Status, usSpeed);
        motorGo(MOTOR_2, usMotor_Status, usSpeed);
        motorGo(MOTOR_3, usMotor_Status, usSpeed);
        motorGo(MOTOR_4, usMotor_Status, usSpeed);
    }

} // end robot forward

void robot_backward()
{
    stopSeguro();
    
    usMotor_Status = CCW;
    motorGo(MOTOR_1, usMotor_Status, usSpeed);
    motorGo(MOTOR_2, usMotor_Status, usSpeed);
    motorGo(MOTOR_3, usMotor_Status, usSpeed);
    motorGo(MOTOR_4, usMotor_Status, usSpeed);
    
} // end robot backward
void robot_right(float angulo)
{
    float tempo = tempoDeCurva(angulo);
    long tempoInicio = millis();

    while (tempo > (millis() - tempoInicio))
    {
        usMotor_Status = CCW;
        motorGo(MOTOR_1, usMotor_Status, usSpeedGiro);
        motorGo(MOTOR_2, usMotor_Status, usSpeedGiro);
        usMotor_Status = CW;
        motorGo(MOTOR_3, usMotor_Status, usSpeedGiro);
        motorGo(MOTOR_4, usMotor_Status, usSpeedGiro);
    }
    //Desliga tudo para não dar açoite quando for andar para frente
    stopSeguro();
    //Para o robo andar para frente
    usMotor_Status = CCW;

} // end robot right

void robot_left(float angulo)
{
    float tempo = tempoDeCurva(angulo);
    long tempoInicio = millis();

    while (tempo > (millis() - tempoInicio))
    {
        usMotor_Status = CW;
        motorGo(MOTOR_1, usMotor_Status, usSpeedGiro);
        motorGo(MOTOR_2, usMotor_Status, usSpeedGiro);
        usMotor_Status = CCW;
        motorGo(MOTOR_3, usMotor_Status, usSpeedGiro);
        motorGo(MOTOR_4, usMotor_Status, usSpeedGiro);
    }
    //Desliga tudo para não dar açoite quando for andar para frente
    stopSeguro();
    //Para o robo andar para frente
    usMotor_Status = CCW;
} // end robot backward
void robot_stop()
{
  if (usMotor_Status != BRAKE)
  {
    usMotor_Status = BRAKE;
    motorGo(MOTOR_1, usMotor_Status, 0);
    motorGo(MOTOR_2, usMotor_Status, 0);
    motorGo(MOTOR_3, usMotor_Status, 0);
    motorGo(MOTOR_4, usMotor_Status, 0);
  }

} // end robot stop

void stopSeguro()
{
    robot_stop();
    delay(delaySen);
}

float tempoDeCurva(float angulo)
{
    float multiplicador = 5.25; // valor de exemplo
    return round(angulo * multiplicador);
}

void motorGo(uint8_t motor, uint8_t direct, uint8_t pwm) // Function that controls the variables: motor(0 ou 1), direction (cw ou ccw) e pwm (entra 0 e 255);
{
    if (motor == MOTOR_1)
    {
        pwm *= magicWheel1;
        int s1 = S1M1, s2 = S2M1, pwmMotor = PWM1;

        if (direct == CCW)
        {
            digitalWrite(s1, LOW);
            digitalWrite(s2, HIGH);
        }
        else if (direct == CW)
        {
            digitalWrite(s1, HIGH);
            digitalWrite(s2, LOW);
        }
        else
        {
            digitalWrite(s1, LOW);
            digitalWrite(s2, LOW);
        }

        analogWrite(pwmMotor, pwm);
    }
    else if (motor == MOTOR_2)
    {
        pwm *= magicWheel2;
        int s1 = S1M2, s2 = S2M2, pwmMotor = PWM2;

        if (direct == CCW)
        {
            digitalWrite(s1, LOW);
            digitalWrite(s2, HIGH);
        }
        else if (direct == CW)
        {
            digitalWrite(s1, HIGH);
            digitalWrite(s2, LOW);
        }
        else
        {
            digitalWrite(s1, LOW);
            digitalWrite(s2, LOW);
        }

        analogWrite(pwmMotor, pwm);
    }
    else if (motor == MOTOR_3)
    {
        pwm *= magicWheel3;
        int s1 = S1M3, s2 = S2M3, pwmMotor = PWM3;

        if (direct == CCW)
        {
            digitalWrite(s1, LOW);
            digitalWrite(s2, HIGH);
        }
        else if (direct == CW)
        {
            digitalWrite(s1, HIGH);
            digitalWrite(s2, LOW);
        }
        else
        {
            digitalWrite(s1, LOW);
            digitalWrite(s2, LOW);
        }

        analogWrite(pwmMotor, pwm);
    }
    else if (motor == MOTOR_4)
    {
        pwm *= magicWheel4;
        int s1 = S1M4, s2 = S2M4, pwmMotor = PWM4;

        if (direct == CCW)
        {
            digitalWrite(s1, LOW);
            digitalWrite(s2, HIGH);
        }
        else if (direct == CW)
        {
            digitalWrite(s1, HIGH);
            digitalWrite(s2, LOW);
        }
        else
        {
            digitalWrite(s1, LOW);
            digitalWrite(s2, LOW);
        }

        analogWrite(pwmMotor, pwm);
    }
}
