#include <Arduino.h>
#include <EEPROM.h>
#include "functions.h"
#include <Wire.h> 
#include <SPI.h>
#include <MFRC522.h>

// Comandos soportados---------------
//#define UPDATE_STATUS   0x01

#define UPDATE_STATUS       0x01      //Master to slave
#define RESPONSE_REJECT     0x02      //Master to slave
#define RESPONSE_ACCEPT     0x03      //Master to slave
#define RESPONSE_NO_ACTIVITY 0x04     //Slave to master -- System is waiting for key approx
#define RESPONSE_STORED_OK  0x05      //Master to slave -- Shower service stored to this user
#define REQUEST_VALIDATE    0x06      //Slave to master -- System has a key pending to validate
#define REQUEST_STORE_END   0x07      //Slave to master -- User had finished the service, master must store it
#define SHOWER_BLOCKED      0x08      //BIDI
#define SHOWER_FORCED       0x09      //BIDI
#define CMD_LED_ON          0x0A
#define CMD_LED_OFF         0x0B
#define CHANGE_STATUS       0x0C
#define CHANGE_SHOWER_TIME  0x0D

#define ERROR               0x0F

#define HEAD 0xAA
#define TAIL 0xFE
// Formato de Trama: <HEAD> <SLAVE_ID> <CMD> <TAIL>
// Ejemplo: 0xFF 0x34 0x01 0xFE -> Indica que el esclavo 0x34 debe ejecutar la orden 0x01
#define RS485_PIN_MODE 25         // HIGH -> Transmision; LOW-> recepcion
//#define SLAVE 0x01
byte trama[5], idx;
//----------------------------------
const int RST_PIN = 48;               //RFID RESET
const int SS_PIN = 53;                //RFID SDA
int codigoMaster = 11223;
const int maxUsers = 1300;
long usersKey = 1300;
bool blackList[maxUsers];
short remainCredit[maxUsers];
int showersNumber=1;
int maxV =3000;
bool firstBoot = true;
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

void sendCommand(byte esclavo, byte cmd,byte val){
  trama[0] = HEAD;
  trama[1] = esclavo;
  trama[2] = cmd;
  trama[3] = val;
  trama[4] = TAIL;
  digitalWrite(RS485_PIN_MODE, HIGH); // modo tx
  Serial3.write(trama, 5);
  Serial3.flush();  
  digitalWrite(RS485_PIN_MODE, LOW); // modo rx
}

int recibirRespuesta( byte esclavo ){
  digitalWrite(RS485_PIN_MODE, LOW); // modo rx
  delay(100);                         //estaba a 2000  antes por que lo deje asi?
  Serial3.readBytes( trama, 6 );

  if( trama[0] != HEAD )    // error en la trama
    return -1;
    
  if( trama[1] != esclavo ) // respuesta de otro esclavo
    return -1;

  if( trama[5] != TAIL )    // error en trama
    return -1;
  return trama[4]; 
}

int getUserKey(){
  return ((trama[2]*1000)+trama[3]);
}

boolean validTag(byte tag[16]){
  int secret = 0;
  int result = tag[0]+tag[1]+tag[2]+tag[3];
  int vx = result -((tag[5]*5)*tag[6]);
  if(tag[5] % 2 == 0) {secret = (vx + 16 - tag[5]);}
  else{secret =(vx + 11 - tag[5]);}
  return secret == tag[4];
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //RS485.begin(9600);
  SPI.begin();                        
	mfrc522.PCD_Init(); 
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }                 
  setupConfig();
  pinMode( RS485_PIN_MODE, OUTPUT );
  digitalWrite(RS485_PIN_MODE, LOW);  // modo recepcion
  Serial3.begin(9600);                // Configurar Serial3 para el bus RS-485

  firstBoot = EEPROM.read(1);  //Poner a 1 si es firts boot
  //EEPROM.write(1,1);
  if(firstBoot){
    Serial.println("ES FIRST BOOT");
    for(int n=0;n<maxUsers;n++){
      blackList[n]=false;
      remainCredit[n]=2;
    }
    //firstBoot =false;
  }

}

