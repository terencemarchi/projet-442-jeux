#include "reseau_ia_stm.h"

#include "math.h"
#include "modele_policy_sb3_stm.h"

static uint16_t ReseauIaStm_CaseVersIndex(uint8_t ligne, uint8_t colonne);
static void ReseauIaStm_IndexVersCase(uint16_t indexCase, uint8_t *ligne, uint8_t *colonne);
static uint16_t ReseauIaStm_EncoderActionDepuisCoup(const CoupIaStm *coup);

void ReseauIaStm_CalculerActivationsCachees(const float entrees[REGLES_IA_NB_ENTREES],
                                            float couche1[64],
                                            float couche2[64])
{
  float somme;
  uint32_t indexNeurone;
  uint32_t indexEntree;

  for (indexNeurone = 0U; indexNeurone < MODELE_POLICY_SB3_STM_NB_COUCHE1; indexNeurone++)
  {
    somme = g_modelePolicySb3B1[indexNeurone];

    for (indexEntree = 0U; indexEntree < REGLES_IA_NB_ENTREES; indexEntree++)
    {
      somme += g_modelePolicySb3W1[indexNeurone][indexEntree] * entrees[indexEntree];
    }

    couche1[indexNeurone] = tanhf(somme);
  }

  for (indexNeurone = 0U; indexNeurone < MODELE_POLICY_SB3_STM_NB_COUCHE2; indexNeurone++)
  {
    uint32_t indexPrecedent;

    somme = g_modelePolicySb3B2[indexNeurone];
    for (indexPrecedent = 0U; indexPrecedent < MODELE_POLICY_SB3_STM_NB_COUCHE1; indexPrecedent++)
    {
      somme += g_modelePolicySb3W2[indexNeurone][indexPrecedent] * couche1[indexPrecedent];
    }

    couche2[indexNeurone] = tanhf(somme);
  }
}

float ReseauIaStm_EvaluerActionDepuisActivations(const float couche2[64], uint16_t indexAction)
{
  float somme;
  uint32_t indexNeurone;

  somme = g_modelePolicySb3B3[indexAction];
  for (indexNeurone = 0U; indexNeurone < MODELE_POLICY_SB3_STM_NB_COUCHE2; indexNeurone++)
  {
    somme += g_modelePolicySb3W3[indexAction][indexNeurone] * couche2[indexNeurone];
  }

  return somme;
}

uint8_t ReseauIaStm_ChoisirMeilleurCoup(const EtatJeuIaStm *etat,
                                        CoupIaStm *meilleurCoup,
                                        float *meilleurScore)
{
  ListeCoupsIaStm listeCoups;
  float entrees[REGLES_IA_NB_ENTREES];
  float couche1[MODELE_POLICY_SB3_STM_NB_COUCHE1];
  float couche2[MODELE_POLICY_SB3_STM_NB_COUCHE2];
  float score;
  uint16_t indexAction;
  uint32_t indexCoup;

  ReglesIaStm_GenererCoupsPossibles(etat, &listeCoups);
  if (listeCoups.nbCoups == 0U)
  {
    return 0U;
  }

  ReglesIaStm_ConvertirEtatEnEntrees(etat, entrees);
  ReseauIaStm_CalculerActivationsCachees(entrees, couche1, couche2);

  for (indexCoup = 0U; indexCoup < listeCoups.nbCoups; indexCoup++)
  {
    indexAction = ReseauIaStm_EncoderActionDepuisCoup(&listeCoups.coups[indexCoup]);
    score = ReseauIaStm_EvaluerActionDepuisActivations(couche2, indexAction);

    if ((indexCoup == 0U) || (score > *meilleurScore))
    {
      *meilleurCoup = listeCoups.coups[indexCoup];
      *meilleurScore = score;
    }
  }

  return 1U;
}

static uint16_t ReseauIaStm_CaseVersIndex(uint8_t ligne, uint8_t colonne)
{
  return (uint16_t)((ligne * 5U) + (colonne / 2U));
}

static void ReseauIaStm_IndexVersCase(uint16_t indexCase, uint8_t *ligne, uint8_t *colonne)
{
  *ligne = (uint8_t)(indexCase / 5U);
  if (((*ligne) % 2U) == 0U)
  {
    *colonne = (uint8_t)(1U + (2U * (indexCase % 5U)));
  }
  else
  {
    *colonne = (uint8_t)(2U * (indexCase % 5U));
  }
}

static uint16_t ReseauIaStm_EncoderActionDepuisCoup(const CoupIaStm *coup)
{
  uint16_t indexDepart;
  uint16_t indexArrivee;
  uint8_t ligneVerification;
  uint8_t colonneVerification;

  indexDepart = ReseauIaStm_CaseVersIndex(coup->ligneDepart, coup->colonneDepart);
  indexArrivee = ReseauIaStm_CaseVersIndex(coup->ligneArrivee, coup->colonneArrivee);

  ReseauIaStm_IndexVersCase(indexDepart, &ligneVerification, &colonneVerification);
  (void)ligneVerification;
  (void)colonneVerification;

  return (uint16_t)((indexDepart * REGLES_IA_NB_CASES_JOUABLES) + indexArrivee);
}
