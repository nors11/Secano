/* #include <Arduino.h> */
#include "functions.h"
#include <LiquidCrystal_I2C.h>
#include <Keypad.h> 
//#include "RTClib.h"
LiquidCrystal_I2C lcd(0x27,20,4);

char pinMaster[6];
int clockData[8]={0,0,0,0,0,0,0,0};
short pinMasterLength=6;

const byte rowsCount = 4;
const byte columsCount = 4;

char keys[rowsCount][columsCount] = {
   { '1','2','3', 'A' },
   { '4','5','6', 'B' },
   { '7','8','9', 'C' },
   { '*','0','#', 'D' }
};

const byte rowPins[rowsCount] = { 11, 10, 9, 8 };
const byte columnPins[columsCount] = { 7, 6, 5, 4 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, columnPins, rowsCount, columsCount);
//RTC_DS3231 rtc;

void setupConfig(){
  Serial.println("Start Setup");
  lcd.init();
  lcd.backlight();
  //rtc.adjust(DateTime(2024, 3, 12, 10, 0, 0));    // Set date time on compile  12 de Marzo de 2024 a las 10:00:00
  return;
}

void setFirstConfig(){
  int configDone = false;
  bool chekList[4]={false,false,false,false};
  lcdWriteData(3,0,"- SECANO 241 -");
  lcdWriteData(3,2,"PULSAR # PARA");
  lcdWriteData(1,3,"CONFIGURAR SISTEMA");
  char key = keypad.getKey();
  while(!configDone){
    char key = keypad.getKey();  
    if (key) {Serial.println(key);  } 
    if(key=='#'){
      /*
      chekList[0] = setMasterCode();
      if(chekList[0]){
        Serial.println("Fase 1 ok");
        lcd.clear();
        lcdWriteData(0,0," NO OLVIDE SU CLAVE");
        for(int x=0;x<3;x++){
          for(int n=0;n<pinMasterLength;n++){
            String keyStr(pinMaster[n]);
            lcdWriteData((7+n),2,keyStr);
            delay(150);
          }
          lcdWriteData((7),2,"        ");
        }
      }*/
      chekList[1] = setDateTime();
    }
  }
  

}
bool setMasterCode(){
      char key = keypad.getKey();
      char lastKey ='0';
      int position=0;
      short attempt = 0;
      bool equals= false;
      char codeA[6]={'0','0','0','0','0'};
      char codeB[6]={'0','0','0','0','0'};
      Serial.println("Set Master Code");
      lcd.clear();
      lcdWriteData(0,0,"NUEVA CLAVE MASTER:");
      lcdWriteData(7,2,"******");
      lcdWriteData(0,3,"D= borrar     #= OK");

      while((position < pinMasterLength) || (key != '#')){          //MasterCode must be 6 digits length
        key = keypad.getKey();
        if(position<6){
          lcdWriteData((position+7),2," ");
          delay(100);
          lcdWriteData((position+7),2,"*");
          delay(100);
        }else{
          String strP (pinMaster[5]);
          lcdWriteData((12),2," ");
          delay(100);
          lcdWriteData((12),2,strP);
          delay(100);
        }
        
        if(key){
          if(key!='#'){
            if(key!='D' && position<6){
              String keyStr(key);
              lcdWriteData((7+position),2,keyStr);
              pinMaster[position]=key;
              position++;
            }else if(position>0){
              position--;
              lcdWriteData((7+position),2,"*");
              pinMaster[position]=key;
            }
            Serial.println(key);
          }
          for(int n=0;n<pinMasterLength;n++){
              Serial.print(pinMaster[n]);
          }
        }
      }
  

      return true;
}
bool setDateTime(){
  char key = keypad.getKey();
  int position=5;
  bool done =false;
  lcd.clear();
  lcdWriteData(1,0,"AJUSTE FECHA-HORA");
  lcdWriteData(5,2,"YYYY-MM-DD");
  lcdWriteData(0,3,"D= borrar     #= OK");
  
  while(!done || (key != '#')){
    key = keypad.getKey();
    if(position<15){
      switch (position) {
              case 5:
                lcdWriteData(5,2," ");
                delay(100);
                lcdWriteData(5,2,"Y");
                delay(100);
                break;
              case 6:
                lcdWriteData(6,2," ");
                delay(100);
                lcdWriteData(6,2,"Y");
                delay(100);
                break;
              case 7:
                lcdWriteData(7,2," ");
                delay(100);
                lcdWriteData(7,2,"Y");
                delay(100);
                break;
              case 8:
                lcdWriteData(8,2," ");
                delay(100);
                lcdWriteData(8,2,"Y");
                delay(100);
                break;
              case 10:
                lcdWriteData(10,2," ");
                delay(100);
                lcdWriteData(10,2,"M");
                delay(100);
                break;
              case 11:
                lcdWriteData(11,2," ");
                delay(100);
                lcdWriteData(11,2,"M");
                delay(100);
                break;
              case 13:
                lcdWriteData(13,2," ");
                delay(100);
                lcdWriteData(13,2,"D");
                delay(100);
                break;
              case 14:
                lcdWriteData(14,2," ");
                delay(100);
                lcdWriteData(14,2,"D");
                delay(100);
                break;
              default:
              Serial.println("default");
                lcdWriteData(5,2,"******");
            }
    }
    if(key){   
      if(key =='D'){
        position --;
        if(position == 9){position=8;}
        else if(position == 12){position=11;}
        if(position<9){lcdWriteData(position,2,"Y");}
        else if(position ==10 || position ==11){lcdWriteData(position,2,"M");}
        else if(position ==13 || position ==14){lcdWriteData(position,2,"D");}
      }
      else if(position<=14){
        String strP (key);
        //REVISAR FORMATO MES no mas de 12 dia no mas de 31??
        if(checkers(position,key)){                     //Validations of format month(0-12)- day(1-31)  
          int intValue = String(key).toInt();
          lcdWriteData(position,2,strP);
          switch(position){                             //Store data in array to set clock later with these values
            case 5:
              clockData[0] = intValue;
              break;
            case 6:
              clockData[1] = intValue;
              break;
            case 7:
              clockData[2] = intValue;
              break;
            case 8:
              clockData[3] = intValue;
              break;
            case 10:
              clockData[4] = intValue;
              break;
            case 11:
              clockData[5] = intValue;
              break;
            case 13:
              clockData[6] = intValue;
              break;
            case 14:
              clockData[7] = intValue;
              done = true;
              break;
            default:
              break;
          }
          position++;
          if(position==9){position=10;}
          else if(position==12) {position=13;}
        }  
      }   
    }
  }
  Serial.println("FECHA-HORA CONFIGURADA");
  return true;
}

