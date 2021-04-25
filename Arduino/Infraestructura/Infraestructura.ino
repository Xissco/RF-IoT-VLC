#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x03}; 
char IPsemaforo[] = "172.16.1.253"; 
int puertoSemaforo = 8001;
unsigned int localPort = 8002;
byte emergencyFlag = 0;

EthernetUDP Udp;

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
  
  Udp.begin(localPort);

  Serial.print("My IP Address: ");
  Serial.println(Ethernet.localIP());
}

void loop()
{
  UDPSendPacket (emergencyFlag, IPsemaforo, puertoSemaforo);
  delay(1000);
}

void UDPSendPacket(byte data, char remote_IP[], int remote_port)
{
  Udp.beginPacket(remote_IP, remote_port);
  Udp.write(data);
  Udp.endPacket();    
}
