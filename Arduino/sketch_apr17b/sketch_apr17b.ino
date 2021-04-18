#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};
int cont = 0;
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
  renovarDHCP();
  cont++;
  if (cont == 101) cont = 0;
  Serial.println(cont);
  delay(1000);  
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
