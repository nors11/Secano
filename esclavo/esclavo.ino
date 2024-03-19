//RST          D9
//SDA(SS)      D10
//MOSI         D11
//MISO         D12
//SCK          D13
//-----------------------------ESCLAVO 120324_13_23

#include <SPI.h>
#include <MFRC522.h>
#include <AESLib.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#define HEAD 0xAA
//#define MY_SLAVE_ID 0x01
#define TAIL 0xFE
#define RS485_PIN_MODE 8         // HIGH -> Transmision; LOW-> recepcion
#define ERROR_LED 2
#define OK_LED 3
#define WATER 4
#define PULSES_IN 5

//Comandos soportados
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
#define CHANGE_YOUR_ID      0x0E
#define ERROR               0x0F

int MY_SLAVE_ID = 0x01;
byte buffer[18];
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
bool pendingToValidateTag = false;
bool pendingToConfirmShowerEnd = false;
bool booted =false;
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
SoftwareSerial RS485(6, 7);    // RX, TX
byte idUser[2]={0x01,0xF0};    // 001 240
byte buff[5], idx;
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
void sendResponse( int x ){
  buff[0] = HEAD;
  buff[1] = MY_SLAVE_ID;
  buff[2] = idUser[0];
  buff[3] = idUser[1];
  buff[4] = (byte)x;    //accion
  buff[5] = TAIL;
  digitalWrite( RS485_PIN_MODE, HIGH ); // poner en modo Tx
  RS485.write( buff, 6 );               // transmitir mensaje
  RS485.flush();
  digitalWrite( RS485_PIN_MODE, LOW);   // poner en modo Rx
}

