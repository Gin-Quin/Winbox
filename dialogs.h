#ifndef DIALOGS_H
#define DIALOGS_H

#include "widgets.h"
#include "Dialog.h"
#include "Text.h"
#include "menus.h"


wMENU *wPopup(wMENU *m, int *n);
wMENU *wPopupXY(wMENU *m, int x, int y, int *n);
void wShowMsg(const char *title, const char *msg);
int wInputMsg(const char *title, const char *msg);
void wFastMsg(const char *msg, ...);
int wOpenBox(const char *path, const char *pattern, char *fileName);
int wSaveBox(const char *path, char *fileName);

#endif