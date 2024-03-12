#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <Arduino.h>
#include "vglobals.h"
void setupConfig();                                 //On init system
void setFirstConfig();                              //First system on config by user
void changeMasterCode(int *codeNew ); 
void lcdWriteData(int row,int column,String text);
bool setMasterCode();
bool setDateTime();
bool checkers(int,char);
#endif