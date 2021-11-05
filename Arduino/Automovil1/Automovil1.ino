#include <SoftwareSerial.h>
SoftwareSerial mySerial(6, 7,true); // RX, TX

#include <Wire.h>     // libreria para bus I2C
#include <Adafruit_GFX.h>   // libreria para pantallas graficas
#include <Adafruit_SSD1306.h>   // libreria para controlador SSD1306
 
#define ANCHO 128     // reemplaza ocurrencia de ANCHO por 128
#define ALTO 64       // reemplaza ocurrencia de ALTO por 64

#define OLED_RESET 4      // necesario por la libreria pero no usado
Adafruit_SSD1306 oled(ANCHO, ALTO, &Wire, OLED_RESET);  // crea objeto

void setup() {
  Serial.begin(9600);
  Wire.begin();         // inicializa bus I2C
  mySerial.begin (9600);  //probar a poner velocidades diferentes en cada punto.
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C); // inicializa pantalla con direccion 0x3C
  
  Serial.println("Starting Chat Program...");
  

}
void loop() { //en cada loop leo un caracter si hay en alguno de los bufferes
  
    oled.clearDisplay();      // limpia pantalla 
  
    oled.display();     // muestra en pantalla todo lo establecido anteriormente

  if (mySerial.available()) {
    Serial.print((char)mySerial.read());
    pantalla();
  }
  if (Serial.available()) {
    mySerial.print((char)Serial.read());
    pantalla();
  }
  //delay(500);
}

void pantalla(){
  oled.clearDisplay();      // limpia pantalla      
  oled.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
  oled.setCursor(30, 0);     // ubica cursor en inicio de coordenadas 0,0
  oled.setTextSize(1);      // establece tamano de texto en 1
  oled.print("VLC UDA 2021");  // escribe en pantalla el texto
  oled.drawRect(4,17 , 124, 45, WHITE); // dibuja rectangulo
  oled.setCursor(8, 20);   // ubica cursor en coordenadas 28,34
  oled.setTextSize(2);      // establece tamano de texto en 2
  oled.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
  oled.print("ALERTA");     // escribe texto

  oled.setCursor(8, 37);   // ubica cursor en coordenadas 28,34
  oled.setTextSize(2);      // establece tamano de texto en 2
  oled.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
  oled.print("AMBULANCIA");     // escribe texto
  
  oled.display();     // muestra en pantalla todo lo establecido anteriormente  
}