void loop() {
  //if(firstBoot){setFirstConfig();}
  firstBoot =false;
  configMenu();
  showDateTime(); 
  delay(10);

  /*int tagPresent = rfidRead();   devuelve numero de llavero leido
  if (tagPresent != -1)
  {
    Serial.print("Id llavero :");Serial.println(tagPresent);
  }*/
  
   
  /*
  for(int nSlave=1;nSlave<=showersNumber;nSlave++){
    sendCommand(nSlave, UPDATE_STATUS,0);
    Serial.print( "Num_Esclavo: " );Serial.print(nSlave);
    int response = recibirRespuesta(nSlave);
        if( response == -1 ){
          Serial.println( " ->No se recibio respuesta" );
        }        
        else{
          Serial.print( " -> respuesta:" );
          Serial.println(response);
          switch(response){
            case RESPONSE_NO_ACTIVITY:
              Serial.println( "NO_ACTIVITY" );
              break;
            case REQUEST_VALIDATE:
              Serial.println( "REQUEST_VALIDATE" );
              Serial.print( "User ID:" ); Serial.print(trama[2]);Serial.println(trama[3]);
              if(isKeyAccepted()) sendCommand(nSlave,RESPONSE_ACCEPT,0);
              else sendCommand(nSlave,RESPONSE_REJECT,0);
              break;
            case REQUEST_STORE_END:
              Serial.println( "REQUEST_STORE_END" );
              Serial.print( "User ID:" ); Serial.print(trama[2]);Serial.println(trama[3]);
              remainCredit[getUserKey()]--;    //= false;
              sendCommand(nSlave,RESPONSE_STORED_OK,0);
              break;
            case SHOWER_BLOCKED:
              Serial.println( "Estado Bloqueada" );
              break;
            case SHOWER_FORCED:
              Serial.println( "Estado Bypass" );
              break;
            default:
              break;
          }
          /*
          Serial.print( "User ID:" ); Serial.print(trama[2]);Serial.println(trama[3]);
          if(trama[4]==0x01){ //Esclavo solicita validar tag
          Serial.println("Esclavo pide valdar tag");
            if(isKeyAccepted()) sendCommand(nSlave,RESPONSE_ACCEPT,0);
            else sendCommand(nSlave,RESPONSE_REJECT,0);
          }
          else if(trama[4]==0x02){
              //Aqui esclavo me esatria diciendo que este tag ha realizado una ducha y debo incrementar en memoria
          }
        }   
  }
  delay(2000);*/
  //EEPROM.put( eeAddress, customVar );
  
  //delay(1000);

}
bool isKeyAccepted(){
  Serial.print( "User ID EN HEX  :" ); Serial.print(trama[2],HEX);Serial.println(trama[3],HEX);
  bool result = true;
  int dataA= trama[2];
  int dataB= trama[3];
  Serial.print( "User ID EN ACCP_B:" ); Serial.print(dataA);Serial.println(dataB);
  long userKey= (dataA*1000)+dataB;
  Serial.print("Tag usuario convertido: ");Serial.println(userKey);
  if(blackList[userKey] || remainCredit[userKey]==0){result = false;}
  
  return result;
}

void findDevices(){
  bool showerStatus[showersNumber];
  for(int i=1;i<showersNumber;i++){
      sendCommand(i,UPDATE_STATUS,0);
      int response = recibirRespuesta(i);
      if( response == -1 ) showerStatus[i] = 0;
      else showerStatus[i] = 1;            
  }
}

bool isAlive(int slaveNumber){
    sendCommand(slaveNumber,UPDATE_STATUS,0);
    int response = recibirRespuesta(slaveNumber);
    if( response == -1 ) return false;
    else return true;            
}

bool isInBlackList(int id){
  return (blackList[id]);
}

bool putInBlacklist(int id){
  if(id>maxUsers){return false;}
  else{
    blackList[id] = true;
    return true;
  }
}

bool takeOutOfBlackList(int id){
  if(id>maxUsers){return false;}
  else{
    blackList[id] = false;
    return true;
  }
}

int rfidRead(){
  if (!mfrc522.PICC_IsNewCardPresent())
        return -1;

  if (!mfrc522.PICC_ReadCardSerial())
        return -1;

  MFRC522::StatusCode status;
  byte trailerBlock = 7;
  byte sector = 1;
  byte blockAddr = 4;

  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  byte buffer[18];
  byte size = sizeof(buffer);
  // Read data from the block (again, should now be what we have written)
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  printArray(buffer, 16); Serial.println();
        // Halt PICC
  mfrc522.PICC_HaltA();
      // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
  
  if (validTag(buffer))
  {
    return (buffer[2]*1000)+buffer[3];
  }
  return -1;
}

void printArray(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

short getRemainCredit(int id){
  return remainCredit[id];
}

int getShowerStatus(int id){
 sendCommand(id,UPDATE_STATUS,0);
 int status = recibirRespuesta(id);
 return status;
}

bool setShowerStatus(int id,int st){
  sendCommand(id,CHANGE_STATUS,st);
  int response = recibirRespuesta(id);
  if(response != -1){return true;}
  else return false;
}

bool updateShowerTimeToDevices(int time){
  int succes =0;
  for(int nSlave=1;nSlave<=showersNumber;nSlave++){
    sendCommand(nSlave,CHANGE_SHOWER_TIME,time);
    int response = recibirRespuesta(nSlave);
    if(response == RESPONSE_STORED_OK) succes ++;
  }
  return (succes == showersNumber);
}
/*  Serial.print("codigo antes de ir a funcion = ");Serial.println(codigoMaster);
  int *dir;
  dir = &codigoMaster;
  changeMasterCode(dir);
  Serial.print("codigo al volver = ");Serial.println(codigoMaster);
  delay(3000); */
/*
void printDate(DateTime date)
{
  Serial.print(date.year(), DEC);
  Serial.print('/');
  Serial.print(date.month(), DEC);
  Serial.print('/');
  Serial.print(date.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[date.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(date.hour(), DEC);
  Serial.print(':');
  Serial.print(date.minute(), DEC);
  Serial.print(':');
  Serial.print(date.second(), DEC);
  Serial.println();
}*/
