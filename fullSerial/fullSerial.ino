#include <Ultrasonic.h>
#include "Adafruit_VL53L0X.h"
#include <MPU6050_tockn.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

#define GPS_RX 17
#define GPS_TX 5
#define GPS_Serial_Baud 115200

TinyGPS gps;

SoftwareSerial gpsSerial(GPS_RX, GPS_TX);

int status;
MPU6050 mpu6050(Wire);

// Limite de aceleração para objeto parado
#define STOP_OFFSET 0.1

// ----- Variáveis globais -----
float distancia = 0;
String messageTemp = "";
char c;
// Variavel para armazenar o tempo entre cada amostra
unsigned long t_amostra = 0;
// Variaveis para verificar se o objeto está parado
unsigned long t_parado = 0;
bool parado = true;
// Variavel para guardar a distancia deslocada
float distx = 0;
long timer = 0;

bool newData = false;
unsigned long chars;

// ----- Funções -----
void setID();
float calculo_trapezio(float dist, float acel, unsigned long tempo);
void ultrasound_reading();
void definirFuncao(String funcao);
int listaFuncao(String funcao);

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
    Serial.println(F("Concluido to VL53L0X 1"));
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
    Serial.println(F("Concluido to VL53L0X 2"));
}
/*
 * Calcula em tempo real a distância percorrida pela regra do trapézio
 * Parâmetros:
 *  - dist - distância que está sendo calculada
 *  - acel - aceleração medida atualmente
 *  - tempo - tempo em microssegundos gasto desde a ultima medição
 */
float calculo_trapezio(float dist, float acel, unsigned long tempo)
{
    // Armazena o ultimo valor de aceleração e velocidade dentro da própria
    // função declarando a variável como static (no 1º momento ela será 0)
    static float last_acel = 0.0;
    static float last_vel = 0.0;

    float vel;
    // Converte o tempo para um valor em segundos
    float t = (float)tempo / 1000000.0;

    // Se o movimento acabou de começar, reinicia a velocidade e a aceleração
    if (dist == 0.0)
    {
        last_vel = 0.0;
        last_acel = 0.0;
    }

    // Calculo utilizando a regra do trapézio
    // Soma a velocidade anterior
    vel = last_vel + (last_acel + acel) * t / 2.0;

    dist = dist + (last_vel + vel) * t / 2.0;

    // Atualiza os valores antigos de aceleração e velocidade
    last_acel = acel;
    last_vel = vel;

    return dist;
}
void setup()
{
    Serial.begin(115200);
    gpsSerial.begin(GPS_Serial_Baud);
    // Serial1.begin(9600);
    while (!Serial)
    {
    }
    while (!gpsSerial)
    {
    }
    Wire.begin();
    mpu6050.begin();
    mpu6050.calcGyroOffsets(true);
    /*while (MPU.begin() < 0)
    {
        Serial.print("Falha de inicialização, confira as conexões. Status: ");
        Serial.println(status);
    }*/
    /*while (!Serial1)
    {
    }*/

    pinMode(SHT_LOX1, OUTPUT);
    pinMode(SHT_LOX2, OUTPUT);

    Serial.println(F("Shutdown pins inited..."));

    digitalWrite(SHT_LOX1, LOW);
    digitalWrite(SHT_LOX2, LOW);

    Serial.println(F("Both in reset mode...(pins are low)"));
    Serial.println(F("Starting..."));

    // função de configuração para novo endereço i2c
    setID();

    Serial.println(("OK"));
}

void loop()
{
    while (Serial.available() > 0)
    {
        c = Serial.read();
        messageTemp += c;
    }
    if (messageTemp.length() > 0)
    {
        definirFuncao(messageTemp);
        messageTemp = "";
    }
}

// void readGPS()
// {
//     for (unsigned long start = millis(); millis() - start < 1000;)
//     {
//         while (gpsSerial.available())
//         {
//             char c = gpsSerial.read();
//             // Serial.write(c); //apague o comentario para mostrar os dados crus
//             if (gps.encode(c)) // Atribui true para newData caso novos dados sejam recebidos
//                 newData = true;
//         }
//     }
//     if (newData)
//     {
//         unsigned long age;
//         gps.f_get_position(&flat, &flon, &age);
//     }
// }

