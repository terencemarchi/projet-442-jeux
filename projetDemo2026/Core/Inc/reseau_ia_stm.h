#ifndef RESEAU_IA_STM_H
#define RESEAU_IA_STM_H

#include "regles_ia_stm.h"

float ReseauIaStm_EvaluerEntrees(const float entrees[REGLES_IA_NB_ENTREES]);
float ReseauIaStm_EvaluerEtat(const EtatJeuIaStm *etat);

#endif /* RESEAU_IA_STM_H */