bool checkers(int p,char c){ 
  bool result = false;
  switch(p){
    case 10:
      switch (c) {
        case '0':
          result =true;
          break;
        case '1':
          result =true;
          break;
        default:
          break;
      }
      break;
    case 11:
      //Serial.print("pos =");Serial.print(p);Serial.print(" num: ");Serial.print(c);Serial.print(" digito anterior =");Serial.println(clockData[4]);
      if(clockData[4] == 1){
        switch (c) {
          case '0':
            result =true;
            break;
          case '1':
            result =true;
            break;
          case '2':
            result =true;
            break;
          default:
            return false;
            break;
        }
      }
      if(clockData[4] == 0){
        switch (c) {
          case '0':
            result =false;
            break;
          default:
            result =true;
            break;
        }
      }
      else result =true;
      break;
     
    case 13:
      switch (c) {
          case '0':
            result =true;
            break;
          case '1':
            result =true;
            break;
          case '2':
            result =true;
            break;
          case '3':
            result =true;
            break;
          default:
            break;
        }
      break;
    case 14:
      if(clockData[6] == 3){
        switch (c) {
          case '0':
            result =true;
            break;
          case '1':
            result =true;
            break;
          default:
            return false;
            break;
        }
      }else if(clockData[6] == 0){
        switch (c) {
          case '0':
            result =false;
            break;
          default:
            result =true;
            break;
        }
      }
      else{result = true;}
      break;
    default:
      result = true;
      break;
  } 
  return result;
}

void changeMasterCode(int *codeNew){
  int nuevo = 1234;
  *codeNew = nuevo;
}

void lcdWriteData(int column,int row,String text){
  lcd.setCursor(column, row);
  lcd.print(text);
}