void definirFuncao(String funcao)
{
    String retorno = "Erro para " + funcao;
    bool setUp = true;

    switch (listaFuncao(funcao))
    { // decide qual sensor ler de acordo com a requisição
    // grava na variável distancia o valor de leitura de qualquer sensor
    case 1:
        distancia = ultrasonic1.read(CM);
        break;
    case 2:
        distancia = ultrasonic2.read(CM);
        break;
    case 3:
        distancia = ultrasonic3.read(CM);
        break;
    case 4:
        lox1.rangingTest(&measure1, false);
        distancia = measure1.RangeStatus != 4 ? measure1.RangeMilliMeter / 10 : 666;
        break;
    case 5:
        lox2.rangingTest(&measure2, false);
        distancia = measure2.RangeStatus != 4 ? measure2.RangeMilliMeter / 10 : 666;
        break;
    case 6: // latitude
        float flat, flon;
        unsigned long age;
        for (unsigned long start = millis(); millis() - start < 1000;)
        {
            while (gpsSerial.available())
            {
                char c = gpsSerial.read();
                // Serial.write(c); //apague o comentario para mostrar os dados crus
                if (gps.encode(c)) // Atribui true para newData caso novos dados sejam recebidos
                    newData = true;
            }
        }
        if (newData)
        {
            gps.f_get_position(&flat, &flon, &age);
        }
        distancia = flat == TinyGPS::GPS_INVALID_F_ANGLE ? 666.0 : flat, 6;
        // distancia = flat == TinyGPS::GPS_INVALID_F_ANGLE ? 666.0 : flat, 6;
        break;
    case 7: // longitude
        //readGPS();
        //float flon = 0;
        distancia = flon == TinyGPS::GPS_INVALID_F_ANGLE ? 666.0 : flon, 6;
        break;
    case 8: // angulo
        mpu6050.update();
        distancia = mpu6050.getAngleZ();
        break;
    case 9:
        // Anda pra frente por x tempo, x = funcao.substring(1).toFloat()
        distancia = andaParaFrente(funcao.substring(2).toFloat());
        // distancia = ultrasonic2.read(CM);
        break;
    case 10:
        distancia = ultrasonic2.read(CM);
        break;
    case 11:
        distancia = ultrasonic2.read(CM);
        break;
    default:
        Serial.println("Sensor Erro " + funcao);
        return;
    }
    // O retorno por terminal precisa ser obrigatoriamente com println, print nao serve
    Serial.println(distancia, 6);
}

int listaFuncao(String comando)
{
    String funcao = comando.substring(0, 2);

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
    else if (funcao == "g1")
    { // gps latitude
        return 6;
    }
    else if (funcao == "g2")
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

float andaParaFrente(float tempo)
{
    long tempoInicio = millis();
    float distancia = 0;

    t_amostra = micros();
    t_parado = 0;

    parado = true;
    distx = 0;
    timer = 0;

    while ((millis() - tempoInicio) < tempo)
    {
        mpu6050.update();

        float aux_acx = mpu6050.getAccY();
        aux_acx *= 9.81;
        /*if (fabs(aux_acx) < STOP_OFFSET && abs(long(millis() - t_parado)) > 50)
        {
            parado = true;

            t_amostra = micros();
        }
        else if (fabs(aux_acx) >= STOP_OFFSET)
        {
            t_parado = millis();
            parado = false;
        }

        // ----- Calculo da distancia deslocada -----
        // Se está parado
        if (parado)
        {
            /*if (distx != 0.0)
            {
                // A função retorna o valor em metros,
                // então multiplica por 100 para converter para centímetros
                distancia += distx*100;
            }
            distx = 0.0;*/
        /*}
        // Está se movendo
        else
        {*/
        // Calcula o tempo percorrido
        t_amostra = micros() - t_amostra;

        distx = calculo_trapezio(distx, aux_acx, t_amostra);

        t_amostra = micros();
        // }
        //}
    }
    // distancia = 2000.20;
    return distx * 100;
}
