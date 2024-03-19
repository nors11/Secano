#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <Arduino.h>
#include "vglobals.h"
void setupConfig();                                 //On init system
void configMenu();
void setFirstConfig();                              //First system on config by user
void restoreMasterCode(char *codeNew ,int size); 
void lcdWriteData(int row,int column,String text);
bool setMasterCode();
bool setDate();
bool setTime();
bool setShowersNumber();        //falta parpadeo digito + error solo digito 1 lo coge
bool setShowerTime();          //falta parpadeo digito
int  getShowerTime();
bool setNumberOfShowersDay();   //He de modificar para que sea de 0 a 9 +falta parpadeo digito
bool dateCheckers(int,char);
bool timeCheckers(int,char);
bool passValidate();
bool setDateTimetoClock(char);
int convertDataTime(int[8],int[6]);
int getNumberByKeypad(int);
int getShowersNumber();
int getShowerStatus(int);       //shwer id
bool setShowerStatus(int,int);  //shower id, status
int rfidRead();
short getRemainCredit(int);
void showDateTime();
bool isAlive(int);
bool isAliveDisp(int);
bool isInBlackList(int);
bool putInBlacklist(int);
bool takeOutOfBlackList(int);
bool updateShowerTimeToDevices(int);
bool subMenu1();
bool subMenu2();
bool subMenu3();
bool subMenu2_2();
bool restoreData();
bool saveDataToEEPROM(int,int);
bool factoryReset();
bool changeDisp(int newId);


#endif