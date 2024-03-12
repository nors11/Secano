#include <EEPROM.h>
#include "functions.h"
#include <Wire.h> 

/* #include "vglobals.h" */



int codigoMaster = 11223;
//bool estado[2500];
//bool estado2[2500];
int maxV =3000;
/*
String daysOfTheWeek[7] = { "Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado" };
String monthsNames[12] = { "Enero", "Febrero", "Marzo", "Abril", "Mayo",  "Junio", "Julio","Agosto","Septiembre","Octubre","Noviembre","Diciembre" };*/


bool firstBoot = true;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  setupConfig();


  // Si se ha perdido la corriente, fijar fecha y hora

  //firstBoot = EEPROM.read(1);  Poner a 1 si es firts boot
}

void loop() {

  //EEPROM.put( eeAddress, customVar );
  if(firstBoot){setFirstConfig();}
  delay(1000);
/*  Serial.print("codigo antes de ir a funcion = ");Serial.println(codigoMaster);
  int *dir;
  dir = &codigoMaster;
  changeMasterCode(dir);
  Serial.print("codigo al volver = ");Serial.println(codigoMaster);
  delay(3000); */
}

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
