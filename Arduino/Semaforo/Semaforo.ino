#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02}; 
unsigned int localPort = 8001;

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
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    byte data2 = UDPReceivePacket();
    Serial.println(data2);
  }
}
  byte UDPReceivePacket()
  {
    byte bufferRead [1]; 
    Udp.read(bufferRead,1);
    byte data = bufferRead[0];
    return data;    
  }
