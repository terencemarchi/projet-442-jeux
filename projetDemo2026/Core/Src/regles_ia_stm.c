#include "regles_ia_stm.h"

#include "string.h"

static uint8_t ReglesIaStm_EstDansPlateau(int32_t ligne, int32_t colonne);
static void ReglesIaStm_EvaluerFinPartie(EtatJeuIaStm *etat);

uint8_t ReglesIaStm_EstCaseJouable(uint8_t ligne, uint8_t colonne)
{
  return (uint8_t)(((ligne + colonne) % 2U) != 0U);
}

void ReglesIaStm_InitialiserPartie(EtatJeuIaStm *etat)
{
  uint32_t ligne;
  uint32_t colonne;

  memset(etat, 0, sizeof(*etat));
  etat->joueurCourant = REGLES_IA_JOUEUR_BLANC;
  etat->gagnant = REGLES_IA_GAGNANT_AUCUN;

  for (ligne = 0U; ligne < REGLES_IA_TAILLE_PLATEAU; ligne++)
  {
    for (colonne = 0U; colonne < REGLES_IA_TAILLE_PLATEAU; colonne++)
    {
      if (ReglesIaStm_EstCaseJouable((uint8_t)ligne, (uint8_t)colonne) == 0U)
      {
        continue;
      }

      if (ligne < REGLES_IA_NB_LIGNES_PIONS)
      {
        etat->plateau[ligne][colonne] = REGLES_IA_PION_BLANC;
      }
      else if (ligne >= (REGLES_IA_TAILLE_PLATEAU - REGLES_IA_NB_LIGNES_PIONS))
      {
        etat->plateau[ligne][colonne] = REGLES_IA_PION_NOIR;
      }
    }
  }
}

void ReglesIaStm_CopierEtat(const EtatJeuIaStm *source, EtatJeuIaStm *destination)
{
  *destination = *source;
}

uint8_t ReglesIaStm_CompterPions(const EtatJeuIaStm *etat, int8_t joueur)
{
  uint32_t ligne;
  uint32_t colonne;
  uint8_t compteur = 0U;

  for (ligne = 0U; ligne < REGLES_IA_TAILLE_PLATEAU; ligne++)
  {
    for (colonne = 0U; colonne < REGLES_IA_TAILLE_PLATEAU; colonne++)
    {
      if (etat->plateau[ligne][colonne] == joueur)
      {
        compteur++;
      }
    }
  }

  return compteur;
}

void ReglesIaStm_GenererCoupsPossibles(const EtatJeuIaStm *etat, ListeCoupsIaStm *listeCoups)
{
  int8_t direction;
  int8_t adversaire;
  uint32_t ligne;
  uint32_t colonne;

  listeCoups->nbCoups = 0U;

  direction = (etat->joueurCourant == REGLES_IA_JOUEUR_BLANC) ? 1 : -1;
  adversaire = (int8_t)(-etat->joueurCourant);

  for (ligne = 0U; ligne < REGLES_IA_TAILLE_PLATEAU; ligne++)
  {
    for (colonne = 0U; colonne < REGLES_IA_TAILLE_PLATEAU; colonne++)
    {
      int32_t ligneArrivee;
      int32_t colonneArrivee;
      int32_t ligneIntermediaire;
      int32_t colonneIntermediaire;
      int32_t deltaColonne;

      if (etat->plateau[ligne][colonne] != etat->joueurCourant)
      {
        continue;
      }

      for (deltaColonne = -1; deltaColonne <= 1; deltaColonne += 2)
      {
        ligneArrivee = (int32_t)ligne + direction;
        colonneArrivee = (int32_t)colonne + deltaColonne;

        if ((ReglesIaStm_EstDansPlateau(ligneArrivee, colonneArrivee) != 0U) &&
            (ReglesIaStm_EstCaseJouable((uint8_t)ligneArrivee, (uint8_t)colonneArrivee) != 0U) &&
            (etat->plateau[ligneArrivee][colonneArrivee] == REGLES_IA_CASE_VIDE) &&
            (listeCoups->nbCoups < REGLES_IA_NB_COUPS_MAX))
        {
          listeCoups->coups[listeCoups->nbCoups].ligneDepart = (uint8_t)ligne;
          listeCoups->coups[listeCoups->nbCoups].colonneDepart = (uint8_t)colonne;
          listeCoups->coups[listeCoups->nbCoups].ligneArrivee = (uint8_t)ligneArrivee;
          listeCoups->coups[listeCoups->nbCoups].colonneArrivee = (uint8_t)colonneArrivee;
          listeCoups->coups[listeCoups->nbCoups].estPrise = 0U;
          listeCoups->nbCoups++;
        }
      }

      for (deltaColonne = -2; deltaColonne <= 2; deltaColonne += 4)
      {
        ligneArrivee = (int32_t)ligne + (2 * direction);
        colonneArrivee = (int32_t)colonne + deltaColonne;
        ligneIntermediaire = (int32_t)ligne + direction;
        colonneIntermediaire = (int32_t)colonne + (deltaColonne / 2);

        if ((ReglesIaStm_EstDansPlateau(ligneArrivee, colonneArrivee) == 0U) ||
            (ReglesIaStm_EstCaseJouable((uint8_t)ligneArrivee, (uint8_t)colonneArrivee) == 0U))
        {
          continue;
        }

        if ((etat->plateau[ligneArrivee][colonneArrivee] != REGLES_IA_CASE_VIDE) ||
            (etat->plateau[ligneIntermediaire][colonneIntermediaire] != adversaire) ||
            (listeCoups->nbCoups >= REGLES_IA_NB_COUPS_MAX))
        {
          continue;
        }

        listeCoups->coups[listeCoups->nbCoups].ligneDepart = (uint8_t)ligne;
        listeCoups->coups[listeCoups->nbCoups].colonneDepart = (uint8_t)colonne;
        listeCoups->coups[listeCoups->nbCoups].ligneArrivee = (uint8_t)ligneArrivee;
        listeCoups->coups[listeCoups->nbCoups].colonneArrivee = (uint8_t)colonneArrivee;
        listeCoups->coups[listeCoups->nbCoups].estPrise = 1U;
        listeCoups->nbCoups++;
      }
    }
  }
}

