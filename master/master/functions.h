#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <Arduino.h>
#include "vglobals.h"
void setupConfig();                                 //On init system
void configMenu();
void setFirstConfig();                              //First system on config by user
void changeMasterCode(int *codeNew ); 
void lcdWriteData(int row,int column,String text);
bool setMasterCode();
bool setDate();
bool setTime();
bool setShowersNumber();        //falta parpadeo digito
bool setShowersTime();          //falta parpadeo digito
bool setNumberOfShowersDay();   //He de modificar para que sea de 0 a 9 +falta parpadeo digito
bool dateCheckers(int,char);
bool timeCheckers(int,char);
bool setDateTimetoClock(char);
int convertDataTime(int[8],int[6]);
void showDateTime();
bool isAlive(int);
#endif