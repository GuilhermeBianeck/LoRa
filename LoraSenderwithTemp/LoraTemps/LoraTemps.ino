#include <OneWire.h>
#include <DallasTemperature.h>
#include "Arduino.h"
#include "LoRaWan_APP.h"

#define timetillsleep 500
#define timetillwakeup 30000
static TimerEvent_t sleep;
static TimerEvent_t wakeUp;
uint8_t lowpower=1;

void onSleep()
{
  Serial.printf("Going into lowpower mode, %d ms later wake up.\r\n",timetillwakeup);
  lowpower=1;
  //timetillwakeup ms later wake up;
  TimerSetValue( &wakeUp, timetillwakeup );
  TimerStart( &wakeUp );
}
void onWakeUp()
{
  Serial.printf("Woke up, %d ms later into lowpower mode.\r\n",timetillsleep);
  lowpower=0;
  //timetillsleep ms later into lowpower mode;
  TimerSetValue( &sleep, timetillsleep );
  TimerStart( &sleep );
}
 
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

  Radio.Sleep( );
  TimerInit( &sleep, onSleep );
  TimerInit( &wakeUp, onWakeUp );
  onSleep();

}

uint8_t i=0;

void loop()
{


  if(lowpower){
    lowPowerHandler();
  }

    sensors.requestTemperatures(); // Send the command to get temperatures again to refresh at every transmission
    leitura = sensors.getTempCByIndex(0);

    uint16_t batteryVoltage = getBatteryVoltage();


    txNumber += 0.01;

    char *str_temp;

    String msgT = "";
    msgT = String(leitura, 1);

    memset(txpacket,0, BUFFER_SIZE);

    str_temp = (char*)(msgT).c_str();

    sprintf(txpacket,"%s",str_temp);
    Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));
    Radio.Send( (uint8_t *)txpacket, strlen(txpacket) );
    Serial.print("BatteryVoltage:");
	  Serial.println(batteryVoltage);
    turnOnRGB(1,0);
    delay(500);

}