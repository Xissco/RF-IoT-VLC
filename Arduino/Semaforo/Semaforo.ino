#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <PubSubClient.h>   
#include <ArduinoJson.h>   

EthernetClient ethernetClient;
PubSubClient mqttClient(ethernetClient);
EthernetUDP Udp;

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02}; 
unsigned int localPort = 8001;
int packetSize;
byte mqttFlag = 0;
byte RFFlag = 0;
byte VLCFlag = 0;
byte emergencyFlag = 0;
byte redFlag1 = 0;
byte yellowFlag1 = 0;
byte greenFlag1 = 0;
byte redFlag2 = 0;
byte yellowFlag2 = 0;
byte greenFlag2 = 0;
byte tlState = 0;
byte readFt = 0;
byte contR = 0;
char JSON_Data_Tx[100];
unsigned long previousMillis = 0;
long interval = 1*1000; 
int cont = 0;

// ****************************** ThingsBoard login details ******************************
const char* server = "172.16.1.254";           // MQTT Broker (i.e. server)
const int port = 1883;                         // Default MQTT port
 
const char* client_id = "Semaforo";   // Can be anything
const char* username =  "LodxRSnCktZNrFKLhKm7";   // Authentication token here

const char* topicToPublish_DATA = "v1/devices/me/telemetry"; // Topic address to publish to for sending data. 
const char* topicToPublish_ATTRIBUTES = "v1/devices/me/attributes"; // Topic address to publish to for sending attributes. 
// ****************************************************************************************

void setup()
{
  Serial.begin(115200);
  while (!Serial) delay(2);
  Ethernet.init(10);
    
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
    if(cont>=1  and cont <=60)
    {
      packetSize = Udp.parsePacket();
      if (packetSize)
      {
        emergencyFlag = UDPReceivePacket();
      }
      cont --;
      trafficLightState(3);
      if(mqttFlag == 0) mqttFlag = 1;
      else mqttFlag = 0;
      create_JSON_Data_Tx(); // Set up the data to be published
      mqttClient.publish(topicToPublish_ATTRIBUTES, JSON_Data_Tx);
      if(cont==0)
      { 
        readFt = 3;
        Udp.flush();
      }
      while(readFt >=1) 
      {
        packetSize = Udp.parsePacket();
        if (packetSize) contR = contR + UDPReceivePacket();
        Serial.println(contR);
        readFt--;
        delay(500);
      }
      if(contR > 0)
      {
        cont = 60;
        contR = 0;
      }
      else
      {
        tlState = 0;
        contR = 0;
      }
    }
    else
    {
      packetSize = Udp.parsePacket();
      if (packetSize)
      {
        emergencyFlag = UDPReceivePacket();
        if(emergencyFlag==1) cont = 60;
      }
      tlState++;
      trafficLightState(tlState);
      if(mqttFlag == 0) mqttFlag = 1;
      else mqttFlag = 0;
      create_JSON_Data_Tx(); // Set up the data to be published
      mqttClient.publish(topicToPublish_ATTRIBUTES, JSON_Data_Tx);
      if(tlState>=4) tlState = 0;
    }
 }
  mqttClient.loop();    
}

void renovarDHCP()
{
  switch (Ethernet.maintain()) {
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
    return data;    
}

void trafficLightState(int state)
{
  if (state==1)
  {
    redFlag1 = 1;
    yellowFlag1 = 0;
    greenFlag1 = 0;
    
    redFlag2 = 0;
    yellowFlag2 = 0;
    greenFlag2 = 1;  
  }
  else if (state==2)
  {
    redFlag1 = 1;
    yellowFlag1 = 1;
    greenFlag1 = 0;

    redFlag2 = 0;
    yellowFlag2 = 0;
    greenFlag2 = 1;    
  }
  else if (state==3)
  {
    redFlag1 = 0;
    yellowFlag1 = 0;
    greenFlag1 = 1; 

    redFlag2 = 1;
    yellowFlag2 = 0;
    greenFlag2 = 0; 
  }
  else if (state==4)
  {
    redFlag1 = 0;
    yellowFlag1 = 0;
    greenFlag1 = 1; 

    redFlag2 = 1;
    yellowFlag2 = 1;
    greenFlag2 = 0; 
  }
}

void create_JSON_Data_Tx(void)
{
  StaticJsonBuffer<200> JSON_Buffer; // Allocate JSON buffer with 200-byte pool
  JsonObject& JSON_Object = JSON_Buffer.createObject(); // Create JSON object (i.e. document)
  
  // Now populate the JSON document with data
  JSON_Object["MQTT"] = mqttFlag;
  JSON_Object["RF"] = RFFlag;
  JSON_Object["VLC"] = VLCFlag;
  JSON_Object["state"] = emergencyFlag;
  JSON_Object["red1"] = redFlag1;
  JSON_Object["yellow1"] = yellowFlag1;
  JSON_Object["green1"] = greenFlag1;
  JSON_Object["red2"] = redFlag2;
  JSON_Object["yellow2"] = yellowFlag2;
  JSON_Object["green2"] = greenFlag2;
  JSON_Object["contador"] = cont;
  
  JSON_Object.printTo(JSON_Data_Tx); // Store the data on global variable
}
  
