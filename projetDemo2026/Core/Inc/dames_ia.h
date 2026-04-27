#ifndef DAMES_IA_H
#define DAMES_IA_H

#include "stdint.h"

typedef enum
{
  DAMES_IA_MODE_IA_VS_IA = 0,
  DAMES_IA_MODE_JOUEUR_VS_IA
} DamesIaMode;

typedef enum
{
  DAMES_IA_ACTION_AUCUNE = 0,
  DAMES_IA_ACTION_QUITTER
} DamesIaAction;

void DamesIa_AfficherNouvellePartie(DamesIaMode mode);
DamesIaAction DamesIa_GererTouch(uint16_t x, uint16_t y);
void DamesIa_MettreAJour(uint32_t tempsCourantMs);

#endif /* DAMES_IA_H */
