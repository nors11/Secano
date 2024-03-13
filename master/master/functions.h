#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <Arduino.h>
#include "vglobals.h"
void setupConfig();                                 //On init system
void setFirstConfig();                              //First system on config by user
void changeMasterCode(int *codeNew ); 
void lcdWriteData(int row,int column,String text);
bool setMasterCode();
bool setDate();
bool setTime();
bool setShowersNumber();
bool dateCheckers(int,char);
bool timeCheckers(int,char);
bool setDateTimetoClock();
int convertDataTime(int[8],int[6]);
void showDateTime();
#endif