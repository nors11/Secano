//RST          D9
//SDA(SS)      D10
//MOSI         D11
//MISO         D12
//SCK          D13

#include <SPI.h>
#include <MFRC522.h>
#include <AESLib.h>

#define ERROR_LED 2
#define OK_LED 3
#define WATER 4
#define PULSES_IN 5

const int RST_PIN = 9;
const int SS_PIN = 10;
int button = 8;
int action = 0;
unsigned long maxiumShowerTime = 30000;//1min ahora para test-----   //Maxium time that shower can be active, master must define it
unsigned long onTimeStart;        //Time (millis) at shower system turns on
unsigned long totalOnTime;        //Time (millis) tha the shower has been active 
unsigned long activeTimes[20];
unsigned long lastActiveTag;      //Last tag number that the shower had autorithed 
short lastStatusPulses=0;
enum validStatus {
  active,
  inactive,
  blocked,
  forced
}; 
enum WATERSTATUS {
  runningWater,
  noRunningWater
}; 
validStatus showerStatus = inactive;
WATERSTATUS waterStatus =noRunningWater;
WATERSTATUS lastWaterStatus =noRunningWater;
unsigned char *str;
byte gymId=0x01;
byte section=0x01;
byte keyRFID0=0x00;
byte keyRFID1=0x00;
byte keyRFID2=0x96;
byte data[16] = { gymId, section, keyRFID0, keyRFID1, keyRFID2,0x00,0x00,0x00,0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
byte *writeData = data;

MFRC522 mfrc522(SS_PIN, RST_PIN);

MFRC522::MIFARE_Key key;

void printArray(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
void changeAction(){
  action = !action; 
  if(action==0){Serial.println("Escritura");} 
  else{Serial.println("Lectura");}
  delay(100);//
}
byte generateSecret(byte r1,byte r2,byte r3){
  byte sum = r1+r2+r3;
  if(sum % 2 == 0){ //es par
    sum = sum + 16;
  }
  else{sum =sum +15;}
  if(r3<200){sum= sum+4;}
  else{sum =sum +9;}
  return sum;
}
void setup()
{ 
  pinMode(ERROR_LED,OUTPUT);
  pinMode(OK_LED,OUTPUT);
  pinMode(WATER,OUTPUT);
  pinMode(PULSES_IN,INPUT);
  digitalWrite(PULSES_IN,HIGH);
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  for(int n=0;n<20;n++){          //Inicilize array content
    activeTimes[20]=0;
  }

}
boolean validTag(byte tag[16]){

  int secret = 0;
  int result = tag[0]+tag[1]+tag[2]+tag[3];
  int vx = result -((tag[5]*5)*tag[6]);
  if(tag[5] % 2 == 0) {secret = (vx + 16 - tag[5]);}
  else{secret =(vx + 11 - tag[5]);}
  return secret == tag[4];
}

boolean havePermision(byte tag[16]){
  unsigned long tagId = getTagNumber(tag);
  //--------------------------------------------------------Enviar a master tag para ver si esta registrado
  return true;
}

unsigned long getTagNumber(byte buffer[16]){
    unsigned long a = buffer[2];
    unsigned long b = buffer[3];
    unsigned long c;
    c = a * 1000 + b;
    return c;
}
unsigned long calculateRestTime(){
    int n =0;
    boolean addRegistrer = false;
    unsigned long time = millis()-onTimeStart;
    do{
      if(activeTimes[n]==0){
        activeTimes[n]=time;
        addRegistrer=true;
      }
      n++;
    }while (!addRegistrer && n<20);
    totalOnTime =0;
    for(int n=0;n<20;n++){
      totalOnTime += activeTimes[n];
    }
    return maxiumShowerTime - totalOnTime;
}
void loop()
{
  /*
  updateWaterStatus();
  if(waterStatus != lastWaterStatus){
    Serial.print("Detectado cambio, agua ");
    if(waterStatus==runningWater){Serial.println("corriendo");lastWaterStatus = runningWater;}
    else{Serial.println("detenida");lastWaterStatus= noRunningWater;}
  }*/
  
  if(showerStatus== active){
    updateWaterStatus();    //Check if the water is running
    if(waterStatus==runningWater && lastWaterStatus == noRunningWater){  //First water activation 
      Serial.println("ESTADO 1: ");
      onTimeStart = millis();
      lastWaterStatus = runningWater;
    }
    else if(waterStatus==noRunningWater && lastWaterStatus == runningWater){  //water stops runnig save the time that it was on and running
      Serial.println("ESTADO 2: ");
      int n =0;
      boolean addRegistrer = false;
      unsigned long time = millis()-onTimeStart;
      Serial.print("tiempo periodo : ");
      Serial.println(time);
      do{
        if(activeTimes[n]==0){
          activeTimes[n]=time;
          addRegistrer=true;
        }
        n++;
      }while (!addRegistrer && n<20);
      lastWaterStatus = noRunningWater;
      delay(300);
    }
    else if(waterStatus==runningWater && lastWaterStatus == runningWater){
      Serial.println("ESTADO 3: ");
      unsigned long tActual = millis();
      totalOnTime =0;
      for(int n=0;n<20;n++){
        totalOnTime += activeTimes[n];
      }
      Serial.print("Tiempo total: ");
      Serial.println(totalOnTime);
      delay(300);
      if(totalOnTime>maxiumShowerTime || (tActual-onTimeStart)>maxiumShowerTime || ((tActual-onTimeStart)+totalOnTime)>maxiumShowerTime ){     //    Maxium time was exceeded stop shower and restart values
        Serial.print("PARO DE DUCHA POR TIEMPO-------------- ");
        lastWaterStatus = noRunningWater;
        for(int n=0;n<20;n++){                                                        //Restore array with time periods
           activeTimes[n]=0;;
        }
        showerOff();
        sendInfoToMaster(0,lastActiveTag);
      }
    }
    else{
       //Serial.println("ESTADO 4$$$$$$: ");
    }

  }

  if (!mfrc522.PICC_IsNewCardPresent())
    return;

  if (!mfrc522.PICC_ReadCardSerial())
    return;

  MFRC522::StatusCode status;
  byte trailerBlock = 7;
  byte sector = 1;
  byte blockAddr = 4;

  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
 ///* if (status != MFRC522::STATUS_OK) {
  //  Serial.print(F("PCD_Authenticate() failed: "));
   // Serial.println(mfrc522.GetStatusCodeName(status));
   // return;
 // }
  byte buffer[18];
  byte size = sizeof(buffer);
  // Read data from the block (again, should now be what we have written)
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  printArray(buffer, 16); Serial.println();                                     //Print info of the readed Tag

  //----------------------------------------------------------------------------Aqui solo llega si han pasado un TAG sea valido o no
  if(showerStatus == inactive){
    //Serial.println("estado Inactivo");
    if(validTag(buffer) && havePermision(buffer)){   //Tag es valido y master autoriza para hoy 
      Serial.println("Pasamos a estado activo");
      lastActiveTag = getTagNumber(buffer);
      showerOn();
    }else{error();} 
  }
  else if(showerStatus == active){
    //Serial.println("estado activo");
    if(validTag(buffer)){     //Tag es valido
      if(getTagNumber(buffer) == lastActiveTag){                                            //The tag readed is the same that have the shower at this moment registered
          showerOff();
          unsigned long spareTime = calculateRestTime();                                    //calculates the remaining time left for the user
          sendInfoToMaster(spareTime,lastActiveTag);                                        //Send info to master
          totalOnTime = 0;                                                                  //
          onTimeStart=0;
          lastWaterStatus = noRunningWater;
          for(int n=0;n<20;n++){                                                            //Restore array with time periods
           activeTimes[n]=0;;
          }
      }
    }
  
  }
  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
  
  
  delay(5);
}
void error(){
  digitalWrite(ERROR_LED,HIGH);
  delay(300);
  digitalWrite(ERROR_LED,LOW);
  return;
}
void showerOn(){
  digitalWrite(OK_LED,HIGH);
  digitalWrite(WATER,HIGH);
  showerStatus= active;
  delay(1000);
  return;
}
void showerOff(){
  digitalWrite(OK_LED,LOW);
  digitalWrite(WATER,LOW);
  showerStatus = inactive;
  delay(1000);
  return;
}
void updateWaterStatus(){ 
  //if(!digitalRead(PULSES_IN)) waterStatus = runningWater; 
  //else waterStatus = noRunningWater;
  
  short status=0;
  short pulses = 0;  
  for(int n= 0;n<20;n++){
    if(!digitalRead(PULSES_IN)){
      status =1;
    }
    else{status=0;}
    if(status != lastStatusPulses){
      lastStatusPulses =status;
      pulses ++;
      //Serial.print("Pulsos: ");
      Serial.println(pulses);
      //Serial.println("PULSOS");
    }
    delay(1);
  }
  
  if(pulses>2)waterStatus = runningWater;   //Si velocidad es muy alta deberia augmentar numero de lecturas o reducir a "pulses > 1"
  else waterStatus = noRunningWater;
  
}
void sendInfoToMaster(unsigned long spareTime, unsigned long tagId){
  Serial.print("Tiempo restante de usuario nº ");
  Serial.print(tagId); Serial.print(" = ");Serial.print(spareTime); 
}