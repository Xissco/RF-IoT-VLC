#include <SPI.h>
#include <Ethernet.h>
#include <ThingsBoard.h>

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

EthernetClient ethernetClient;
ThingsBoard tb(ethernetClient);

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};

// ****************************** ThingsBoard login details ******************************
const char* server = "172.16.1.254";           // MQTT Broker (i.e. server)
 
const char* client_id = "Infraestructura 1";   // Can be anything
const char* username =  "kuswzx9USm0ufFMXBMSm";   // Authentication token here

const char* topicToPublish_DATA = "v1/devices/me/telemetry"; // Topic address to publish to for sending data. 
const char* topicToPublish_ATTRIBUTES = "v1/devices/me/attributes"; // Topic address to publish to for sending attributes. 
// ****************************************************************************************

// Data variables
int led_delay;
int cont = 0;
bool emergencyFlag = false;

RPC_Response processDelayChange(const RPC_Data &data)
{
  led_delay = data;
  Serial.println(led_delay);
  return RPC_Response(NULL, true);
}

RPC_Response processGetDelay(const RPC_Data &data)
{
  Serial.println("Received the get value method");

  return RPC_Response(NULL, led_delay);
}

RPC_Callback callbacks[] = 
{
  { "getState",         processDelayChange },
};

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

}

void loop() 
{
  unsigned long currentMillis = millis(); // Store the time since the board started
  renovarDHCP();
  reconnect();
  if(currentMillis - previousMillis > interval) 
  {
    previousMillis = currentMillis; // reset the previous millis, so that it will continue to publish data.
    workflow(); // Workflow to use data
  }

  tb.loop(); // Called to maintain connection to server
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

void reconnect()
{
  if (!tb.connected()) 
  {
    while (!tb.connect(server, username)) 
    {
      Serial.println("Failed to connect");
      delay(5000);
    }
    while (!tb.RPC_Subscribe(callbacks, COUNT_OF(callbacks))) 
    {
      Serial.println("Failed to subscribe for RPC");
      delay(5000);
    }
  }
}

void workflow()
{
  cont++;
  Serial.println(cont);
  tb.sendAttributeInt("cont", cont);
  if(emergencyFlag) tb.sendAttributeBool("state", emergencyFlag);
  else tb.sendAttributeBool("state", emergencyFlag);
  if (cont==100) {cont = 0;}  
}
