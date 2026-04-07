#ifndef DAMES_H
#define DAMES_H

#include "stdint.h"

typedef enum
{
  DAMES_ACTION_AUCUNE = 0,
  DAMES_ACTION_QUITTER
} DamesAction;

void Dames_AfficherNouvellePartie(void);
DamesAction Dames_GererTouch(uint16_t x, uint16_t y);

#endif /* DAMES_H */
