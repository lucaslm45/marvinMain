/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Ultrasonic.h>

// Replace the next variables with your SSID/Password combination
const char* ssid = "Ctba_Lucas";
const char* password = "1020oeku";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
//const char* mqtt_server = "mqtt://192.168.137.32";
const char* mqtt_server = "192.168.137.32";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

float distancia = 0;

// LED Pin
const int ledPin = 2;

unsigned long previousMillis = 0;
unsigned long interval = 30000;
/*******************************************************************************************************/
/*                     CONFIGURAÇÃO HC-SR04                           */
/*********************************************************************************************************/
#define echo1 14
#define trig1 27

#define echo2 12
#define trig2 13 

#define echo3 33 
#define trig3 32

Ultrasonic ultrasonic1(trig1, echo1);                                 // criação dos objetos sensores ultrassom
Ultrasonic ultrasonic2(trig2, echo2);
Ultrasonic ultrasonic3(trig3, echo3);

void ultrasound_reading();
void definirFuncao(String funcao);
int listaFuncao(String funcao);

void setup() {
  Serial.begin(115200);
  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  //status = bme.begin();  

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

// pinMode(ledPin, OUTPUT);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
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

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/rasp");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
 /* unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }*/
  client.loop();
}

void definirFuncao(String funcao){
  char tempString[8];
  
  switch(listaFuncao(funcao)){ //decide qual sensor ler de acordo com a requisição
    //grava na variável distancia o valor de leitura de qualquer sensor
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
      distancia = ultrasonic2.read(CM);
      break;
    case 5:
      Serial.println("l2");
      distancia = ultrasonic2.read(CM);
      break;
    case 6:
      Serial.println("gl1");
      distancia = ultrasonic2.read(CM);
      break;
    case 7:
      Serial.println("gl2");
      distancia = ultrasonic2.read(CM);
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
  
  //converte em string o valor de distância e salva na variável tempString
  dtostrf(distancia, 5, 2, tempString);
  Serial.print("Distancia: ");
  Serial.println(tempString);
  
  //define o nome do tópico de retorno concatenando esp32/ com a função desejada
  String topico = "esp32/" + funcao;
  char topicoChar[(topico.length() + 1)];

  //converte o nome do tópico para char e salva na variável topicoChar
  topico.toCharArray(topicoChar, (topico.length() + 1));
  
  //publica para o raspberry a mensagem tempString no tópico topicoChar
  client.publish(topicoChar, tempString);
}

int listaFuncao(String funcao){
  if(funcao == "u1"){ //ultrassom 1
    return 1;
  } else if(funcao == "u2"){ //ultrassom 2
    return 2;
  } else if(funcao == "u3"){ //ultrassom 3
    return 3;
  } else if(funcao == "l1"){ //laser 1
    return 4;
  } else if(funcao == "l2"){ //laser 2
    return 5;
  } else if(funcao == "gl1"){ //gps latitude
    return 6;
  } else if(funcao == "gl2"){ //gps longitude
    return 7;
  } else if(funcao == "ga"){ //angulo de rotação do robô
    return 8;
  } else if(funcao == "ax"){ //aceleração em X
    return 9;
  } else if(funcao == "ay"){ //aceleração em Y
    return 10;
  } else if(funcao == "az"){ //aceleração em Z
    return 11;
  }
  return 0; //requisição inválida
}
