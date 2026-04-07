#ifndef MENU_H
#define MENU_H

#include "stdint.h"

typedef enum
{
  MENU_ACTION_AUCUNE = 0,
  MENU_ACTION_JEU_DAMES
} MenuAction;

void Menu_Afficher(void);
MenuAction Menu_GererTouch(uint16_t x, uint16_t y);

#endif /* MENU_H */
