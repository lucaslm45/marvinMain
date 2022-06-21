#include <WiFi.h>
#include <PubSubClient.h>

// Replace the next variables with your SSID/Password combination
//const char *ssid = "Jordana - NOVA FIBRA";
//const char *password = "ox7td34w";

const char *ssid = "Marvin";
const char *password = "marvin2605";

// Add your MQTT Broker IP address, example:
//const char *mqtt_server = "192.168.3.22";
const char *mqtt_server = "192.168.4.3";
int port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];

unsigned long previousMillis = 0;
unsigned long interval = 30000;
unsigned long currentMillis;

#define delaySen 800

// Definição dos Pinos
#define S1M1 14
#define S2M1 27 // S1 = CW (Horario), S2 = CCW (AntiHorario)

//#define S1M2 12
//#define S2M2 13

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

// --- Variáveis Globais ---
short usSpeed = 85;     // default motor speed
short usSpeedGiro = 185; // default motor speed Giro
short magicWheel1 = 70/usSpeed;
short magicWheel2 = 70/usSpeed;
short magicWheel3 = (usSpeed + 30)/usSpeed;
short magicWheel4 = (usSpeed + 30)/usSpeed;

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
void callback(char *topic, byte *message, unsigned int length);
void definirFuncao(String funcao);
void moverRobo(String funcao);
int listaFuncao(String comando);
float tempoDeCurva(float angulo);

unsigned char velocidade = 0x00; // armazena a velocidade dos motores (8 bits)
void setup()
{
    Serial.begin(9600); // Initiates the serial to do the monitoring

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

    setup_wifi();
    client.setServer(mqtt_server, port);
    client.setCallback(callback);
}
void setup_wifi()
{
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(50);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}
void callback(char *topic, byte *message, unsigned int length)
{
    Serial.print("Mensagem chegou no tópico: ");
    Serial.print(topic);
    Serial.print(". Mensagem: ");
    String messageTemp;

    for (int i = 0; i < length; i++)
    {
        Serial.print((char)message[i]);
        messageTemp += (char)message[i];
    }
    Serial.println();

    // Verifica se o tópico recebido é o esp32m/rasp
    if (String(topic) == "esp32m/rasp")
    {
        Serial.print("Comando solicitado: ");
        moverRobo(messageTemp);
    }
}
void reconnect() {
  // Loop until we're reconnected
  Serial.println(client.state());
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32m/rasp");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  Serial.print("Saiu: ");
  Serial.println(client.state());
}
void loop()
{
    if (!client.connected())
    {
        reconnect();
    }
    currentMillis = millis();
    // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
    if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval))
    {
        Serial.print(millis());
        Serial.println("Reconnecting to WiFi...");
        WiFi.disconnect();
        WiFi.reconnect();
        previousMillis = currentMillis;
    }
    client.loop();
}

void moverRobo(String funcao)
{
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
        Serial.println(funcao.substring(1).toFloat());
        robot_right(funcao.substring(1).toFloat());
        break;
    case 5:
        Serial.println(funcao.substring(1).toFloat());
        robot_left(funcao.substring(1).toFloat());
        break;
    default:
        Serial.println("requisição inválida");
        break;
    }
    client.publish("esp32/motor", "chegou");
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
    Serial.println("Forward");
    if(usMotor_Status != CW)
    {
      stopSeguro();
    
      usMotor_Status = CW;
      motorGo(MOTOR_1, usMotor_Status, usSpeed);
      motorGo(MOTOR_2, usMotor_Status, usSpeed);
      motorGo(MOTOR_3, usMotor_Status, usSpeed);
      motorGo(MOTOR_4, usMotor_Status, usSpeed);
    }

} // end robot forward

void robot_backward()
{
    Serial.println("Reverse");
    if(usMotor_Status != CCW)
    {
      stopSeguro();
      usMotor_Status = CCW;
      motorGo(MOTOR_1, usMotor_Status, usSpeed);
      motorGo(MOTOR_2, usMotor_Status, usSpeed);
      motorGo(MOTOR_3, usMotor_Status, usSpeed);
      motorGo(MOTOR_4, usMotor_Status, usSpeed);
    }
} // end robot backward
void robot_right(float angulo)
{
    Serial.println("Right");
    stopSeguro();
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

} // end robot right

void robot_left(float angulo)
{
    Serial.println("Left");
    stopSeguro();
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
} // end robot backward
void robot_stop()
{
    Serial.println("Stop");
    usMotor_Status = BRAKE;
    motorGo(MOTOR_1, usMotor_Status, 0);
    motorGo(MOTOR_2, usMotor_Status, 0);
    motorGo(MOTOR_3, usMotor_Status, 0);
    motorGo(MOTOR_4, usMotor_Status, 0);

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
        pwm += magicWheel1;
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
        pwm += magicWheel2;
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
        pwm += magicWheel3;
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
        pwm += magicWheel4;
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
