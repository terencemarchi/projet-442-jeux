#ifndef REGLES_IA_STM_H
#define REGLES_IA_STM_H

#include "stdint.h"

#define REGLES_IA_TAILLE_PLATEAU      10U
#define REGLES_IA_NB_CASES_JOUABLES   50U
#define REGLES_IA_NB_LIGNES_PIONS     4U
#define REGLES_IA_NB_ENTREES          51U
#define REGLES_IA_NB_COUPS_MAX        64U

#define REGLES_IA_PION_NOIR           (-1)
#define REGLES_IA_CASE_VIDE           0
#define REGLES_IA_PION_BLANC          1

#define REGLES_IA_JOUEUR_NOIR         (-1)
#define REGLES_IA_JOUEUR_BLANC        1

#define REGLES_IA_GAGNANT_AUCUN       0

typedef struct
{
  uint8_t ligneDepart;
  uint8_t colonneDepart;
  uint8_t ligneArrivee;
  uint8_t colonneArrivee;
  uint8_t estPrise;
} CoupIaStm;

typedef struct
{
  int8_t plateau[REGLES_IA_TAILLE_PLATEAU][REGLES_IA_TAILLE_PLATEAU];
  int8_t joueurCourant;
  uint8_t partieTerminee;
  int8_t gagnant;
  uint16_t nbCoupsJoues;
} EtatJeuIaStm;

typedef struct
{
  CoupIaStm coups[REGLES_IA_NB_COUPS_MAX];
  uint8_t nbCoups;
} ListeCoupsIaStm;

uint8_t ReglesIaStm_EstCaseJouable(uint8_t ligne, uint8_t colonne);
void ReglesIaStm_InitialiserPartie(EtatJeuIaStm *etat);
void ReglesIaStm_CopierEtat(const EtatJeuIaStm *source, EtatJeuIaStm *destination);
uint8_t ReglesIaStm_CompterPions(const EtatJeuIaStm *etat, int8_t joueur);
void ReglesIaStm_GenererCoupsPossibles(const EtatJeuIaStm *etat, ListeCoupsIaStm *listeCoups);
uint8_t ReglesIaStm_CoupEstValide(const EtatJeuIaStm *etat, const CoupIaStm *coup);
uint8_t ReglesIaStm_AppliquerCoup(const EtatJeuIaStm *etat, const CoupIaStm *coup, EtatJeuIaStm *nouvelEtat);
void ReglesIaStm_ConvertirEtatEnEntrees(const EtatJeuIaStm *etat, float entrees[REGLES_IA_NB_ENTREES]);

#endif /* REGLES_IA_STM_H */
