/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com
*********/

#include <WiFi.h>
#include <PubSubClient.h>
#include <Ultrasonic.h>
#include "Adafruit_VL53L0X.h"

// Replace the next variables with your SSID/Password combination
/*const char* ssid = "Ctba_Lucas";
const char* password = "1020oeku";*/
const char *ssid = "Jordana - NOVA FIBRA";
const char *password = "ox7td34w";

// Add your MQTT Broker IP address, example:
// const char* mqtt_server = "192.168.1.144";
// const char* mqtt_server = "mqtt://192.168.137.32";
const char *mqtt_server = "192.168.3.22";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

float distancia = 0;

void setID();

unsigned long previousMillis = 0;
unsigned long interval = 30000;
unsigned long currentMillis;

/*******************************************************************************************************/
/*                     CONFIGURAÇÃO HC-SR04                           */
/*********************************************************************************************************/
#define echo1 14
#define trig1 27

#define echo2 12
#define trig2 13

#define echo3 33
#define trig3 32

Ultrasonic ultrasonic1(trig1, echo1); // criação dos objetos sensores ultrassom
Ultrasonic ultrasonic2(trig2, echo2);
Ultrasonic ultrasonic3(trig3, echo3);

/*******************************************************************************************************/
/*                     CONFIGURAÇÃO Laser                           */
/*********************************************************************************************************/
// definição do novo endereço i2c do sensor
#define LOX1_ADDRESS 0x30
#define LOX2_ADDRESS 0x31

// definição dos pinos xshunt
#define SHT_LOX1 19
#define SHT_LOX2 18

// criação dos objetos sensores a laser
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();

// variável para armazenar as medições dos sensores
VL53L0X_RangingMeasurementData_t measure1;
VL53L0X_RangingMeasurementData_t measure2;

/*
  Os sensores vl53l0x tem como padrão o endereço i2c 0x29, ao usar mais de um sensor é preciso alterar este valor.
  Pra isso é utilizado os pinos XSHUNT de cada sensor, quando ele estiver HIGH liga o sensor e quando estiver em LOW vai desliga-lo.
  1 - Desligue todos os sensores colocando os XSHUNT em LOW
  2 - Coloque XSHUNT1 em HIGH e use a função lox.begin(new_i2c_address) para escolher o novo endereço i2c do sensor
  3 - Coloque XSHUNT2 em HIGH e use a função lox.begin(new_i2c_address) para escolher o novo endereço i2c do sensor
  4 - Coloque XSHUNT2 em LOW
  * Para escolher o endereço i2c, o valor deve ser abaixo de 0x7F, exceto 0x29
*/
void setID()
{
    // desliga todos os sensores
    digitalWrite(SHT_LOX1, LOW);
    digitalWrite(SHT_LOX2, LOW);
    delay(10);

    // liga os dois sensores
    digitalWrite(SHT_LOX1, HIGH);
    digitalWrite(SHT_LOX2, HIGH);
    delay(10);

    // liga o sensor 1 e desliga o sensor 2
    digitalWrite(SHT_LOX1, HIGH);
    digitalWrite(SHT_LOX2, LOW);

    // configura o novo endereço i2c no sensor 1, avisa caso haja falha
    if (!lox1.begin(LOX1_ADDRESS))
    {
        Serial.println(F("Failed to boot first VL53L0X"));
        while (1)
        {
        };
    }
    delay(10);

    // agora ativa também o sensor 2 (isso é possível porquê o sensor 1 já foi configurado com endereço diferente)
    digitalWrite(SHT_LOX2, HIGH);
    delay(10);

    // configura novo endereço para o sensor 2
    if (!lox2.begin(LOX2_ADDRESS))
    {
        Serial.println(F("Failed to boot second VL53L0X"));
        while (1)
        {
        };
    }
}

void ultrasound_reading();
void definirFuncao(String funcao);
int listaFuncao(String funcao);

