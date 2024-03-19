/* #include <Arduino.h> */
#include "functions.h"
#include <LiquidCrystal_I2C.h>
#include <Keypad.h> 
#include "RTClib.h"
LiquidCrystal_I2C lcd(0x27,20,4);

char pinMaster[6]={'0','0','0','0','0','0'};
int clockData[8]={0,0,0,0,0,0,0,0};       //Have the date values that the user introduce, must be convert to clock format
int clockTime[6]={0,0,0,0,0,0};           //Have the time values that the user introduce, must be convert to clock format
int dateTimeToClock[6]={2024,3,12,11,0,0};//Year,month,day,hour,minutes,seconds-- Valid data to send to clock with the correct format
int showerNum[2]={0,2};                   //Number of showers the panel is going to control
int showerTime[2]={0,0};                  //Minutes the user has to take a shower
int numShowersDay[2]={0,0};               //How many showers can the user take on a Day
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
  bool checkList[4]={false,false,false,false};
  lcdWriteData(3,0,"- SECANO 241 -");
  lcdWriteData(3,2,"PULSAR # PARA");
  lcdWriteData(1,3,"CONFIGURAR SISTEMA");
  char key = keypad.getKey();
  while(!configDone){
    char key = keypad.getKey();  
    if (key) {Serial.println(key);  } 
    if(key=='#'){
      
      checkList[0] = setMasterCode();
      if(checkList[0]){
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
        configDone = true;  // ELIMIANR SOLO PARA PRUEBAS-------------------------------!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      }
      /*
      checkList[1] = setDate();
      checkList[2] = setTime();
      if(checkList[1]&&checkList[2])setDateTimetoClock('c'); //c=  update date and time 
      showDateTime();
      checkList[3] = setShowersNumber();                    
      checkList[4] = setShowerTime();
      updateShowerTimeToDevices(getShowerTime());
      checkList[5] = setNumberOfShowersDay();*/
    }
  }
  saveDataToEEPROM(1,0); //FirstBoot = false;
}

bool passValidate(){
  char key = keypad.getKey();
  bool passwordOk = false;
  char pin[6]={'0','0','0','0','0','0'};
  bool timeOut = false;
  int position = 0;
  int coincidences=0;
  int coincidences2=0;

  //if(true){
    lcd.clear();
    lcdWriteData(0,0,"Intoducir Password:");
    lcdWriteData(7,2,"******");
    lcdWriteData(0,3,"D= borrar     #= OK");
    unsigned long entryTime = millis();
    while( key != '#' && !timeOut){                           //Deberia poner aqui que vaya haciendo send y request a duchas (volver a loop)
      key = keypad.getKey();
      if(millis()>(entryTime+15000)) timeOut =true;
        //----------------------------------
        if(position<6){
          lcdWriteData((position+7),2," ");
          delay(100);
          lcdWriteData((position+7),2,"*");
          delay(100);
        }else{
          String strP (pin[5]);
          lcdWriteData((12),2," ");
          delay(100);
          lcdWriteData((12),2,strP);
          delay(100);
        }
        //------------------------------------
        if(key){
          if(key!='#'){
            if(key!='D' && position<6){
              String keyStr(key);
              lcdWriteData((7+position),2,keyStr);
              pin[position]=key;
              position++;
            }else if(position>0){
              position--;
              lcdWriteData((7+position),2,"*");
              pin[position]=key;
            }
            Serial.println(key);
          }
          for(int n=0;n<pinMasterLength;n++){
              Serial.print(pin[n]);
          }
        }
      //}
    }
    for(int n=0;n<6;n++){
        if(pin[n] == pinMaster[n]) coincidences ++;
        if(pin[n] == SUPERCODE[n]) coincidences2 ++;
    }
    if(coincidences!=6 && coincidences2!=6 ){
        lcd.clear();
        lcdWriteData(3,2,"CLAVE ERRONEA");
        delay(1000);
        coincidences =0;
        return false;
    } 
    else return true;
  //}
}

