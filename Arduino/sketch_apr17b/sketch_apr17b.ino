#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>   
#include <ArduinoJson.h>     

EthernetClient ethernetClient;
PubSubClient mqttClient(ethernetClient);

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};

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
char JSON_Data[100]; // Used to store the generated data in JSON

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
}

void send_data(void)
{
  create_JSON_Data(); // Set up the data to be published
  mqttClient.publish(topicToPublish_DATA, JSON_Data); // Publish JSON data to ThingsBoard
}

void create_JSON_Data(void)
{
  StaticJsonBuffer<200> JSON_Buffer; // Allocate JSON buffer with 200-byte pool
  JsonObject& JSON_Object = JSON_Buffer.createObject(); // Create JSON object (i.e. document)
  
  // Now populate the JSON document with data
  JSON_Object["Contador"] = cont; // Create JSON object named "Temperature", assigned with our temperature data
  JSON_Object["Humedad"] = 90;
  
  JSON_Object.printTo(JSON_Data); // Store the data on global variable
}
