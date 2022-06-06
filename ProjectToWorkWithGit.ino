//Jest to próba ulepszenia poprzedniej wersji programu poprzez optymalizację kodu i zwiększenie jego czytelności
//Gdy zworka jest zamontowana, to wykorzystujemy diodę czerwoną i moduł HMC
#include<math.h>
#include <Wire.h>
#include <MKRWAN.h>
#include "arduino_secrets.h"

#define addressHMC 0x1E    //adres modułu HMC5883L dla I2C
#define addressQMC 0x0D
#define ZajetyMSG "1"
#define WolnyMSG "0"

LoRaModem modem;
double sensitivity=1;
int error_counter=0;
const int buttonPin = 2;
const int resetButtonPin = 16;
const int resetPin=0;
const int RedDiode =  3;
const int YellowDiode =  4;
String LastMessage = "NULL";

void reboot(){digitalWrite(RedDiode, HIGH);
      digitalWrite(YellowDiode, HIGH);
      delay(1000);
      digitalWrite(RedDiode, LOW);
      digitalWrite(YellowDiode, LOW);
      digitalWrite(resetPin, LOW);}

int address=addressHMC;
double Ex=0;
double Varx=1;
double pomiary[10];
bool IsHMC=true;
int ledPin=4;
int buttonState = 0;
//void(* resetFunc) (void) = 0;

// Please enter your sensitive data in the Secret tab or arduino_secrets.h
String appEui = SECRET_APP_EUI;
String appKey = SECRET_APP_KEY;

bool LoraSender(String msg)
{
  modem.beginPacket();
  modem.print(msg);
 int err = modem.endPacket(true);
    if (err > 0) {
    error_counter=0;
  }
  else{error_counter++;}
  if(error_counter>15){
    reboot();
    Serial.println("Will not send because of an error");
    }//in case of multiple troubles in a row with sending messages
	return err;//zwraca 1 jak sie uda, i 0 jak sie nie uda wyslac
}

String LoraReceiver(String msg)
{
  if (!modem.available()) {
    return "";
  }
  unsigned char rcv[64];//czy na pewno unsigned????
  int i = 0;
  while (modem.available()) {
    rcv[i++] = (char)modem.read();
  }
  /*
  Serial.print("Received: ");
  for (unsigned int j = 0; j < i; j++) {
    Serial.print(rcv[j] >> 4, HEX);
    Serial.print(rcv[j] & 0xF, HEX);
    Serial.print(" ");
  }
  */
  sensitivity=(rcv[0] >> 4)/4;
  if((rcv[0]& 0xF)==1){reboot();}
  
}

int LoraAdvancedSender(String msg){
  Serial.println("LoraAdvancedSender");
	if (LastMessage.compareTo("NULL")==0)
	{
		if (LoraSender(msg)){LastMessage=msg;return 1;}
		else{return -1;}
	}
	else if (LastMessage.compareTo(msg)!=0){
		if (LoraSender(msg)){LastMessage=msg;return 1;}
		else{return -1;}}
	return -1;
}


void ComS(bool isHMC){

  if (isHMC)
    {

  Wire.beginTransmission(addressHMC);
  Wire.write(0x02);
  Wire.write(0x00);
  Wire.endTransmission();
    }
  else
  {
  Wire.beginTransmission(addressQMC);
  Wire.write(0x0B);
  Wire.write(0x01);
  Wire.endTransmission();
  Wire.beginTransmission(addressQMC);
  Wire.write(0x09);
  Wire.write(0x1D);
  Wire.endTransmission();
  }
  }


  void ComS2(bool isHMC){

  if (isHMC)
    {
    
      Wire.beginTransmission(addressHMC);
  Wire.write(0x03);
  Wire.endTransmission();
    
    }
  else
  {
       Wire.beginTransmission(addressQMC);
  Wire.write(0x00);
  Wire.endTransmission();
  }
 
  
  }

