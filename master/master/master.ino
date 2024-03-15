#include <EEPROM.h>
#include "functions.h"
#include <Wire.h> 

/* #include "vglobals.h" */
#define HEAD 0xAA
#define TAIL 0xFE

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

// Formato de Trama: <HEAD> <SLAVE_ID> <CMD> <TAIL>
// Ejemplo: 0xFF 0x34 0x01 0xFE -> Indica que el esclavo 0x34 debe ejecutar la orden 0x01
#define RS485_PIN_MODE 25         // HIGH -> Transmision; LOW-> recepcion
//#define SLAVE 0x23
#define SLAVE 0x01
byte trama[5], idx;
//----------------------------------
int codigoMaster = 11223;
const int maxUsers = 1500;
long usersKey = 1500;
bool blackList[maxUsers];
short remainCredit[maxUsers];
int showersNumber=2;
int maxV =3000;
bool firstBoot = true;

void sendCommand(byte esclavo, byte cmd){
  trama[0] = HEAD;
  trama[1] = esclavo;
  trama[2] = cmd;
  trama[3] = TAIL;
  digitalWrite(RS485_PIN_MODE, HIGH); // modo tx
  Serial3.write(trama, 4);
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




void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
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
  if(firstBoot){setFirstConfig();}
  firstBoot =false;
  configMenu();
  showDateTime();
  delay(10);
  /*
  for(int nSlave=1;nSlave<=showersNumber;nSlave++){
    sendCommand(nSlave, UPDATE_STATUS);
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
              if(isKeyAccepted()) sendCommand(nSlave,RESPONSE_ACCEPT);
              else sendCommand(nSlave,RESPONSE_REJECT);
              break;
            case REQUEST_STORE_END:
              Serial.println( "REQUEST_STORE_END" );
              Serial.print( "User ID:" ); Serial.print(trama[2]);Serial.println(trama[3]);
              remainCredit[getUserKey()]--;    //= false;
              sendCommand(nSlave,RESPONSE_STORED_OK);
              break;
            default:
              break;
          }
          /*
          Serial.print( "User ID:" ); Serial.print(trama[2]);Serial.println(trama[3]);
          if(trama[4]==0x01){ //Esclavo solicita validar tag
          Serial.println("Esclavo pide valdar tag");
            if(isKeyAccepted()) sendCommand(nSlave,RESPONSE_ACCEPT);
            else sendCommand(nSlave,RESPONSE_REJECT);
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
      sendCommand(i,UPDATE_STATUS);
      int response = recibirRespuesta(i);
      if( response == -1 ) showerStatus[i] = 0;
      else showerStatus[i] = 1;            
  }
}
bool isAlive(int slaveNumber){
    sendCommand(slaveNumber,UPDATE_STATUS);
    int response = recibirRespuesta(slaveNumber);
    if( response == -1 ) return false;
    else return true;            
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
