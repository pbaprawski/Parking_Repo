//Jest to próba ulepszenia poprzedniej wersji programu poprzez optymalizację kodu i zwiększenie jego czytelności
//Gdy zworka jest zamontowana, to wykorzystujemy diodę czerwoną i moduł HMC
#include<math.h>
#include <Wire.h>
const int buttonPin = 2;
const int RedDiode =  3;
const int YellowDiode =  4;
#define addressHMC 0x1E    //adres modułu HMC5883L dla I2C
#define addressQMC 0x0D
int address=addressHMC;
  double Ex=0;
  double Varx=1;
  double pomiary[10];
bool IsHMC=true;
int ledPin=4;
int buttonState = 0;
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
  
  double pom=0;
  double pom2=0;

  pinMode(buttonPin, INPUT);
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
  
}
void loop(){
  
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

  for(int i=0;i<9;i++){
    pomiary[i]=pomiary[i+1];
    }
    pomiary[9]=sqrt(x*x+y*y+z*z);
  
  double pom=0;
  for(int i=0;i<10;i++){
    pom+=pomiary[i];
    }
  tEx=pom/10;
  

  if (tEx<Ex-sqrt(Varx) || tEx>Ex+sqrt(Varx)){digitalWrite(ledPin, HIGH);

  Serial.println("Wykryto zaburzenie");
  }
  else{digitalWrite(ledPin, LOW);
  Serial.println("Wykryto brak xD");
  }
  Serial.print("vector: ");
  double v=x*x+y*y+z*z;
  Serial.print(sqrt(v));
  Serial.print(":");
  Serial.print(Ex);
  Serial.print(":");
  Serial.print(tEx);
  Serial.print(":");
  delay(500);
}
