#ifndef MODELE_IA_STM_H
#define MODELE_IA_STM_H

#include "regles_ia_stm.h"

#define MODELE_IA_STM_NB_NEURONES_CACHES 16U

extern const float g_modeleIaPoidsEntreeCachee[MODELE_IA_STM_NB_NEURONES_CACHES][REGLES_IA_NB_ENTREES];
extern const float g_modeleIaBiaisCachee[MODELE_IA_STM_NB_NEURONES_CACHES];
extern const float g_modeleIaPoidsCacheeSortie[MODELE_IA_STM_NB_NEURONES_CACHES];
extern const float g_modeleIaBiaisSortie;

#endif /* MODELE_IA_STM_H */
