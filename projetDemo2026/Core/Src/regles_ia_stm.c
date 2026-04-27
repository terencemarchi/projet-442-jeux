#include "regles_ia_stm.h"

#include "string.h"

static uint8_t ReglesIaStm_EstDansPlateau(int32_t ligne, int32_t colonne);
static void ReglesIaStm_EvaluerFinPartie(EtatJeuIaStm *etat);
static uint8_t ReglesIaStm_EstDame(int8_t piece);
static uint8_t ReglesIaStm_PieceAppartientAuJoueur(int8_t piece, int8_t joueur);
static uint8_t ReglesIaStm_PieceEstAdverse(int8_t piece, int8_t joueur);

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
      if (ReglesIaStm_PieceAppartientAuJoueur(etat->plateau[ligne][colonne], joueur) != 0U)
      {
        compteur++;
      }
    }
  }

  return compteur;
}

void ReglesIaStm_GenererCoupsPossibles(const EtatJeuIaStm *etat, ListeCoupsIaStm *listeCoups)
{
  uint32_t ligne;
  uint32_t colonne;

  listeCoups->nbCoups = 0U;

  for (ligne = 0U; ligne < REGLES_IA_TAILLE_PLATEAU; ligne++)
  {
    for (colonne = 0U; colonne < REGLES_IA_TAILLE_PLATEAU; colonne++)
    {
      int8_t piece;
      int8_t direction;
      int32_t directions[4][2];
      int32_t directionsPrise[4][2];
      uint32_t nbDirections;
      uint32_t indexDirection;

      piece = etat->plateau[ligne][colonne];
      if (ReglesIaStm_PieceAppartientAuJoueur(piece, etat->joueurCourant) == 0U)
      {
        continue;
      }

      if (ReglesIaStm_EstDame(piece) != 0U)
      {
        directions[0][0] = -1; directions[0][1] = -1;
        directions[1][0] = -1; directions[1][1] = 1;
        directions[2][0] = 1; directions[2][1] = -1;
        directions[3][0] = 1; directions[3][1] = 1;
        directionsPrise[0][0] = -2; directionsPrise[0][1] = -2;
        directionsPrise[1][0] = -2; directionsPrise[1][1] = 2;
        directionsPrise[2][0] = 2; directionsPrise[2][1] = -2;
        directionsPrise[3][0] = 2; directionsPrise[3][1] = 2;
        nbDirections = 4U;
      }
      else
      {
        direction = (etat->joueurCourant == REGLES_IA_JOUEUR_BLANC) ? 1 : -1;
        directions[0][0] = direction; directions[0][1] = -1;
        directions[1][0] = direction; directions[1][1] = 1;
        directionsPrise[0][0] = 2 * direction; directionsPrise[0][1] = -2;
        directionsPrise[1][0] = 2 * direction; directionsPrise[1][1] = 2;
        nbDirections = 2U;
      }

      for (indexDirection = 0U; indexDirection < nbDirections; indexDirection++)
      {
        int32_t ligneArrivee;
        int32_t colonneArrivee;

        ligneArrivee = (int32_t)ligne + directions[indexDirection][0];
        colonneArrivee = (int32_t)colonne + directions[indexDirection][1];

        if ((ReglesIaStm_EstDansPlateau(ligneArrivee, colonneArrivee) == 0U) ||
            (ReglesIaStm_EstCaseJouable((uint8_t)ligneArrivee, (uint8_t)colonneArrivee) == 0U))
        {
          continue;
        }

        if ((etat->plateau[ligneArrivee][colonneArrivee] != REGLES_IA_CASE_VIDE) ||
            (listeCoups->nbCoups >= REGLES_IA_NB_COUPS_MAX))
        {
          continue;
        }

        listeCoups->coups[listeCoups->nbCoups].ligneDepart = (uint8_t)ligne;
        listeCoups->coups[listeCoups->nbCoups].colonneDepart = (uint8_t)colonne;
        listeCoups->coups[listeCoups->nbCoups].ligneArrivee = (uint8_t)ligneArrivee;
        listeCoups->coups[listeCoups->nbCoups].colonneArrivee = (uint8_t)colonneArrivee;
        listeCoups->coups[listeCoups->nbCoups].estPrise = 0U;
        listeCoups->nbCoups++;
      }

      for (indexDirection = 0U; indexDirection < nbDirections; indexDirection++)
      {
        int32_t ligneArrivee;
        int32_t colonneArrivee;
        int32_t ligneIntermediaire;
        int32_t colonneIntermediaire;

        ligneArrivee = (int32_t)ligne + directionsPrise[indexDirection][0];
        colonneArrivee = (int32_t)colonne + directionsPrise[indexDirection][1];
        ligneIntermediaire = (int32_t)ligne + (directionsPrise[indexDirection][0] / 2);
        colonneIntermediaire = (int32_t)colonne + (directionsPrise[indexDirection][1] / 2);

        if ((ReglesIaStm_EstDansPlateau(ligneArrivee, colonneArrivee) == 0U) ||
            (ReglesIaStm_EstCaseJouable((uint8_t)ligneArrivee, (uint8_t)colonneArrivee) == 0U))
        {
          continue;
        }

        if ((etat->plateau[ligneArrivee][colonneArrivee] != REGLES_IA_CASE_VIDE) ||
            (ReglesIaStm_PieceEstAdverse(etat->plateau[ligneIntermediaire][colonneIntermediaire],
                                         etat->joueurCourant) == 0U) ||
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
  int8_t piece;

  if (ReglesIaStm_CoupEstValide(etat, coup) == 0U)
  {
    return 0U;
  }

  ReglesIaStm_CopierEtat(etat, nouvelEtat);
  joueur = nouvelEtat->joueurCourant;
  piece = etat->plateau[coup->ligneDepart][coup->colonneDepart];

  nouvelEtat->plateau[coup->ligneDepart][coup->colonneDepart] = REGLES_IA_CASE_VIDE;
  nouvelEtat->plateau[coup->ligneArrivee][coup->colonneArrivee] = piece;

  if (coup->estPrise != 0U)
  {
    uint8_t ligneCapturee;
    uint8_t colonneCapturee;

    ligneCapturee = (uint8_t)((coup->ligneDepart + coup->ligneArrivee) / 2U);
    colonneCapturee = (uint8_t)((coup->colonneDepart + coup->colonneArrivee) / 2U);
    nouvelEtat->plateau[ligneCapturee][colonneCapturee] = REGLES_IA_CASE_VIDE;
  }

  if ((piece == REGLES_IA_PION_BLANC) &&
      (coup->ligneArrivee == (REGLES_IA_TAILLE_PLATEAU - 1U)))
  {
    nouvelEtat->plateau[coup->ligneArrivee][coup->colonneArrivee] = REGLES_IA_DAME_BLANCHE;
  }
  else if ((piece == REGLES_IA_PION_NOIR) &&
           (coup->ligneArrivee == 0U))
  {
    nouvelEtat->plateau[coup->ligneArrivee][coup->colonneArrivee] = REGLES_IA_DAME_NOIRE;
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

static uint8_t ReglesIaStm_EstDame(int8_t piece)
{
  return (uint8_t)((piece == REGLES_IA_DAME_BLANCHE) || (piece == REGLES_IA_DAME_NOIRE));
}

static uint8_t ReglesIaStm_PieceAppartientAuJoueur(int8_t piece, int8_t joueur)
{
  if (joueur == REGLES_IA_JOUEUR_BLANC)
  {
    return (uint8_t)((piece == REGLES_IA_PION_BLANC) || (piece == REGLES_IA_DAME_BLANCHE));
  }

  return (uint8_t)((piece == REGLES_IA_PION_NOIR) || (piece == REGLES_IA_DAME_NOIRE));
}

static uint8_t ReglesIaStm_PieceEstAdverse(int8_t piece, int8_t joueur)
{
  if (joueur == REGLES_IA_JOUEUR_BLANC)
  {
    return (uint8_t)((piece == REGLES_IA_PION_NOIR) || (piece == REGLES_IA_DAME_NOIRE));
  }

  return (uint8_t)((piece == REGLES_IA_PION_BLANC) || (piece == REGLES_IA_DAME_BLANCHE));
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