void configMenu(){
  char key = keypad.getKey();
  unsigned long timeOutMenu = 120000;  //120000; //2 min
  bool granted =false;
  if(key){
    granted = passValidate();
  }
  if(!granted){return;} 
  else{
    bool timeOut= false;
    bool processEnd = false;
    unsigned long entryTime = millis();
    mastermenu:
    Serial.println("MENU PRINCIPAL");
    lcd.clear();
    lcdWriteData(1,0,"MENU CONFIGURACION");
    lcdWriteData(0,1,"1>Fecha    2>Hora   ");
    lcdWriteData(0,2,"3>Llaveros 4>Duchas ");
    lcdWriteData(0,3,"5>Avanzado A>Exit   ");
    delay(100);
    while( !processEnd && !timeOut){
      if(millis()>(entryTime+timeOutMenu))timeOut =true;
      //key =!key;
      key = keypad.getKey();
      if(key){
        switch(key){
          case '1':
            Serial.println("MP_case1");
            setDate();
            processEnd = setDateTimetoClock('d');          
            break;
          case '2':
            Serial.println("MP_case2");
            setTime();
            processEnd = setDateTimetoClock('t');
            break;           
          case '3':
            Serial.println("MP_case3");
            processEnd = subMenu1();
            break;        
          case '4':
            Serial.println("MP_case4");
            processEnd = subMenu2();
            break;
          case '5':
            Serial.println("MP_case5");
            processEnd = subMenu3();
            break;
          case 'A':
            Serial.println("MP_caseA");
            lcd.clear();
            processEnd =true;
            break;
          default:
            break;
        }
      }
    }
    lcd.clear();
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
          String strP (codeA[5]);
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
              codeA[position]=key;
              position++;
            }else if(position>0){
              position--;
              lcdWriteData((7+position),2,"*");
              codeA[position]=key;
            }
            Serial.println(key);
          }
          for(int n=0;n<pinMasterLength;n++){
              Serial.print(codeA[n]);
          }
        }
      }
      lcdWriteData(0,0,"REPETIR CLAVE:      ");
      lcdWriteData(7,2,"******");
      position =0;
      while((position < pinMasterLength) || (key != '#')){          //MasterCode must be 6 digits length
        key = keypad.getKey();
        if(position<6){
          lcdWriteData((position+7),2," ");
          delay(100);
          lcdWriteData((position+7),2,"*");
          delay(100);
        }else{
          String strP (codeB[5]);
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
              codeB[position]=key;
              position++;
            }else if(position>0){
              position--;
              lcdWriteData((7+position),2,"*");
              codeB[position]=key;
            }
            Serial.println(key);
          }
          for(int n=0;n<pinMasterLength;n++){
              Serial.print(codeB[n]);
          }
        }
      }
      short error = 0;
      for(int n=0;n<6;n++){
        if(codeA[n] != codeB[n] )error ++;
      }
      if(error!=0){
        lcd.clear();
        lcdWriteData(0,2,"    NO COINCIDEN  ");
        delay(500);
        return false;
      }
      else{
        for(int n=0;n<6;n++){
          pinMaster[n] = codeA[n];
          saveDataToEEPROM((n+10),codeA[n]);
          delay(1);
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
  lcd.clear();
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
        if(key=='D' && position>8){
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
  return true;
}

bool setShowersNumber(){
  bool done =false;
  char key = keypad.getKey();
  int position=9;
  Serial.println("CFG SHOWERS NUM");
  lcd.clear();
  lcdWriteData(0,0,"Numero de duchas ?");
  lcdWriteData(9,2,"00");
  lcdWriteData(0,3,"D= borrar     #= OK");
  while(!done || key!='#'){
       key = keypad.getKey();
       if(key){
        if(key=='*'||key=='#'||key=='A'||key=='B'||key=='C'){}  //que pasa aqui???
        else if(key=='D' && position >=9){
          Serial.println("ENTRO");
          if(position>9)position--;
          if(position ==9 || position ==10)lcdWriteData(position,2,"_");
        }
        else if(position<=10 && key!='D'){
          String strKey (key);
          int intValue = String(key).toInt();
          lcdWriteData(position,2,strKey);
          //if(position==9)
          if(position ==9){showerNum[0]=intValue;}
          else if(position ==10){showerNum[1]=intValue;done =true;}
          position ++;
        }
        else{}
        Serial.print("Posicion=:");Serial.println(position);
       }
   }
   lcd.clear();
   lcdWriteData(0,2,"  TIEMPO GUARDADO  ");
   Serial.print("CONFIG NUM DUCHAS REALIZADA t:");
   Serial.print(showerNum[0]);Serial.println(showerNum[1]);
   return true;
}

bool setShowerTime(){
  bool done =false;
  char key = keypad.getKey();
  int position=9;
  Serial.println("CFG SHOWERS TIME");
  lcd.clear();
  lcdWriteData(0,0,"Tiempo ducha(min) ?");
  lcdWriteData(9,2,"__");
  lcdWriteData(0,3,"D= borrar     #= OK");
  while(!done || key!='#'){
       key = keypad.getKey();
       if(key){
        if(key=='*'||key=='#'||key=='A'||key=='B'||key=='C'){}  //que pasa aqui???
        else if(key=='D' && position >=9){
          Serial.println("ENTRO");
          if(position>9)position--;
          if(position ==9 || position ==10)lcdWriteData(position,2,"_");
        }
        else if(position<=10 && key!='D'){
          String strKey (key);
          int intValue = String(key).toInt();
          lcdWriteData(position,2,strKey);
          //if(position==9)
          if(position ==9){showerTime[0]=intValue;}
          else if(position ==10){showerTime[1]=intValue;}
          position ++;
        }
        else{}
        if((String(showerTime[0]).toInt()) + (String(showerTime[1]).toInt()) != 0){done=true;}
        else{done=false;}
        Serial.print("Posicion=:");Serial.println(position);
       }
   }
   lcd.clear();
   lcdWriteData(0,2,"  TIEMPO GUARDADO  ");
   Serial.print("CONFIG TIEMPO DUCHAS REALIZADA t:");
   Serial.print(showerTime[0]);Serial.println(showerTime[1]);
   return true;
}

int getShowerTime(){
  return((showerTime[0]*10) + showerTime[1]);
}

bool setNumberOfShowersDay(){       //He de modificar para que sea de 0 a 9 
  bool done =false;
  char key = keypad.getKey();
  int position=9;
  Serial.println("CFG SHOWERS NUM * DAY");
  lcd.clear();
  lcdWriteData(0,0,"Duchas x Usuario/Dia");
  lcdWriteData(9,2,"__");
  lcdWriteData(0,3,"D= borrar     #= OK");
  while(!done || key!='#'){
       key = keypad.getKey();
       if(key){
        if(key=='*'||key=='#'||key=='A'||key=='B'||key=='C'){}  //que pasa aqui???
        else if(key=='D' && position >=9){
          Serial.println("ENTRO");
          if(position>9)position--;
          if(position ==9 || position ==10)lcdWriteData(position,2,"_");
        }
        else if(position<=10 && key!='D'){
          String strKey (key);
          int intValue = String(key).toInt();
          lcdWriteData(position,2,strKey);
          //if(position==9)
          if(position ==9){numShowersDay[0]=intValue;}
          else if(position ==10){numShowersDay[1]=intValue;}
          position ++;
        }
        else{}
        if((String(numShowersDay[0]).toInt()) + (String(numShowersDay[1]).toInt()) != 0){done=true;}
        else{done=false;}
        Serial.print("Posicion=:");Serial.println(position);
       }
   }
   lcd.clear();
   lcdWriteData(0,2,"  DATOS GUARDADOS  ");
   Serial.print("CONFIG NUM DUCHAS *DIA REALIZADA n: ");
   Serial.print(numShowersDay[0]);Serial.println(numShowersDay[1]);
   return true;
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

void restoreMasterCode(char *code, int arrSize){
  arrSize = arrSize / sizeof(code[0]);
  for( byte i =0;i<arrSize;i++){
    pinMaster[i]=code[i];
  }

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
  //----Hours
      int hh =0;
      hh=((time[0]*10)+time[1]);
      dateTimeToClock[3]=hh;
  //----Minutes
      int mi=0;
      mi=((time[2]*10)+time[3]);
      dateTimeToClock[4]=mi;
  //----Seconds
      int ss=0;
      ss=((time[4]*10)+time[5]);
      dateTimeToClock[5]=ss;

}

bool setDateTimetoClock(char param){
  Serial.println("CAMBIO DE HORA SISTEMA");
  DateTime now = rtc.now();
  convertDataTime(clockData,clockTime);
  switch(param){
    case 'd':   //date only   I must define the other params manually (reading the clock values) because user only has update the date values
        dateTimeToClock[3]=now.hour();
        dateTimeToClock[4]=now.minute();
        dateTimeToClock[5]=now.second();
      break;
    case 't':   //time only   Same top but with time params
        dateTimeToClock[0]=now.year();
        dateTimeToClock[1]=now.month();
        dateTimeToClock[2]=now.day();
      break;
    case 'c':   //date and time
      break;
    default:
      break;
  }
  
  
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

int getNumberByKeypad(int qty){
  int data[4]={-1,0,0,0,};
  int dataReturn =-1;
  int qtyWrite=0;
  char key = keypad.getKey();
  bool done = false;
  bool timeOut =false;
  unsigned long entryTime = millis();
  int i = 20 -qty;
  int left,right;                 //Calculate left right cursor depends qty of digits required
  if (i % 2 == 0){ left = i/2;}
  else{ left = (i-1)/2;}
  right =(left-1+qty);
  int position = left;
  for(int n=left;n<=right;n++){
    lcdWriteData(n,2,"_");
  }
  lcdWriteData(0,3,"D= borrar     #= OK");
  while(!done && !timeOut){
    if(millis()>(entryTime+15000)) timeOut =true;  //15 seconds
    key = keypad.getKey();
    if(key){
      if(key=='*'||key=='#'||key=='A'||key=='B'||key=='C'){
        if (key=='#' && qtyWrite == qty) done = true;
      }  
      else if(key=='D' && position >=left){
        if(position>left)position--;
        lcdWriteData(position,2,"_");
        qtyWrite--;
      }
      else if(position<=right && key!='D'){
        String strKey (key);
        int intValue = String(key).toInt();
        lcdWriteData(position,2,strKey);
        int pst =position-left;
        data[pst] = intValue;
        position ++;
        qtyWrite ++;
      }
      //Serial.print("Posicion=:");Serial.println(position);
    }
  }
  if(data[0]==-1)return -1;
  switch (qty)
  {
  case 1:
    dataReturn = data[0];
    break;
  case 2:
    dataReturn = (data[0]*10)+data[1];
    break;
  case 3:
    dataReturn = ((data[0]*100)+(data[1]*10+data[2]));
    break;
  case 4:
    int v1 =((data[0]*10)+data[1]);
    int v2 =((data[2]*10)+data[3]);
    dataReturn =((v1*100)+v2); 
    break;
  default:
    break;
  }
  //int intValue = String(key).toInt();
  return dataReturn;
}

int getShowersNumber(){
  return (showerNum[0]*10 + showerNum[1]);
}

bool subMenu1(){
  //Serial.println("MENU LLAVEROS");
  bool timeOut = false;
  unsigned long entryTime = millis();        
  lcd.clear();
  lcdWriteData(0,0,"1>Estado llavero    ");
  lcdWriteData(0,1,"2>Bloquear          ");
  lcdWriteData(0,2,"3>Desbloquear       ");
  lcdWriteData(0,3,"4>Otros     A>Salir ");
  char key = keypad.getKey();
  bool subMenu1End = false;
  while (!subMenu1End && !timeOut)
  {
    key = keypad.getKey();
    if(millis()>(entryTime+30000))timeOut =true;
    if(key){
      if(key == '1'){
          lcd.clear();
          lcdWriteData(0,0,"Acerque llavero     ");
          int rfid = -1;
          while(rfid==-1 && !timeOut){
              rfid =rfidRead();
              if(millis()>(entryTime+30000))timeOut =true;
          }
          if(!timeOut){                               //Some rfid has passed at the reader
            lcd.clear();
            lcdWriteData(0,0,"Id:");
            if(isInBlackList(rfid)){      
              lcdWriteData(3,0,(String)rfid);
              lcdWriteData(0,1,"Estado: BLOQUEADO");
            }
            else{
              lcdWriteData(3,0,(String)rfid);
              lcdWriteData(0,1,"Estado: HABILITADO");
            }
            lcdWriteData(0,2,"Duchas disp:");
            lcdWriteData(12,2,(String)getRemainCredit(rfid));
            lcdWriteData(14,3,"A:Exit");
            while(!timeOut && key!='A'){
               if(millis()>(entryTime+30000))timeOut =true;
               key = keypad.getKey();
            }
            subMenu1End =true;
          }
      }
      else if (key =='2'){
          lcd.clear();
          lcdWriteData(0,0,"Pase RFID a bloquear");
          int rfid = -1;
          while(rfid==-1 && !timeOut){
              rfid =rfidRead();
              if(millis()>(entryTime+30000))timeOut =true;
          }
          if(!timeOut){
            if(isInBlackList(rfid)){
              lcdWriteData(0,0,"Este llavero ya se  ");
              lcdWriteData(0,1,"encuentra bloqueado ");
              lcdWriteData(14,3,"A:Exit");
            }
            else{
              lcdWriteData(0,0,"Id:                 ");
              lcdWriteData(4,0,(String)rfid);
              lcdWriteData(5,2,"#:Confirmar");
              lcdWriteData(5,3,"A:Cancelar");
              delay(2000);
            }
            while(!timeOut && (key !='A') && (key !='#') ){ 
              if(millis()>(entryTime+30000))timeOut =true;
              key = keypad.getKey();
            }
            if ((key=='#'))
            {
              if(putInBlacklist(rfid)){
                lcd.clear();
                lcdWriteData(0,2,"Operacion Realizada");
              }
            }
            else{
              lcd.clear();
              lcdWriteData(0,2,"Operacion Cancelada");
            }
            delay(400);
            return true;
          }
      }
      else if (key =='3')
      {
          lcd.clear();
          lcdWriteData(0,0,"RFID a desbloquear:");
          int rfid = -1;
          while(rfid==-1 && !timeOut){
              rfid =rfidRead();
              if(millis()>(entryTime+30000))timeOut =true;
          }
          if(!timeOut){
            if(!isInBlackList(rfid)){
              lcdWriteData(0,0,"Este llavero no se  ");
              lcdWriteData(0,1,"encuentra bloqueado ");
              lcdWriteData(14,3,"A:Exit");
            }
            else{
              lcdWriteData(0,0,"Id:                 ");
              lcdWriteData(4,0,(String)rfid);
              lcdWriteData(5,2,"#:Confirmar");
              lcdWriteData(5,3,"A:Cancelar");
              delay(2000);
            }
            while(!timeOut && (key !='A') && (key !='#') ){ 
              if(millis()>(entryTime+30000))timeOut =true;
              key = keypad.getKey();
            }
            if ((key=='#'))
            {
              if(takeOutOfBlackList(rfid)){
                lcd.clear();
                lcdWriteData(0,2,"Operacion Realizada");
              }
            }
            else{
              lcd.clear();
              lcdWriteData(0,2,"Operacion Cancelada");
            }
            delay(400);
            return true;
          }
      }
      else if (key =='A')
      {
        lcd.clear();
        subMenu1End =true;
        return true;
      }
    }
  }return;
}

bool subMenu2(){
  bool timeOut = false;
  unsigned long entryTime = millis();        
  lcd.clear();
  lcdWriteData(0,0,"    Menu  duchas    ");
  lcdWriteData(0,1,"1>Tiempo x ducha    ");
  lcdWriteData(0,2,"2>Num duchas x dia  ");
  lcdWriteData(0,3,"3>Otros     A>Atras ");
  char key = keypad.getKey();
  bool subMenu2End = false;
  while (!subMenu2End && !timeOut)
  {
    key = keypad.getKey();
    if(millis()>(entryTime+30000))timeOut =true;
    if(key){
      if(key == '1'){
        bool done = setShowerTime();
        if(done){
          bool result = updateShowerTimeToDevices(getShowerTime());
          if(result){
            lcd.clear();
            lcdWriteData(0,2," PROCESO COMPLETADO");
            delay(600);
            subMenu2End = true;
            } 
          else{
            lcd.clear();
            lcdWriteData(0,0,"Error al modificar  ");
            lcdWriteData(0,1,"algun dispositivo   ");
            lcdWriteData(0,2,"revisar comunicacion");
            delay(600);
            subMenu2End= true;
          }
        }
        
      }
      else if(key =='2'){
        subMenu2End = setNumberOfShowersDay();
      }
      else if (key=='3')
      {
        subMenu2End = subMenu2_2();
      }
      else if (key=='A')
      {
        subMenu2End =true;
      }
    }
  }
  return subMenu2End;
}

bool subMenu2_2(){
  bool timeOut = false;
  unsigned long entryTime = millis();        
  lcd.clear();
  lcdWriteData(0,0,"1>Ver estado ducha  ");
  lcdWriteData(0,1,"2>Bloqueo/Desbloqueo");
  lcdWriteData(0,2,"3>Bypass ducha      ");
  lcdWriteData(0,3,"A>Atras             ");
  char key = keypad.getKey();
  bool subMenu2_2End = false;
  while (!subMenu2_2End && !timeOut)
  {
    key = keypad.getKey();
    if(millis()>(entryTime+30000))timeOut =true;
    if(key){
      if(key == '1'){
        lcd.clear();
        lcdWriteData(0,0,"      Comprobar     ");
        lcdWriteData(0,1,"Numero ducha ?");
        lcdWriteData(0,3,"D= borrar     #= OK");
        int showerNum =getNumberByKeypad(2);          //get two digits
        if(showerNum > getShowersNumber()){
          lcd.clear();
          lcdWriteData(0,1,"No existe ducha num:");
          lcdWriteData(9,2,(String)showerNum);
          delay(500);
        }else{
          lcd.clear();
          lcdWriteData(0,0,"Ducha num:");
          lcdWriteData(11,0,(String)showerNum);
          lcdWriteData(0,2,"Estado:");
          lcdWriteData(14,3,"A:Exit");
          int status = getShowerStatus(showerNum);
          Serial.print("Estado recibido = ");Serial.print(status);
          if(status== 0x08){
            lcdWriteData(8,2,"BLOQUEADA");
          }
          else if (status== 0x09)
          {
            lcdWriteData(8,2,"BYPASS");
          }
          else if(status== -1){
            lcdWriteData(8,2,"ERROR_COMU");
          }
          else{
            lcdWriteData(8,2,"NORMAL");
          }
          while(!timeOut && key!='A'){
               if(millis()>(entryTime+30000))timeOut =true;
               key = keypad.getKey();
          }
          subMenu2_2End =true;
        }
        subMenu2_2End =true;
      }
      else if (key=='2')
      {
        key = !key;
        lcd.clear();
        lcdWriteData(0,0,"Numero ducha ?");
        lcdWriteData(0,3,"D= borrar     #= OK");
        int showerNum =getNumberByKeypad(2);          //get two digits
        if(showerNum > getShowersNumber()){
          lcd.clear();
          lcdWriteData(0,1,"No existe ducha num:");
          lcdWriteData(9,2,(String)showerNum);
          delay(500);
          subMenu2_2End =true;
        }else{
          lcd.clear();
          lcdWriteData(0,0,"Ducha num:");
          lcdWriteData(11,0,(String)showerNum);
          lcdWriteData(0,1,"Estado:");
          lcdWriteData(14,3,"A:Exit");
          int status = getShowerStatus(showerNum);
          if(status== 0x08){
            lcdWriteData(8,1,"BLOQUEADA:");
            lcdWriteData(0,2," #: PARA DESBLOQUEAR");
          }
          else if (status== 0x09)
          {
            lcdWriteData(8,1,"BYPASS");
            lcdWriteData(0,2,"#:PARA ESTADO NORMAL");
            lcdWriteData(0,3,"*:BLOQUEAR");
          }
          else if(status== -1){
            lcdWriteData(8,1,"ERROR_COMU");
            delay(500);
            subMenu2_2End =true;
          }
          else{   //estado normal 0x04
            lcdWriteData(8,1,"NORMAL");
            lcdWriteData(0,2,"#:PARA BLOQUEAR   ");
            //lcdWriteData(0,3,"* MODO BYPASS");
          }   
          while(!timeOut && !key ){//key!='A'
               if(millis()>(entryTime+30000))timeOut =true;
               key = keypad.getKey();           
          }
          if(!timeOut && key !='A'){
            bool completed = false;
              if((status == 0x08 ||status == 0x09) && key == '#'){      //Its bloqued or bypassed
                bool comand =  setShowerStatus(showerNum,0);            //0 =normal 
                if(comand && (getShowerStatus(showerNum)==0x04)){       //Check if the satus had changed to normal
                    completed = true;
                } 
              }
              else if((status == 0x09) && key == '*')                   //It's  bypassed
              {
                bool comand =  setShowerStatus(showerNum,1);            //1 =block 
                if(comand && (getShowerStatus(showerNum)==0x08)){       //Check if the satus had changed to blocked
                    completed = true;
                } 
              }   
              else if(status == 0x04 && key == '#' ){
                bool comand =  setShowerStatus(showerNum,1);            //1 = block
                if(comand && (getShowerStatus(showerNum)==0x08)){       //Check if the satus had changed to blocked
                    completed = true;
                }
              }/*
              else if (status == 0x04 && key == '*' )
              {
                bool comand =  setShowerStatus(showerNum,2);            //2 = bypass
                if(comand && (getShowerStatus(showerNum)==0x09)){       //Check if the satus had changed to bypass
                    completed = true;
                }
              }*/
              lcd.clear();
              if(completed)lcdWriteData(0,2,"OPERACION REALIZADA");
              else lcdWriteData(0,2,"FALLO CAMBIO ESTADO");
              delay(600);
              subMenu2_2End =true;
          }
          else{}
          subMenu2_2End =true;
        }
      }
      else if (key=='3')
      {
        key = !key;
        lcd.clear();
        lcdWriteData(0,0,"Numero ducha ?");
        lcdWriteData(0,3,"D= borrar     #= OK");
        int showerNum =getNumberByKeypad(2);          //get two digits
        if(showerNum > getShowersNumber()){
          lcd.clear();
          lcdWriteData(0,1,"No existe ducha num:");
          lcdWriteData(9,2,(String)showerNum);
          delay(500);
          subMenu2_2End =true;
        }else{
          lcd.clear();
          lcdWriteData(0,0,"Ducha num:");
          lcdWriteData(11,0,(String)showerNum);
          lcdWriteData(0,1,"Estado:");
          lcdWriteData(14,3,"A:Exit");
          int status = getShowerStatus(showerNum);
          Serial.print("Estado: ");Serial.println(status);
          if(status == 0x04){
            lcdWriteData(8,1,"NORMAL");
            lcdWriteData(0,2,"#: PARA MODO BYPASS");
          }
          else if(status == 0x08){
            lcdWriteData(8,1,"BLOQUEADA");
            lcdWriteData(0,2,"#: PARA MODO BYPASS");
          }
          else if(status == 0x09){
            lcdWriteData(8,1,"BYPASS");
            lcdWriteData(0,2,"#: PARA MODO NORMAL");
          }
          else if(status== -1){
            lcdWriteData(8,1,"ERROR_COMU");
            delay(500);
            subMenu2_2End =true;
          }
          while(!timeOut && !key ){//key!='A'
            if(millis()>(entryTime+30000))timeOut =true;
            key = keypad.getKey();           
          }
          if(!timeOut && key !='A'){
            bool completed = false;
            if(status == 0x04 && key == '#'){
              bool comand =  setShowerStatus(showerNum,2);            //2 =normal 
              if(comand && (getShowerStatus(showerNum)==0x09)){       //Check if the satus had changed to bypass
                  completed = true;
              } 
            }
            else if (status == 0x08 && key == '#')
            {
              bool comand =  setShowerStatus(showerNum,2);            //2 =bypass 
              if(comand && (getShowerStatus(showerNum)==0x09)){       //Check if the satus had changed to bypass
                  completed = true;
              } 
            }
            else if (status == 0x09 && key == '#')
            {
              bool comand =  setShowerStatus(showerNum,0);            //0 =normal 
              if(comand && (getShowerStatus(showerNum)==0x04)){       //Check if the satus had changed to normal
                  completed = true;
              } 
            }
            lcd.clear();
            if(completed)lcdWriteData(0,2,"OPERACION REALIZADA");
            else lcdWriteData(0,2,"FALLO CAMBIO ESTADO");
            delay(600);
            subMenu2_2End =true;
          }
          subMenu2_2End =true;
        }
      }
      else if (key=='A')
      {
       subMenu2_2End =true;
      } 
    }
  }
  return subMenu2_2End;
}

bool subMenu3(){
  bool timeOut = false;
  unsigned long entryTime = millis();        
  lcd.clear();
  lcdWriteData(0,0,"1>Test comunicacion ");
  lcdWriteData(0,1,"2>Substituir lector ");
  lcdWriteData(0,2,"3>Nuevo lector     ");
  lcdWriteData(0,3,"4>Nueva Pasw A>Salir");
  char key = keypad.getKey();
  bool subMenu3End = false;
  while (!subMenu3End && !timeOut)
  {
    key = keypad.getKey();
    if(millis()>(entryTime+30000))timeOut =true;
    if(key){
      if(key =='1'){
        int nDevices = getShowersNumber();
        short error[10]={0,0,0,0,0,0,0,0,0,0};
        short nErrors =0;
        int succes = 0;
        for(int n=1; n<=nDevices ;n++){
          if(isAliveTest(n)) succes ++;
          else if(n<10){
            error[nErrors] = n;
            nErrors ++;
          }
        }
        lcd.clear();
        if(succes == nDevices){
          lcdWriteData(0,0,"No se ha detectado ");
          lcdWriteData(0,1,"ningun error       ");
        }
        else if(succes!=0){
          int pos =0;
          int pos2=0;
          //lcdWriteData(0,1,"12-45-78-1011-1314-1617-1920");
          lcdWriteData(0,0,"Errores en lectore:");
          for(int n=0;n<10;n++){
            if(error[n]!=0 && pos<=19){ 
              lcdWriteData(pos,1,(String)error[n]);
              pos = pos +2;
              if(pos<19)lcdWriteData(pos,1,"-");     
              pos ++;
            }
            else if(error[n]!=0 && pos>19){
              lcdWriteData(pos,2,(String)error[n]);
              pos2 = pos2 +2;
              lcdWriteData(pos2,2,"-");
              pos2++;
            }
          }
        }
        else if(succes == 0){
          lcdWriteData(0,0," ERROR COMUNICACION ");
          lcdWriteData(0,1,"     GENERAL       ");
        }
        delay(2000);
        subMenu3End =true;
      }
      else if(key =='2'){

      }
      else if(key =='3'){
        subMenu3End = true;
      }
      else if(key =='4'){
        lcd.clear();
        subMenu3End = setMasterCode();
      }
      else if(key =='A'){
        subMenu3End = true;
      }
      else if(key =='*'){
        delay(100);
        short click = 0;
        for(int n=0;n<15;n++){
          key=keypad.getKey();
          if(key=='*') click ++;
          delay(100);
        }
        if(click >2){
          lcd.clear();
          lcdWriteData(0,0,"    Restore Menu  ");
          int code = getNumberByKeypad(4);
          key =!key;
          if(code==resetCode){
            lcdWriteData(0,1,"PULSAR 'A' PARA     ");
            lcdWriteData(0,2,"RESTAURAR A FABRICA ");
            lcdWriteData(0,3,"C:Cancelar A:Confirm");
            while(!timeOut && !subMenu3End ){
              key = keypad.getKey();
              if(millis()>(entryTime+30000))timeOut =true;
              if(key){
                if(key=='A'){   //Reset to factory default Confirmed
                  lcd.clear();
                  lcdWriteData(0,0," SISTEMA RESETEADO ");
                  lcdWriteData(0,2," REINICIE CONSOLA  ");
                  factoryReset();
                  //pongo aqui un while para que no salga hasta que reinicien sistema???
                  delay(1000);
                  subMenu3End = true;
                }
                else{
                  lcd.clear();
                  lcdWriteData(0,2,"OPERACION CANCELADA");
                  delay(1000);
                  subMenu3End =true;
                }
              }
            }
          }
          else{subMenu3End =true;}
        }
      }
    }
  }
  return true;
}

bool factoryReset(){

  saveDataToEEPROM(1,1);

}