void setup(){
pinMode(resetPin, OUTPUT);
digitalWrite(resetPin, HIGH);
pinMode(resetButtonPin, INPUT);
  
  double pom=0;
  double pom2=0;

	pinMode(buttonPin, INPUT);
    pinMode(resetButtonPin, INPUT);
    buttonState = digitalRead(buttonPin);

  if (buttonState == HIGH) {
      address=addressHMC;
      IsHMC=true;
      ledPin=4;
      Serial.println("Using HMC module");
    
  } else {
      address=addressQMC;
      IsHMC=false;
      ledPin=3;
      Serial.println("Using QMC module");
    
  }

  
  
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH);   //Dioda sygnalizacyjna
  delay(1000);                      
  digitalWrite(ledPin, LOW);
  delay(500);   
  Serial.begin(9600);                              
  Wire.begin();                   //inicjalizacja modułu
  double tab[100];

ComS(IsHMC);


  int x,y,z;

 for (int i=0; i<100;i++){
  
ComS2(IsHMC);
  
  Wire.requestFrom(address, 6);
  if(6<=Wire.available()){
    x = Wire.read()<<8;
    x |= Wire.read();
    z = Wire.read()<<8;
    z |= Wire.read();
    y = Wire.read()<<8;
    y |= Wire.read();
  }
x=x/16;y=y/16;z=z/16;

  

  Serial.println(sqrt(x*x+y*y+z*z));
tab[i]=sqrt(x*x+y*y+z*z);
delay(200);
if (digitalRead(resetButtonPin) == HIGH) {reboot();}
 }



for (int i=0; i<100;i++){
  pom+=tab[i];
  pom2+=tab[i]*tab[i];
  }
Ex=pom/100;
Varx=pom2/100-pom*pom/100/100;
for (int i=0;i<10;i++){
  pomiary[i]=tab[i+90];
  }


for (int i=0;i<7;i++){
    digitalWrite(ledPin, HIGH);
  delay(300);
  digitalWrite(ledPin, LOW);
  delay(300);   
  }


  if (!modem.begin(EU868)) {
    Serial.println("Failed to start module");
    reboot();
  };

  int connected = modem.joinOTAA(appEui, appKey);
  if (!connected) {
    while (1) {delay(6000);//In case it crashes it will be reset after a moment
 reboot();}
  }

  modem.minPollInterval(60);



}
void loop(){
  Serial.println("Jestes w petli");
	  if (digitalRead(resetButtonPin) == HIGH) {reboot();}
  int x,y,z;
  double tEx;
	ComS2(IsHMC);
 
  Wire.requestFrom(address, 6);
  if(6<=Wire.available()){
    x = Wire.read()<<8;
    x |= Wire.read();
    z = Wire.read()<<8;
    z |= Wire.read();
    y = Wire.read()<<8;
    y |= Wire.read();
  }
  x=x/16;y=y/16;z=z/16;

  for(int i=0;i<9;i++){pomiary[i]=pomiary[i+1];}
  
  pomiary[9]=sqrt(x*x+y*y+z*z);
  
  double pom=0;
  for(int i=0;i<10;i++){pom+=pomiary[i];}
  tEx=pom/10;
  

  if (tEx<Ex-sensitivity*sqrt(Varx) || tEx>Ex+sensitivity*sqrt(Varx)){
  digitalWrite(ledPin, HIGH);
  LoraAdvancedSender(ZajetyMSG);
  Serial.println("Wykryto zaburzenie");}
  
  else{digitalWrite(ledPin, LOW);
  LoraAdvancedSender(WolnyMSG);
  Serial.println("Wykryto brak xD");}
  
  
  Serial.print("vector: ");double v=x*x+y*y+z*z;
  Serial.print(sqrt(v));Serial.print(":");Serial.print(Ex);
  Serial.print(":");Serial.print(tEx);Serial.print(":");
  delay(500);//TODO zamiast tego delaya trzeba dać usypianie -tryb oszczędzania energii
}