void setup()
{
    Serial.begin(9600);
    while (!Serial)
    {
    }
    pinMode(SHT_LOX1, OUTPUT);
    pinMode(SHT_LOX2, OUTPUT);

    Serial.println(F("Shutdown pins inited..."));

    digitalWrite(SHT_LOX1, LOW);
    digitalWrite(SHT_LOX2, LOW);

    Serial.println(F("Both in reset mode...(pins are low)"));
    Serial.println(F("Starting..."));

    // função de configuração para novo endereço i2c
    setID();
    // default settings
    // (you can also pass in a Wire library object like &Wire2)
    // status = bme.begin();

    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    // pinMode(ledPin, OUTPUT);
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
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *message, unsigned int length)
{
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    String messageTemp;

    for (int i = 0; i < length; i++)
    {
        Serial.print((char)message[i]);
        messageTemp += (char)message[i];
    }
    Serial.println();

    if (String(topic) == "esp32/rasp")
    {
        Serial.print("Changing output to ");
        definirFuncao(messageTemp);
    }
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect("ESP8266Client"))
        {
            Serial.println("connected");
            // Subscribe
            client.subscribe("esp32/rasp");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
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

void definirFuncao(String funcao)
{
    char tempString[8];

    switch (listaFuncao(funcao))
    { // decide qual sensor ler de acordo com a requisição
    // grava na variável distancia o valor de leitura de qualquer sensor
    case 1:
        Serial.println("u1");
        distancia = ultrasonic1.read(CM);
        break;
    case 2:
        Serial.println("u2");
        distancia = ultrasonic2.read(CM);
        break;
    case 3:
        Serial.println("u3");
        distancia = ultrasonic3.read(CM);
        break;
    case 4:
        Serial.println("l1");
        // leitura dos sensores em metros
        distancia = measure1.RangeMilliMeter / 100;
        break;
    case 5:
        Serial.println("l2");
        distancia = measure2.RangeMilliMeter / 100;
        break;
    case 6:
        Serial.println("gl1");
        break;
    case 7:
        Serial.println("gl2");
        break;
    case 8:
        Serial.println("ga");
        distancia = ultrasonic2.read(CM);
        break;
    case 9:
        Serial.println("ax");
        distancia = ultrasonic2.read(CM);
        break;
    case 10:
        Serial.println("ay");
        distancia = ultrasonic2.read(CM);
        break;
    case 11:
        Serial.println("az");
        distancia = ultrasonic2.read(CM);
        break;
    default:
        Serial.println("requisição inválida");
        return;
    }

    if (!(funcao == "gl1" || funcao == "gl2"))
    {
        // converte em string o valor de distância e salva na variável tempString
        dtostrf(distancia, 5, 2, tempString);
    }
    else
    {
        dtostrf(distancia, 8, 4, tempString);
    }
    Serial.print("Distancia: ");
    Serial.println(tempString);

    // define o nome do tópico de retorno concatenando esp32/ com a função desejada
    String topico = "esp32/" + funcao;
    char topicoChar[(topico.length() + 1)];

    // converte o nome do tópico para char e salva na variável topicoChar
    topico.toCharArray(topicoChar, (topico.length() + 1));

    // publica para o raspberry a mensagem tempString no tópico topicoChar
    client.publish(topicoChar, tempString);
}

int listaFuncao(String funcao)
{
    if (funcao == "u1")
    { // ultrassom 1
        return 1;
    }
    else if (funcao == "u2")
    { // ultrassom 2
        return 2;
    }
    else if (funcao == "u3")
    { // ultrassom 3
        return 3;
    }
    else if (funcao == "l1")
    { // laser 1
        return 4;
    }
    else if (funcao == "l2")
    { // laser 2
        return 5;
    }
    else if (funcao == "gl1")
    { // gps latitude
        return 6;
    }
    else if (funcao == "gl2")
    { // gps longitude
        return 7;
    }
    else if (funcao == "ga")
    { // angulo de rotação do robô
        return 8;
    }
    else if (funcao == "ax")
    { // aceleração em X
        return 9;
    }
    else if (funcao == "ay")
    { // aceleração em Y
        return 10;
    }
    else if (funcao == "az")
    { // aceleração em Z
        return 11;
    }
    return 0; // requisição inválida
}