#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <PubSubClient.h>   
#include <ArduinoJson.h>  
#include <VirtualWire.h> //Libreria RF 
#include <SoftwareSerial.h> 

SoftwareSerial mySerial(6,7); // RX, TX
EthernetClient ethernetClient;
PubSubClient mqttClient(ethernetClient);
EthernetUDP Udp;

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x04}; 
unsigned int localPort = 8001;
int packetSize;
byte mqttFlag = 0;
byte VLCFlag = 0; 
byte emergencyFlag = 0;
char JSON_Data_Tx[150];
unsigned long previousMillis = 0;
long interval = 1*1000; 

// ****************************** ThingsBoard login details ******************************
const char* server = "172.16.1.254";           // MQTT Broker (i.e. server)
const int port = 1883;                         // Default MQTT port
 
const char* client_id = "Infraestructura 4";   // Can be anything
const char* username =  "KogldTdG95ukv9ufxqAY";   // Authentication token here

const char* topicToPublish_DATA = "v1/devices/me/telemetry"; // Topic address to publish to for sending data. 
const char* topicToPublish_ATTRIBUTES = "v1/devices/me/attributes"; // Topic address to publish to for sending attributes. 
// ****************************************************************************************

void setup()
{
  Serial.begin(9600);
  mySerial.begin (9600); 
  while (!Serial) delay(2);
  Ethernet.init(10);
  
  vw_setup(2000);  // velocidad: Bits per segundo
  vw_set_rx_pin(2);    //Pin 2 como entrada del RF
  vw_rx_start();       // Se inicia como receptor    
  
  while (Ethernet.begin(mac) == 0) 
  {
    Serial.println("Fallo al configurar DHCP usando Ethernet.");
    
    if (Ethernet.hardwareStatus() == EthernetNoHardware) 
    {
      Serial.println("No se encontro Ethernet Shield.");
    }
    else if (Ethernet.linkStatus() == LinkOFF) 
    {
      Serial.println("Cable Ethernet no conectado.");
    }
    delay(5000);
  }

  Serial.print("My IP Address: ");
  Serial.println(Ethernet.localIP());
  Udp.begin(localPort);
  mqttClient.setServer(server, port);
  for(int pines=2; pines<=13; pines++)
  {
    pinMode(pines,OUTPUT);
  }
}

void loop()
{
  unsigned long currentMillis = millis();
  renovarDHCP();

  if (!mqttClient.connected())
  {
    Serial.println("Cliente mqtt no conectado");
    mqttClient.connect(client_id, username, NULL); 
  }
  if(currentMillis - previousMillis > interval) 
  {
    previousMillis = currentMillis; // reset the previous millis, so that it will continue to publish data.
    packetSize = Udp.parsePacket();
    if (packetSize)
    {
      byte aux = UDPReceivePacket();
      if(aux!=3)
      {
        emergencyFlag = VLCFlag = aux;
      }
    }
    if (VLCFlag >= 1) transmisorVLC1();  
    if(mqttFlag == 0) mqttFlag = 1;
    else mqttFlag = 0;
    create_JSON_Data_Tx(); // Set up the data to be published
    mqttClient.publish(topicToPublish_ATTRIBUTES, JSON_Data_Tx);
  }
  mqttClient.loop();    
}

void renovarDHCP()
{
  switch (Ethernet.maintain()) 
  {
    case 1:
      //renewed fail
      Serial.println("Error en la renovación");
      break;

    case 2:
      //renewed success
      Serial.println("renovacion exitosa");
      //print your local IP address:
      Serial.print("La direccion IP es: ");
      Serial.println(Ethernet.localIP());
      break;

    case 3:
      //rebind fail
      Serial.println("Error: Revinculacion Fallida");
      break;

    case 4:
      //rebind success
      Serial.println("Revinculacion exitosa");
      //print your local IP address:
      Serial.print("la direccion IP es: ");
      Serial.println(Ethernet.localIP());
      break;

    default:
      //nothing happened
      break;
  }  
}

byte UDPReceivePacket()
{
    byte bufferRead [1]; 
    Udp.read(bufferRead,1);
    byte data = bufferRead[0];
    //Serial.println(data);
    return data;    
}

void create_JSON_Data_Tx(void)
{
  StaticJsonBuffer<200> JSON_Buffer; // Allocate JSON buffer with 200-byte pool
  JsonObject& JSON_Object = JSON_Buffer.createObject(); // Create JSON object (i.e. document)
  
  // Now populate the JSON document with data
  JSON_Object["MQTT"] = mqttFlag;
  JSON_Object["state"] = emergencyFlag;
  JSON_Object["VLC"] = VLCFlag;
  
  JSON_Object.printTo(JSON_Data_Tx); 
}

void transmisorVLC1()
{
  Serial.println(1);
  if (VLCFlag>=1)
  {
    Serial.println(2);
    mySerial.println("ALERTA AMBULANCIA"); //Tx VLC hacia Automovil
  }
}  
