#include <VirtualWire.h>

char dato[0];
const int LED1=4;
const int LED2=5;
const int BOTON=3;
int val=0;
int state=2;
int old_val=0;

void setup()
{
    Serial.begin(9600);    // Debugging only
    Serial.println("EMISOR RF");
    pinMode(LED1,OUTPUT);
    pinMode(LED2,OUTPUT);
    pinMode(BOTON,INPUT);

    // Se inicializa el RF
    vw_setup(2000); // velocidad: Bits per segundo
    vw_set_tx_pin(6); //Pin 2 como salida para el RF 
}
void emisorRF(){
   
    char buf[VW_MAX_MESSAGE_LEN]; // Cadena para enviar
    int dato1= 1; //variable con el tiempo en segundos
    int dato2= 2;
    String str="";  
  
val=digitalRead(BOTON);
if ((val == HIGH) && (old_val == LOW)){
//state=1-state;
state++;
if(state==3) state=0;
Serial.println(state);
delay(10);
}    
old_val = val;
    
    if (state==1){

          str="g"; //convertimos el entero a String y agramos un inicio de trama
          str.toCharArray(buf,sizeof(buf)); //convertimos el String en un array
          vw_send((uint8_t *)buf, strlen(buf)); //Enviamos el array
          vw_wait_tx(); //Esperamos hasta que el mensaje se envie
  
          digitalWrite(LED1,HIGH);
          digitalWrite(LED2,LOW);
          delay(500);
          digitalWrite(LED1,LOW);
          digitalWrite(LED2,HIGH);
          delay(500);
             
      }
    else if (state==2)
    {
          str="f"; //convertimos el entero a String y agramos un inicio de trama
          str.toCharArray(buf,sizeof(buf)); //convertimos el String en un array
          vw_send((uint8_t *)buf, strlen(buf)); //Enviamos el array
          vw_wait_tx(); //Esperamos hasta que el mensaje se envie
          digitalWrite(LED1,LOW);
          digitalWrite(LED2,LOW);
      
    }
    else if (state==0){

          str="i"; //convertimos el float a String y agramos un inicio de trama
          str.toCharArray(buf,sizeof(buf)); //convertimos el String en un array
          vw_send((uint8_t *)buf, strlen(buf)); ////Enviamos el array
          vw_wait_tx(); //Esperamos hasta que el mensaje se envie
          
          digitalWrite(LED1,HIGH);
          digitalWrite(LED2,LOW);
          delay(200);
          digitalWrite(LED1,LOW);
          digitalWrite(LED2,HIGH);
          delay(200);
             
      }  
   delay(200);
 
}

void loop()
{
   emisorRF();
}
