/*
  Receive LoRA - send MQTT message to mqtt.tago.io
  Code Based on Heltec WiFi Kit and PubSubClient Examples.
*/
#include "heltec.h" 
#include "Arduino.h"
#include <stdio.h>  
#include "WiFi.h"        
#include <PubSubClient.h>  


#define BAND 433E6  //you can set band here directly,e.g. 868E6,915E6
String rssi = "RSSI --";
String packSize = "--";
String packet ;


/* Tópico MQTT para o qual o ESP32 enviará os dados */
#define MQTT_PUB_TOPIC "ENV"
/* Tópico MQTT do qual o ESP32 receberá dados */
#define MQTT_SUB_TOPIC "INCB_ESP32_MQTT_sub"
/* Substitua aqui por um identificador único para você. 
   Caso seu identificador coincidir com o de alguem no 
   mesmo broker MQTT, um dos dois terá a conexão derrubada. */         
#define MQTT_DTOKEN "ddde2f73-9e4a-4bad-a540-a24b48cf6465" 

const char* ssid_wifi = "APARTAMENTO103";  
/*  WI-FI network password */
const char* password_wifi = "a1b2c3d4e5";     
/* Objeto para conexão a Internet (Wi-Fi) */
WiFiClient espClient;     
/* Endereço do broker MQTT que o módulo deve se conectar */
const char* broker_mqtt = "mqtt.tago.io"; 
/* Porta do broker MQTT que deve ser utilizada (padrão: 1883) */
int broker_port = 1883;                     
/* Objeto para conexão com broker MQTT */
PubSubClient MQTT(espClient); 
/* 
* Prototypes 
*/
void init_wifi(void);
void init_MQTT(void);
void connect_MQTT(void);
void connect_wifi(void);
void verify_wifi_connection(void);
void mqtt_callback(char* topic, byte* payload, unsigned int length); 
/* Função: inicaliza conexão wi-fi
 * Parâmetros: nenhum
 * Retorno: nenhum 
 */
void init_wifi(void) 
{
    delay(10);
    Serial.println("------ WI-FI -----");
    Serial.print("Tentando se conectar a seguinte rede wi-fi: ");
    Serial.println(ssid_wifi);
    Serial.println("Aguarde");    
    connect_wifi();
}
/* Função: Configura endereço do broker e porta para conexão com broker MQTT
 * Parâmetros: nenhum
 * Retorno: nenhum 
 */
void init_MQTT(void)
{
    MQTT.setServer(broker_mqtt, broker_port);
    /* Informa que todo dado que chegar do broker pelo tópico definido em MQTT_SUB_TOPIC
       Irá fazer a função mqtt_callback ser chamada (e essa, por sua vez, irá escrever
       os dados recebidos no Serial Monitor */
    MQTT.setCallback(mqtt_callback);
}
/* Função: conecta-se ao broker MQTT (se não há conexão já ativa)
 * Parâmetros: nenhum
 * Retorno: nenhum 
 */
void connect_MQTT(void) 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao seguinte broker MQTT: ");
        Serial.println(broker_mqtt);
        
        if (MQTT.connect(MQTT_DTOKEN, "MakeABlock", MQTT_DTOKEN)) 
        {
            Serial.println("Conexao ao broker MQTT feita com sucesso!");
            /* Após conexão, se subescreve no tópico definido por MQTT_SUB_TOPIC.
               Dessa forma, torna possível receber informações do broker por 
               este tópico. */
            MQTT.subscribe(MQTT_SUB_TOPIC);
        } 
        else 
        {
            Serial.println("Falha ao se conectar ao broker MQTT.");
            Serial.println("Nova tentativa em 2s...");
            delay(2000);
        }
    }
}
/* Função: connect to wifi network
 * Parâmetros: nenhum
 * Retorno: nenhum 
 */
void connect_wifi(void) 
{
    /* If module is already connected to a WI-FI, there is nothing to do here. 
       If there isn't WI-FI connection established, a WI-FI connection is done */
    if (WiFi.status() == WL_CONNECTED)
        return;
        
    WiFi.begin(ssid_wifi, password_wifi);
    
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
  
    Serial.println();
    Serial.print("Successfully connected to WI-FI network: ");
    Serial.println(ssid_wifi);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}
/* Função: verifica se há conexão wi-fi ativa (e conecta-se caso não haja)
 * Parâmetros: nenhum
 * Retorno: nenhum 
 */
void verify_wifi_connection(void)
{
    connect_wifi(); 
}
/*Função: função de callback 
*         esta função é chamada toda vez que chegam dados
*         pelo tópico definido em MQTT_SUB_TOPIC
* Parâmetros: nenhum
* Retorno: nenhum
*/
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg_broker;
    char c;
    /* Obtem a string do payload (dados) recebido */
    for(int i = 0; i < length; i++) 
    {
       c = (char)payload[i];
       msg_broker += c;
    }
    Serial.print("[MQTT] Dados recebidos do broker: ");        
    Serial.println(msg_broker);        
}

void LoRaData(){
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  Heltec.display->drawString(0 , 15 , "Received "+ packSize + " bytes");
  Heltec.display->drawStringMaxWidth(0 , 26 , 128, packet);
  Heltec.display->drawString(0, 0, rssi);  
  Heltec.display->display();
}

void cbk(int packetSize) {
  packet ="";
  packSize = String(packetSize,DEC);
  for (int i = 0; i < packetSize; i++) { packet += (char) LoRa.read(); }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
  LoRaData();
}

void setup() { 
   //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
  //
  LoRa.setSpreadingFactor(8);
    // put in standby mode
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(4);
  LoRa.setSyncWord(0x12);   //0x34
  LoRa.setPreambleLength(8);
  
  Heltec.display->init();
  Heltec.display->flipScreenVertically();  
  Heltec.display->setFont(ArialMT_Plain_10);

  init_wifi();
  init_MQTT();
  connect_MQTT();

  delay(1500);
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "Heltec.LoRa Initial success!");
  Heltec.display->drawString(0, 10, "Wait for incoming data...");
  Heltec.display->display();
  delay(1000);
  //LoRa.onReceive(cbk);
  LoRa.receive();
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) { cbk(packetSize);
    Serial.println(packet);   
    char dados_mqtt[100] = {0};
    verify_wifi_connection();
    connect_MQTT();
    sprintf(dados_mqtt, "%s", packet);
    MQTT.publish(MQTT_PUB_TOPIC, dados_mqtt); 
    }
  MQTT.loop();
  delay(10);
}
