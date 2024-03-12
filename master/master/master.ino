#include <EEPROM.h>
/* #include <Keypad.h> */
#include "functions.h"
#include <Wire.h> 
#include "RTClib.h"
//#include <LiquidCrystal_I2C.h>
/* #include "vglobals.h" */

//LiquidCrystal_I2C lcd(0x27,20,4);

int codigoMaster = 11223;
//bool estado[2500];
//bool estado2[2500];
int maxV =3000;
/*const int ROW_NUM = 4; //four rows
const int COLUMN_NUM = 3; //three columns
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte pin_rows[ROW_NUM] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {5, 4, 3}; //connect to the column pinouts of the keypad*/
/* Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM ); */
String daysOfTheWeek[7] = { "Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado" };
String monthsNames[12] = { "Enero", "Febrero", "Marzo", "Abril", "Mayo",  "Junio", "Julio","Agosto","Septiembre","Octubre","Noviembre","Diciembre" };

RTC_DS3231 rtc;
bool firstBoot = true;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  setupConfig();

  if (!rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    while (1);
  }
  // Si se ha perdido la corriente, fijar fecha y hora
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(2024, 3, 12, 7, 34, 0));
  }
  //firstBoot = EEPROM.read(1);  Poner a 1 si es firts boot
}

void loop() {

  //EEPROM.put( eeAddress, customVar );
  if(firstBoot){setFirstConfig();}
  delay(100);
/*  Serial.print("codigo antes de ir a funcion = ");Serial.println(codigoMaster);
  int *dir;
  dir = &codigoMaster;
  changeMasterCode(dir);
  Serial.print("codigo al volver = ");Serial.println(codigoMaster);
  delay(3000); */
}
void showDateTime(){
  char buf1[20];
  DateTime now = rtc.now();
  sprintf(buf1, "%02d:%02d:%02d  %02d/%02d/%02d",  now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year()); 
  lcdWriteData(0,0,buf1);
}
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
}
