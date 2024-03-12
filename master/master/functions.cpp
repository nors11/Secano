/* #include <Arduino.h> */
#include "functions.h"
#include <LiquidCrystal_I2C.h>
#include <Keypad.h> 
#include "RTClib.h"
LiquidCrystal_I2C lcd(0x27,20,4);

char pinMaster[6];
int clockData[8]={0,0,0,0,0,0,0,0};       //have the date values that the user introduce, must be convert to clock format
int clockTime[6]={0,0,0,0,0,0};           //have the time values that the user introduce, must be convert to clock format
int dateTimeToClock[6]={2024,3,12,11,0,0};//year,month,day,hour,minutes,seconds-- Valid data to send to clock with the correct format
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
RTC_DS3231 rtc;

void setupConfig(){
  Serial.println("Start Setup");
  lcd.init();
  lcd.backlight();
  if (!rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    while (1);
  }
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(2024, 3, 12, 7, 34, 0));
  }
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
      chekList[1] = setDate();
      
      chekList[2] = setTime();
      setDateTimetoClock();

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

bool setDate(){
  Serial.println("AJUSTE DE FECHA");
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
        if(dateCheckers(position,key)){                     //Validations of format month(0-12)- day(1-31)  
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
  lcdWriteData(0,2,"   FECHA GUARDADA");
  lcdWriteData(0,3,"                    ");
  delay(1000);
  Serial.println("FECHA CONFIGURADA");
  return true;
}

bool setTime(){
  bool done =false;
  char key = keypad.getKey();
  int position=8;
  Serial.println("AJUSTE DE HORA");
  lcdWriteData(0,2,"                    ");
  lcdWriteData(8,2,"HH:MM");
  lcdWriteData(0,3,"D= borrar     #= OK");
  while(!done || (key != '#')){
    key = keypad.getKey();
    if(position<13){
      switch(position){
        case 8:
          lcdWriteData(position,2," ");
          delay(100);
          lcdWriteData(position,2,"H");
          delay(100);
          break;
        case 9:
          lcdWriteData(position,2," ");
          delay(100);
          lcdWriteData(position,2,"H");
          delay(100);
          break;
        case 11:
          lcdWriteData(position,2," ");
          delay(100);
          lcdWriteData(position,2,"M");
          delay(100);
          break;
        case 12:
          lcdWriteData(position,2," ");
          delay(100);
          lcdWriteData(position,2,"M");
          delay(100);
          break;
        default:
          break;
      }
    }
    if(key){
        String strKey (key);
        int intValue = String(key).toInt();
        if(key=='D' && position>0){
          position --;
          if(position==10) position =9;
          switch (position){
            case 8:
              lcdWriteData(8,2,"H");
              break;
            case 9:
              lcdWriteData(9,2,"H");
              break;
            case 11:
              lcdWriteData(11,2,"M");
              break;
            case 12:
              lcdWriteData(12,2,"M");
              break;
            default:
              break;
          }
        }
        else if((timeCheckers(position,key)) && position<13){ //validate format HH -> 0-24  MM 0-60
            lcdWriteData(position,2,strKey);
            switch(position){                                 //Save data in array for convert later to send to clock
              case 8:
                clockTime[0]= intValue;
                break;
              case 9:
                clockTime[1]= intValue;
                break;
              case 11:
                clockTime[2]= intValue;
                break;
              case 12:
                clockTime[3]= intValue;
                done =true;
                break;
            }
            position ++;
            if(position==10) position =11;
        }
    }
  }
  lcdWriteData(0,2,"    HORA GRABADA");
  lcdWriteData(0,3,"                    ");
  delay(1000);
  Serial.println("HORA CONFIGURADA");
}

bool dateCheckers(int p,char c){ 
  if(c=='*'||c=='#'||c=='A'||c=='B'||c=='C'||c=='D') return false;
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

bool timeCheckers(int p,char c){
  bool result =false;
  if(c=='*'||c=='#'||c=='A'||c=='B'||c=='C'||c=='D') return false;
  switch(p){
    case 8:
      switch(c){
        case '0':
        case '1':
        case '2':
          result = true;
        default:
          break;
      }
      break;
    case 9:
      if(clockTime[0]==2){
        switch(c){
          case '0':
          case '1':
          case '2':
          case '3':
            result = true;
          default:
            break;
        }
      }else{result =true;}
    
      break;
    case 11:
      switch(c){
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
          result= true;
          break;
        default:
          break;
      }
      break;
    case 12:
      result =true;
      break;
    default:
      break;
  } 
  return result;
}

void changeMasterCode(int *codeNew){
  int nuevo = 1234;
  *codeNew = nuevo;
}

int convertDataTime(int date[8],int time[6]){
  //----Year
      int yyA = (date[0]*10)+date[1];
      int yyB = (date[2]*10)+date[3];
      int yyyy= (yyA*100)+yyB;
      //Serial.print("year: ");Serial.print(yyyy);
      dateTimeToClock[0]=yyyy;

  //----Month
      int mm =0;
      mm=(date[4]*10)+date[5];
      //Serial.print(" month: ");Serial.print(mm);
      dateTimeToClock[1]=mm;

  //----Day
      int dd =0;
      dd=(date[6]*10)+date[7];
      //Serial.print(" day: ");Serial.println(dd);
      dateTimeToClock[2]=dd;
  //--------

}

bool setDateTimetoClock(){
  Serial.println("CAMBIO DE HORA SISTEMA");
  convertDataTime(clockData,clockTime);
  for(int n =0;n<6;n++){
    Serial.print(dateTimeToClock[n]);Serial.print(" ");
  }
  rtc.adjust(DateTime(dateTimeToClock[0], dateTimeToClock[1], dateTimeToClock[2], dateTimeToClock[3], dateTimeToClock[4], dateTimeToClock[5]));
  return true;
}

void showDateTime(){
  char buf1[20];
  DateTime now = rtc.now();
  sprintf(buf1, "%02d:%02d:%02d  %02d/%02d/%02d",  now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year()); 
  lcdWriteData(0,0,buf1);
}

void lcdWriteData(int column,int row,String text){
  lcd.setCursor(column, row);
  lcd.print(text);
}