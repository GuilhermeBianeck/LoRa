#include <OneWire.h>
#include <DallasTemperature.h>
#include "Arduino.h"
#include "LoRaWan_APP.h"
 
#ifndef LoraWan_RGB
#define LoraWan_RGB 0
#endif

#define ONE_WIRE_BUS GPIO5
#define TEMPERATURE_PRECISION 9

#define RF_FREQUENCY                                433000000 // Hz

#define TX_OUTPUT_POWER                             20        // dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       8         // [SF7..SF12]
#define LORA_CODINGRATE                             4         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false
#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];
static RadioEvents_t RadioEvents;

double txNumber;

int16_t rssi,rxSize;


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
int counter = 0;
float leitura;


/* Application port */
uint8_t appPort = 2;

uint8_t confirmedNbTrials = 8;

/* Prepares the payload of the frame */

static void prepareTxFrame( uint8_t port )
{

    // Read temperatures from DS18xxx
  sensors.begin();
  float temp = (sensors.getTempCByIndex(0));
  Serial.print("Content of temp float value from Sensor: = ");
  Serial.print(temp);
  Serial.println (" Degrees Celsius");
  Serial.println();
  delay(100); // Wait for a while before proceeding
 
  unsigned char *tempout;
  tempout = (unsigned char *)(&temp);
	uint16_t batteryVoltage = getBatteryVoltage();

  appDataSize = 6;
	appData[0] = tempout[0];
	appData[1] = tempout[1];
	appData[2] = tempout[2];
	appData[3] = tempout[3];

	appData[4] = (uint8_t)(batteryVoltage>>8);
	appData[5] = (uint8_t)batteryVoltage;
	
	Serial.print("BatteryVoltage:");
	Serial.println(batteryVoltage);
}

void setup() {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
  boardInitMcu();
  sensors.begin();
	Serial.begin(115200);
  delay(200);
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  Serial.print("Temperature for the device 1 (index 0) is: ");
  Serial.print(sensors.getTempCByIndex(0)); 
  Serial.println ("C");

  txNumber=0;
  rssi=0;

  Radio.Init( &RadioEvents );
  Radio.SetChannel( RF_FREQUENCY );
  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                 LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                 LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                 true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 
}

uint8_t i=0;

void loop()
{
    sensors.requestTemperatures(); // Send the command to get temperatures again to refresh at every transmission
    leitura = sensors.getTempCByIndex(0);

    txNumber += 0.01;

    char *str_temp;

    String msgT = "";
    msgT = String(leitura, 1);

    memset(txpacket,0, BUFFER_SIZE);

    str_temp = (char*)(msgT).c_str();

    sprintf(txpacket,"%s",str_temp);
    Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));
    Radio.Send( (uint8_t *)txpacket, strlen(txpacket) );
    turnOnRGB(1,0);
    delay(50000);
    turnOnRGB(1,0);
}