void setup()
{ 
  Serial.begin(9600);
  RS485.begin(9600);
  SPI.begin();
  pinMode( RS485_PIN_MODE, OUTPUT );
  digitalWrite( RS485_PIN_MODE, LOW );// poner en modo de recepcion
  pinMode(ERROR_LED,OUTPUT);
  pinMode(OK_LED,OUTPUT);
  pinMode(WATER,OUTPUT);
  pinMode(PULSES_IN,INPUT);
  digitalWrite(PULSES_IN,HIGH);
  mfrc522.PCD_Init();
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  for(int n=0;n<20;n++){          //Inicilize array content
    activeTimes[20]=0;
  }
  /*
  const int ID_TO_STORE = 99;
  EEPROM.write(1,true); //Si ejecuta esto el sistema da por completo el primer inicio
  EEPROM.write(10,ID_TO_STORE); 
  */
  if(!EEPROM.read(1)){ //booted =false;
    MY_SLAVE_ID = 99;

  }
  else{
    MY_SLAVE_ID = EEPROM.read(10);
    Serial.print("My ID:");Serial.println(MY_SLAVE_ID);
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
  if( !RS485.available() ){   //Si no hay recepcion de mensaje entra aqui..
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
          //delay(300);
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
          //delay(300);
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
      //byte buffer[18];
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
        if(validTag(buffer)){   // && havePermision(buffer) Tag es valido y master autoriza para hoy Quizas tendria que poner a eperar rspuesta de master
          pendingToValidateTag = true;
          idUser[0]= buffer[2];   
          idUser[1]= buffer[3];
          //Serial.println("Pasamos a estado activo");
          //lastActiveTag = getTagNumber(buffer);
          //showerOn();
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
      
    return;
  }
  
  else{
      byte incoming = RS485.read();
      //Serial.print("Recibido: ");
      //Serial.println(incoming);
        
      if( idx == 0 ){           // principio de trama
        if( incoming != HEAD )  // trama incorrecta
          return;

        buff[idx] = incoming;
        idx++;
      }
      else if ( idx > 0 && idx < 5 ){ // 
        buff[idx++] = incoming;      //  
        if ( idx == 5 ){                // fin de trama
          if( buff[4] == TAIL )         // verificar que termine bien
            ejecutarComando();
          idx = 0;
        }
      }
  }
  
  delay(5);
}

void ejecutarComando(){
  //Serial.println("Ejectutando comando!!!");
  if ( buff[1] != MY_SLAVE_ID ) // el mensaje es para otro esclavo
    return;

  switch( buff[2] ){                      // ejecutar comando

    case UPDATE_STATUS:
        if(pendingToValidateTag){sendResponse(REQUEST_VALIDATE);}           //1 = solicito autorizar tag
        else if(pendingToConfirmShowerEnd){sendResponse(REQUEST_STORE_END);}//2 = ducha realizada, enviamos a master para que almacene datos
        else{
          if(showerStatus== blocked) sendResponse(SHOWER_BLOCKED);
          else if(showerStatus== forced) sendResponse(SHOWER_FORCED);
          else{sendResponse(RESPONSE_NO_ACTIVITY);}
        }
      break;
    case RESPONSE_ACCEPT:
        pendingToValidateTag = false;
        Serial.println("Pasamos a estado activo");
        lastActiveTag = getTagNumber(buffer);
        showerOn();
      break;
    case RESPONSE_REJECT:
        pendingToValidateTag = false;
        error();
      break;
    case RESPONSE_STORED_OK:
      pendingToConfirmShowerEnd =false;
      break;
    case CHANGE_STATUS:
        if(buff[3]==0){
          setShowerNormal();//showerStatus = inactive;
        }
        else if (buff[3]==1)
        {
          setShowerBlock();//showerStatus = blocked;
        }
        else if(buff[3]==2){
          setShowerBypass();//showerStatus = forced;
        }
        
      break;
    case CHANGE_SHOWER_TIME:
        if(buff[3]!=0){
          maxiumShowerTime = (buff[3] * 1000);
          sendResponse(RESPONSE_STORED_OK);
        }else{sendResponse(ERROR);}
      break;
    case CHANGE_YOUR_ID:
        if(buff[3]!=0){
            EEPROM.write(10,buff[3]);
            MY_SLAVE_ID = buff[3];
            sendResponse(RESPONSE_STORED_OK);
        }
      break;
    case CMD_LED_ON:                      // Encender Led
      digitalWrite( LED_BUILTIN, HIGH );  
      enviarRespuesta(200);
      digitalWrite(OK_LED,HIGH);
      break;

    case CMD_LED_OFF:                     // Apagar Led
      digitalWrite( LED_BUILTIN, LOW );
      digitalWrite(OK_LED,LOW);
      break;
    default:                              // Comando Inva'lido
      break;
  }
}

void enviarRespuesta( int x ){
  buff[0] = HEAD;
  buff[1] = MY_SLAVE_ID;
  buff[2] = idUser[0];
  buff[3] = idUser[1];
  buff[4] = (byte)x;    //accion
  buff[5] = TAIL;
  digitalWrite( RS485_PIN_MODE, HIGH ); // poner en modo Tx
  RS485.write( buff, 6 );               // transmitir mensaje
  RS485.flush();
  digitalWrite( RS485_PIN_MODE, LOW);   // poner en modo Rx
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
void setShowerBlock(){
  digitalWrite(OK_LED,LOW);
  digitalWrite(ERROR_LED,HIGH);
  digitalWrite(WATER,LOW);
  showerStatus = blocked;
  //delay(1000);
  return;
}
void setShowerBypass(){
  digitalWrite(OK_LED,HIGH);
  digitalWrite(ERROR_LED,LOW);
  digitalWrite(WATER,HIGH);
  showerStatus = forced;
  //delay(1000);
  return;
}
void setShowerNormal(){
  digitalWrite(OK_LED,LOW);
  digitalWrite(ERROR_LED,LOW);
  digitalWrite(WATER,LOW);
  showerStatus = inactive;
  //delay(1000);
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
  pendingToConfirmShowerEnd =true; 
}