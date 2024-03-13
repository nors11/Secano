#include <EEPROM.h>
#include "functions.h"
#include <Wire.h> 

/* #include "vglobals.h" */
#define HEAD 0xAA
#define TAIL 0xFE

// Comandos soportados---------------
//#define UPDATE_STATUS   0x01
#define UPDATE_STATUS   0x01
#define RESPONSE_REJECT 0x02
#define RESPONSE_ACCEPT 0x03
#define CMD_LED_ON      0x04
#define CMD_LED_OFF     0x05
// Formato de Trama: <HEAD> <SLAVE_ID> <CMD> <TAIL>
// Ejemplo: 0xFF 0x34 0x01 0xFE -> Indica que el esclavo 0x34 debe ejecutar la orden 0x01
#define RS485_PIN_MODE 25         // HIGH -> Transmision; LOW-> recepcion
//#define SLAVE 0x23
#define SLAVE 0x01
byte trama[5], idx;
//----------------------------------
int codigoMaster = 11223;
const int maxUsers = 2000;
long usersKey = 2000;
bool blackList[maxUsers];
bool remainCredit[maxUsers];
int showersNumber=1;
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
  delay(2000);
  Serial3.readBytes( trama, 6 );

  if( trama[0] != HEAD )    // error en la trama
    return -1;
    
  if( trama[1] != esclavo ) // respuesta de otro esclavo
    return -1;

  if( trama[5] != TAIL )    // error en trama
    return -1;

  return trama[4];
  
}






void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  setupConfig();
  pinMode( RS485_PIN_MODE, OUTPUT );
  digitalWrite(RS485_PIN_MODE, LOW);  // modo recepcion
  Serial3.begin(9600);                // Configurar Serial3 para el bus RS-485

  //firstBoot = EEPROM.read(1);  Poner a 1 si es firts boot
  
  if(firstBoot){
    for(int n=0;n<maxUsers;n++){
      blackList[n]=false;
      remainCredit[n]=1;
    }
    firstBoot =false;
  }

}

void loop() {
    
  for(int nSlave=1;nSlave<=showersNumber;nSlave++){
    sendCommand(nSlave, CMD_LED_ON);
    int h = recibirRespuesta(SLAVE);
        if( h == -1 )
          Serial.println( "No se recibio' respuesta" );
        else
          Serial.print( "respuesta:" );
          Serial.println(h);
          Serial.print( "User ID:" ); Serial.print(trama[2]);Serial.println(trama[3]);
          isKeyAccepted();
  }
  delay(5000);
  /*
  if( Serial.available() ){
    char c = Serial.read();
    int h, t;
    switch(c){
      case '0':
        Serial.println("ENVIO ON");
        sendCommand(SLAVE, CMD_LED_ON);
        h = recibirRespuesta(SLAVE);
        if( h == -1 )
          Serial.println( "No se recibio' respuesta" );
        else
          Serial.print( "respuesta:" );
          Serial.println(h);
          Serial.print( "User ID:" ); Serial.print(trama[2]);Serial.println(trama[3]);
          isKeyAccepted();
        break;
        
      case '1':
        Serial.println("ENVIO OFF");
        sendCommand(SLAVE, CMD_LED_OFF);
        break;

      /*case '2':
        enviarComando(SLAVE, CMD_READ_TEMP);
        Serial.print("Temperatura: ");
        t = recibirRespuesta(SLAVE);
        if( t == -1 )
          Serial.println( "No se recibio' respuesta" );
        else
          Serial.println(t);
        break;

      case '3':
        enviarComando(SLAVE, CMD_READ_HUMED);
        Serial.print("Humedad: ");
        h = recibirRespuesta(SLAVE);
        if( h == -1 )
          Serial.println( "No se recibio' respuesta" );
        else
          Serial.println(h);
        break;//

      default:
        break;
    }
    delay(5);
  }*/

  //EEPROM.put( eeAddress, customVar );
  //if(firstBoot){setFirstConfig();}
  //delay(1000);

}
void isKeyAccepted(){
  bool result = true;
  int dataA= trama[2];
  int dataB= trama[3];
  long userKey= (dataA*1000)+dataB;
  Serial.print("Tag usuario convertido: ");Serial.println(userKey);
  if(blackList[userKey] || remainCredit[userKey]==0){result = false;}
  
  return result;
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
