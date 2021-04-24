#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>   
#include <ArduinoJson.h>     

EthernetClient ethernetClient;
PubSubClient mqttClient(ethernetClient);

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x03};

// ****************************** ThingsBoard login details ******************************
const char* server = "172.16.1.254";           // MQTT Broker (i.e. server)
const int port = 1883;                         // Default MQTT port
 
const char* client_id = "Infraestructura 1";   // Can be anything
const char* username =  "kuswzx9USm0ufFMXBMSm";   // Authentication token here

const char* topicToPublish_DATA = "v1/devices/me/telemetry"; // Topic address to publish to for sending data. 
const char* topicToPublish_ATTRIBUTES = "v1/devices/me/attributes"; // Topic address to publish to for sending attributes. 
// ****************************************************************************************

// Data variables
int cont = 0;
int termino = 1;
char JSON_Data_Tx[100]; // Used to store the generated data in JSON
char JSON_Data_Rx[100]; 

unsigned long previousMillis = 0; // Last time updated
long interval = 1*1000;            // Interval at which to publish data (60 sec)

void setup() 
{  
  Ethernet.init(10);
  Serial.begin(9600);
  while (!Serial) {;}
  
  Serial.println("Initialize Ethernet with DHCP:");
  
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
  Serial.print("Conexion Ethernet exitosa, Direccion IP: ");
  Serial.println(Ethernet.localIP());
  mqttClient.setServer(server, port); // Configure the server adress and port.
  mqttClient.setCallback(on_message);
  mqttClient.subscribe("v1/devices/me/rpc/response/+");
  create_JSON_Data_Rx(); // Set up the data to be published
}

void loop() 
{
  unsigned long currentMillis = millis(); // Store the time since the board started
  renovarDHCP();
  // If not connected to MQTT, we will reconnect.
  if (!mqttClient.connected())
  { 
    reconnect();
  }
  
  if(currentMillis - previousMillis > interval) 
  {
    previousMillis = currentMillis; // reset the previous millis, so that it will continue to publish data.
    workflow(); // Workflow to use data
  }

  mqttClient.loop(); // Called to maintain connection to server
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

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) 
  {
    if (mqttClient.connect(client_id, username, NULL)) 
    {
      mqttClient.subscribe("v1/devices/me/rpc/response/+");
      workflow(); // publish data, once connected
    } 
    else 
    {
      delay(5000);  // Wait 5 seconds before retrying
    }
  }
}

void workflow(void)
{
  cont = cont + termino;
  if (cont == 100 or cont == 0) termino = termino * -1;
  Serial.println(cont);
  send_data();         // Publish data to ThingsBoard
  recive_data();
}

void send_data(void)
{
  create_JSON_Data_Tx(); // Set up the data to be published
  mqttClient.publish(topicToPublish_ATTRIBUTES, JSON_Data_Tx); // Publish JSON data to ThingsBoard
}

void recive_data(void)
{
  mqttClient.publish("v1/devices/me/rpc/request/1",JSON_Data_Rx);
}

void create_JSON_Data_Tx(void)
{
  StaticJsonBuffer<200> JSON_Buffer; // Allocate JSON buffer with 200-byte pool
  JsonObject& JSON_Object = JSON_Buffer.createObject(); // Create JSON object (i.e. document)
  
  // Now populate the JSON document with data
  JSON_Object["state"] = cont; // Create JSON object named "Temperature", assigned with our temperature data
  JSON_Object["Humedad"] = 90;
  
  JSON_Object.printTo(JSON_Data_Tx); // Store the data on global variable
}

void create_JSON_Data_Rx(void)
{
  StaticJsonBuffer<200> JSON_Buffer; // Allocate JSON buffer with 200-byte pool
  JsonObject& JSON_Object = JSON_Buffer.createObject(); // Create JSON object (i.e. document)
  
  // Now populate the JSON document with data
  JSON_Object["method"] = "getState"; // Create JSON object named "Temperature", assigned with our temperature data
  JSON_Object["params"] = "";
  
  JSON_Object.printTo(JSON_Data_Rx); // Store the data on global variable
}

void on_message(const char* topic, byte* payload, unsigned int length) 
{
  StaticJsonBuffer<200> jsonBuffer;
  char json[length + 1];
  strncpy (json, (char*)payload, length);
  json[length] = '\0';

  Serial.print("Message: ");
  Serial.println(json);

  JsonObject& root = jsonBuffer.parseObject(json);
  long time = root["params"];
  Serial.println(time);
}
