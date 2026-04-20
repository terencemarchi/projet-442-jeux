#include "reseau_ia_stm.h"

#include "math.h"
#include "modele_ia_stm.h"

float ReseauIaStm_EvaluerEntrees(const float entrees[REGLES_IA_NB_ENTREES])
{
  float activationsCachees[MODELE_IA_STM_NB_NEURONES_CACHES];
  float somme;
  uint32_t indexNeurone;
  uint32_t indexEntree;

  for (indexNeurone = 0U; indexNeurone < MODELE_IA_STM_NB_NEURONES_CACHES; indexNeurone++)
  {
    somme = g_modeleIaBiaisCachee[indexNeurone];

    for (indexEntree = 0U; indexEntree < REGLES_IA_NB_ENTREES; indexEntree++)
    {
      somme += g_modeleIaPoidsEntreeCachee[indexNeurone][indexEntree] * entrees[indexEntree];
    }

    activationsCachees[indexNeurone] = tanhf(somme);
  }

  somme = g_modeleIaBiaisSortie;
  for (indexNeurone = 0U; indexNeurone < MODELE_IA_STM_NB_NEURONES_CACHES; indexNeurone++)
  {
    somme += g_modeleIaPoidsCacheeSortie[indexNeurone] * activationsCachees[indexNeurone];
  }

  return tanhf(somme);
}

float ReseauIaStm_EvaluerEtat(const EtatJeuIaStm *etat)
{
  float entrees[REGLES_IA_NB_ENTREES];

  ReglesIaStm_ConvertirEtatEnEntrees(etat, entrees);
  return ReseauIaStm_EvaluerEntrees(entrees);
}