uint8_t ReglesIaStm_CoupEstValide(const EtatJeuIaStm *etat, const CoupIaStm *coup)
{
  ListeCoupsIaStm listeCoups;
  uint32_t indexCoup;

  ReglesIaStm_GenererCoupsPossibles(etat, &listeCoups);

  for (indexCoup = 0U; indexCoup < listeCoups.nbCoups; indexCoup++)
  {
    if ((listeCoups.coups[indexCoup].ligneDepart == coup->ligneDepart) &&
        (listeCoups.coups[indexCoup].colonneDepart == coup->colonneDepart) &&
        (listeCoups.coups[indexCoup].ligneArrivee == coup->ligneArrivee) &&
        (listeCoups.coups[indexCoup].colonneArrivee == coup->colonneArrivee) &&
        (listeCoups.coups[indexCoup].estPrise == coup->estPrise))
    {
      return 1U;
    }
  }

  return 0U;
}

uint8_t ReglesIaStm_AppliquerCoup(const EtatJeuIaStm *etat, const CoupIaStm *coup, EtatJeuIaStm *nouvelEtat)
{
  int8_t joueur;

  if (ReglesIaStm_CoupEstValide(etat, coup) == 0U)
  {
    return 0U;
  }

  ReglesIaStm_CopierEtat(etat, nouvelEtat);
  joueur = nouvelEtat->joueurCourant;

  nouvelEtat->plateau[coup->ligneDepart][coup->colonneDepart] = REGLES_IA_CASE_VIDE;
  nouvelEtat->plateau[coup->ligneArrivee][coup->colonneArrivee] = joueur;

  if (coup->estPrise != 0U)
  {
    uint8_t ligneCapturee;
    uint8_t colonneCapturee;

    ligneCapturee = (uint8_t)((coup->ligneDepart + coup->ligneArrivee) / 2U);
    colonneCapturee = (uint8_t)((coup->colonneDepart + coup->colonneArrivee) / 2U);
    nouvelEtat->plateau[ligneCapturee][colonneCapturee] = REGLES_IA_CASE_VIDE;
  }

  nouvelEtat->nbCoupsJoues++;
  nouvelEtat->joueurCourant = (int8_t)(-joueur);
  ReglesIaStm_EvaluerFinPartie(nouvelEtat);
  return 1U;
}

void ReglesIaStm_ConvertirEtatEnEntrees(const EtatJeuIaStm *etat, float entrees[REGLES_IA_NB_ENTREES])
{
  uint32_t ligne;
  uint32_t colonne;
  uint32_t indexEntree = 0U;

  for (indexEntree = 0U; indexEntree < REGLES_IA_NB_ENTREES; indexEntree++)
  {
    entrees[indexEntree] = 0.0f;
  }

  indexEntree = 0U;
  for (ligne = 0U; ligne < REGLES_IA_TAILLE_PLATEAU; ligne++)
  {
    for (colonne = 0U; colonne < REGLES_IA_TAILLE_PLATEAU; colonne++)
    {
      if (ReglesIaStm_EstCaseJouable((uint8_t)ligne, (uint8_t)colonne) != 0U)
      {
        entrees[indexEntree] = (float)etat->plateau[ligne][colonne];
        indexEntree++;
      }
    }
  }

  entrees[REGLES_IA_NB_CASES_JOUABLES] = (float)etat->joueurCourant;
}

static uint8_t ReglesIaStm_EstDansPlateau(int32_t ligne, int32_t colonne)
{
  return (uint8_t)((ligne >= 0) && (ligne < (int32_t)REGLES_IA_TAILLE_PLATEAU) &&
                   (colonne >= 0) && (colonne < (int32_t)REGLES_IA_TAILLE_PLATEAU));
}

static void ReglesIaStm_EvaluerFinPartie(EtatJeuIaStm *etat)
{
  ListeCoupsIaStm listeCoups;

  if (ReglesIaStm_CompterPions(etat, REGLES_IA_JOUEUR_BLANC) == 0U)
  {
    etat->partieTerminee = 1U;
    etat->gagnant = REGLES_IA_JOUEUR_NOIR;
    return;
  }

  if (ReglesIaStm_CompterPions(etat, REGLES_IA_JOUEUR_NOIR) == 0U)
  {
    etat->partieTerminee = 1U;
    etat->gagnant = REGLES_IA_JOUEUR_BLANC;
    return;
  }

  ReglesIaStm_GenererCoupsPossibles(etat, &listeCoups);
  if (listeCoups.nbCoups == 0U)
  {
    etat->partieTerminee = 1U;
    etat->gagnant = (int8_t)(-etat->joueurCourant);
    return;
  }

  etat->partieTerminee = 0U;
  etat->gagnant = REGLES_IA_GAGNANT_AUCUN;
}
