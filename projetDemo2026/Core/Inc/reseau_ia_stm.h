#ifndef RESEAU_IA_STM_H
#define RESEAU_IA_STM_H

#include "regles_ia_stm.h"

void ReseauIaStm_CalculerActivationsCachees(const float entrees[REGLES_IA_NB_ENTREES],
                                            float couche1[64],
                                            float couche2[64]);
float ReseauIaStm_EvaluerActionDepuisActivations(const float couche2[64], uint16_t indexAction);
uint8_t ReseauIaStm_ChoisirMeilleurCoup(const EtatJeuIaStm *etat,
                                        CoupIaStm *meilleurCoup,
                                        float *meilleurScore);

#endif /* RESEAU_IA_STM_H */
