#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <PubSubClient.h>   
#include <ArduinoJson.h>   
#include <VirtualWire.h> //Libreria RF
#include <SoftwareSerial.h> 

SoftwareSerial mySerial(7,8); // RX, TX
EthernetClient ethernetClient;
PubSubClient mqttClient(ethernetClient);
EthernetUDP Udp;

int rf;
byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x03}; 
char IPsemaforo[] = "172.16.1.255"; 
int puertoSemaforo = 8001;
unsigned int localPort = 8002;
byte mqttFlag = 0;
byte RFFlag = 0;
byte VLCFlag = 0;
byte emergencyFlag = 0;
char JSON_Data_Tx[100];
unsigned long previousMillis = 0;
long interval = 1*1000; 

// ****************************** ThingsBoard login details ******************************
const char* server = "172.16.1.254";           // MQTT Broker (i.e. server)
const int port = 1883;                         // Default MQTT port
 
const char* client_id = "Infraestructura 1";   // Can be anything
const char* username =  "kuswzx9USm0ufFMXBMSm";   // Authentication token here

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
}

void loop()
{
  unsigned long currentMillis = millis();
  renovarDHCP();
  
  if (!mqttClient.connected())
  {
    if (mqttClient.connect(client_id, username, NULL)) Serial.println("Cliente mqtt no conectado");
  }
  
  if(currentMillis - previousMillis > interval) 
  {
    previousMillis = currentMillis; // reset the previous millis, so that it will continue to publish data.
    recibirdato();
    transmisorVLC1();
    UDPSendPacket(emergencyFlag, IPsemaforo, puertoSemaforo);
    create_JSON_Data_Tx(); // Set up the data to be published
    mqttClient.publish(topicToPublish_ATTRIBUTES, JSON_Data_Tx);
    if(mqttFlag == 0) mqttFlag = 1;
    else mqttFlag = 0;
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

void UDPSendPacket(byte data, char remote_IP[], int remote_port)
{
  Udp.beginPacket(remote_IP, remote_port);
  Udp.write(data);
  Udp.endPacket();  
}

void create_JSON_Data_Tx(void)
{
  StaticJsonBuffer<200> JSON_Buffer; // Allocate JSON buffer with 200-byte pool
  JsonObject& JSON_Object = JSON_Buffer.createObject(); // Create JSON object (i.e. document)
  
  // Now populate the JSON document with data
  JSON_Object["MQTT"] = mqttFlag;
  JSON_Object["RF"] = RFFlag;
  JSON_Object["VLC"] = VLCFlag;
  JSON_Object["state"] = emergencyFlag; // Create JSON object named "Temperature", assigned with our temperature data
  
  JSON_Object.printTo(JSON_Data_Tx); // Store the data on global variable
}

void recibirdato(){  // Recibir el dato RF 
    
    uint8_t dato;
    uint8_t datoleng=8;
    //verificamos si hay un dato valido en el RF
    if (vw_get_message(&dato,&datoleng))
    {
        if((char)dato=='1')
        {
            digitalWrite(13, true); //LED on
            rf=5;
            emergencyFlag = 1;
            RFFlag = 1;
            Serial.print("Dato RF recibido: ");
            Serial.println(rf);
            Serial.println("Inicio VLC");
            
            
        }
        else if((char)dato=='2')
        {
            digitalWrite(13, false); //LED off
             Serial.println("apagado");
             rf=0;
             emergencyFlag = 0;
             RFFlag = 0;
        }            
    }
}

void transmisorVLC1(){  //Transmision de Alerta VLC

if (rf==5)
{
    mySerial.println("ALERTA AMBULANCIA"); //Tx VLC hacia Automovil
    VLCFlag = 1;
  }
  else VLCFlag = 0;
}
