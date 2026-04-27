#include "dames_ia.h"
#include "main.h"
#include "regles_ia_stm.h"
#include "reseau_ia_stm.h"

#include "stdio.h"
#include "string.h"
#include "stm32746g_discovery_lcd.h"

#define TAILLE_CASE_IA                24U
#define PLATEAU_IA_X                  16U
#define PLATEAU_IA_Y                  16U
#define RAYON_PION_IA                 9U

#define COULEUR_CASE_CLAIRE_IA        ((uint32_t)0xFFF1E3C6)
#define COULEUR_CASE_FONCEE_IA        ((uint32_t)0xFF8B5A2B)
#define COULEUR_FOND_ECRAN_IA         LCD_COLOR_WHITE
#define COULEUR_BORDURE_PLATEAU_IA    LCD_COLOR_BLACK
#define COULEUR_PION_BLANC_IA         ((uint32_t)0xFFF7F3EB)
#define COULEUR_PION_NOIR_IA          ((uint32_t)0xFF303030)
#define COULEUR_CONTOUR_PION_BLANC_IA LCD_COLOR_BLACK
#define COULEUR_CONTOUR_PION_NOIR_IA  LCD_COLOR_WHITE
#define COULEUR_INFOS_IA              LCD_COLOR_DARKBLUE
#define COULEUR_BOUTON_QUITTER_IA     LCD_COLOR_RED
#define COULEUR_TEXTE_QUITTER_IA      LCD_COLOR_WHITE
#define COULEUR_ZONE_INFOS_IA         COULEUR_FOND_ECRAN_IA
#define COULEUR_SELECTION_IA          ((uint32_t)0xFF2E86DE)

#define BOUTON_QUITTER_IA_X           360U
#define BOUTON_QUITTER_IA_Y           224U
#define BOUTON_QUITTER_IA_LARGEUR     100U
#define BOUTON_QUITTER_IA_HAUTEUR     32U

#define DELAI_COUP_AUTOMATIQUE_MS     500U
#define ZONE_INFOS_X                  286U
#define ZONE_INFOS_Y                  20U
#define ZONE_INFOS_LARGEUR            180U
#define ZONE_INFOS_HAUTEUR            130U

typedef struct
{
  EtatJeuIaStm etatJeu;
  DamesIaMode mode;
  uint32_t dernierTempsCoupMs;
  uint8_t selectionActive;
  uint8_t ligneSelection;
  uint8_t colonneSelection;
} EtatDamesIa;

static EtatDamesIa etatDamesIa;

static void RafraichirAffichageIa(void);
static uint8_t ChoisirMeilleurCoupIa(const EtatJeuIaStm *etat, CoupIaStm *meilleurCoup);
static void DessinerPlateauIa(void);
static void DessinerPionsIa(const EtatDamesIa *etat);
static void DessinerSelectionIa(const EtatDamesIa *etat);
static void DessinerPionIa(uint32_t ligne, uint32_t colonne, uint32_t couleurRemplissage, uint32_t couleurContour);
static void DessinerMarqueDameIa(uint32_t ligne, uint32_t colonne, uint32_t couleur);
static void DessinerBoutonQuitterIa(void);
static void DessinerInfosIa(const EtatDamesIa *etat);
static void ObtenirCentreCaseIa(uint32_t ligne, uint32_t colonne, uint16_t *x, uint16_t *y);
static void AfficherTexteCentreZoneIa(uint16_t x, uint16_t y, uint16_t largeur, const char *texte);
static uint8_t CoordonneesSontDansZoneIa(uint16_t x, uint16_t y,
                                         uint16_t zoneX, uint16_t zoneY,
                                         uint16_t largeur, uint16_t hauteur);
static uint8_t ConvertirCoordonneesEnCaseIa(uint16_t x, uint16_t y, uint8_t *ligne, uint8_t *colonne);
static uint8_t JoueurHumainPeutJouerIa(const EtatDamesIa *etat);
static uint8_t PieceAppartientAuJoueurHumainIa(int8_t piece);
static void SelectionnerCaseIa(EtatDamesIa *etat, uint8_t ligne, uint8_t colonne);
static void DeselectionnerCaseIa(EtatDamesIa *etat);
static uint8_t TenterJouerCoupHumainIa(EtatDamesIa *etat, uint8_t ligneArrivee, uint8_t colonneArrivee);

void DamesIa_AfficherNouvellePartie(DamesIaMode mode)
{
  memset(&etatDamesIa, 0, sizeof(etatDamesIa));
  etatDamesIa.mode = mode;
  ReglesIaStm_InitialiserPartie(&etatDamesIa.etatJeu);
  RafraichirAffichageIa();
}

DamesIaAction DamesIa_GererTouch(uint16_t x, uint16_t y)
{
  uint8_t ligneCase;
  uint8_t colonneCase;

  if (CoordonneesSontDansZoneIa(x, y, BOUTON_QUITTER_IA_X, BOUTON_QUITTER_IA_Y,
                                BOUTON_QUITTER_IA_LARGEUR, BOUTON_QUITTER_IA_HAUTEUR) != 0U)
  {
    return DAMES_IA_ACTION_QUITTER;
  }

  if ((JoueurHumainPeutJouerIa(&etatDamesIa) == 0U) ||
      (ConvertirCoordonneesEnCaseIa(x, y, &ligneCase, &colonneCase) == 0U) ||
      (ReglesIaStm_EstCaseJouable(ligneCase, colonneCase) == 0U))
  {
    return DAMES_IA_ACTION_AUCUNE;
  }

  if (etatDamesIa.selectionActive == 0U)
  {
    if (PieceAppartientAuJoueurHumainIa(etatDamesIa.etatJeu.plateau[ligneCase][colonneCase]) != 0U)
    {
      SelectionnerCaseIa(&etatDamesIa, ligneCase, colonneCase);
      RafraichirAffichageIa();
    }
    return DAMES_IA_ACTION_AUCUNE;
  }

  if ((etatDamesIa.ligneSelection == ligneCase) &&
      (etatDamesIa.colonneSelection == colonneCase))
  {
    DeselectionnerCaseIa(&etatDamesIa);
    RafraichirAffichageIa();
    return DAMES_IA_ACTION_AUCUNE;
  }

  if (PieceAppartientAuJoueurHumainIa(etatDamesIa.etatJeu.plateau[ligneCase][colonneCase]) != 0U)
  {
    SelectionnerCaseIa(&etatDamesIa, ligneCase, colonneCase);
    RafraichirAffichageIa();
    return DAMES_IA_ACTION_AUCUNE;
  }

  if (TenterJouerCoupHumainIa(&etatDamesIa, ligneCase, colonneCase) != 0U)
  {
    etatDamesIa.dernierTempsCoupMs = HAL_GetTick();
    RafraichirAffichageIa();
  }

  return DAMES_IA_ACTION_AUCUNE;
}

void DamesIa_MettreAJour(uint32_t tempsCourantMs)
{
  CoupIaStm meilleurCoup;
  EtatJeuIaStm nouvelEtat;

  if (etatDamesIa.etatJeu.partieTerminee != 0U)
  {
    return;
  }

  if (etatDamesIa.mode == DAMES_IA_MODE_JOUEUR_VS_IA)
  {
    if (etatDamesIa.etatJeu.joueurCourant == REGLES_IA_JOUEUR_BLANC)
    {
      return;
    }
  }
  else if (etatDamesIa.mode != DAMES_IA_MODE_IA_VS_IA)
  {
    return;
  }

  if ((tempsCourantMs - etatDamesIa.dernierTempsCoupMs) < DELAI_COUP_AUTOMATIQUE_MS)
  {
    return;
  }

  if (ChoisirMeilleurCoupIa(&etatDamesIa.etatJeu, &meilleurCoup) == 0U)
  {
    return;
  }

  if (ReglesIaStm_AppliquerCoup(&etatDamesIa.etatJeu, &meilleurCoup, &nouvelEtat) == 0U)
  {
    return;
  }

  etatDamesIa.etatJeu = nouvelEtat;
  etatDamesIa.dernierTempsCoupMs = tempsCourantMs;
  RafraichirAffichageIa();
}

static void RafraichirAffichageIa(void)
{
  DessinerPlateauIa();
  DessinerPionsIa(&etatDamesIa);
  DessinerSelectionIa(&etatDamesIa);
  DessinerBoutonQuitterIa();
  DessinerInfosIa(&etatDamesIa);
}

static uint8_t ChoisirMeilleurCoupIa(const EtatJeuIaStm *etat, CoupIaStm *meilleurCoup)
{
  float meilleurScore = 0.0f;
  return ReseauIaStm_ChoisirMeilleurCoup(etat, meilleurCoup, &meilleurScore);
}

static void DessinerPlateauIa(void)
{
  uint32_t ligne;
  uint32_t colonne;
  uint16_t xCase;
  uint16_t yCase;

  BSP_LCD_SelectLayer(0);
  BSP_LCD_Clear(COULEUR_FOND_ECRAN_IA);
  BSP_LCD_SetTextColor(COULEUR_BORDURE_PLATEAU_IA);
  BSP_LCD_DrawRect((uint16_t)(PLATEAU_IA_X - 1U), (uint16_t)(PLATEAU_IA_Y - 1U),
                   (uint16_t)(REGLES_IA_TAILLE_PLATEAU * TAILLE_CASE_IA + 2U),
                   (uint16_t)(REGLES_IA_TAILLE_PLATEAU * TAILLE_CASE_IA + 2U));

  for (ligne = 0U; ligne < REGLES_IA_TAILLE_PLATEAU; ligne++)
  {
    for (colonne = 0U; colonne < REGLES_IA_TAILLE_PLATEAU; colonne++)
    {
      xCase = (uint16_t)(PLATEAU_IA_X + (colonne * TAILLE_CASE_IA));
      yCase = (uint16_t)(PLATEAU_IA_Y + (ligne * TAILLE_CASE_IA));

      BSP_LCD_SetTextColor((((ligne + colonne) % 2U) == 0U) ? COULEUR_CASE_CLAIRE_IA : COULEUR_CASE_FONCEE_IA);
      BSP_LCD_FillRect(xCase, yCase, TAILLE_CASE_IA, TAILLE_CASE_IA);
    }
  }

  BSP_LCD_SelectLayer(1);
  BSP_LCD_Clear(0x00000000);
}

static void DessinerPionsIa(const EtatDamesIa *etat)
{
  uint32_t ligne;
  uint32_t colonne;

  for (ligne = 0U; ligne < REGLES_IA_TAILLE_PLATEAU; ligne++)
  {
    for (colonne = 0U; colonne < REGLES_IA_TAILLE_PLATEAU; colonne++)
    {
      if (etat->etatJeu.plateau[ligne][colonne] == REGLES_IA_PION_BLANC)
      {
        DessinerPionIa(ligne, colonne, COULEUR_PION_BLANC_IA, COULEUR_CONTOUR_PION_BLANC_IA);
      }
      else if (etat->etatJeu.plateau[ligne][colonne] == REGLES_IA_DAME_BLANCHE)
      {
        DessinerPionIa(ligne, colonne, COULEUR_PION_BLANC_IA, COULEUR_CONTOUR_PION_BLANC_IA);
        DessinerMarqueDameIa(ligne, colonne, COULEUR_PION_NOIR_IA);
      }
      else if (etat->etatJeu.plateau[ligne][colonne] == REGLES_IA_PION_NOIR)
      {
        DessinerPionIa(ligne, colonne, COULEUR_PION_NOIR_IA, COULEUR_CONTOUR_PION_NOIR_IA);
      }
      else if (etat->etatJeu.plateau[ligne][colonne] == REGLES_IA_DAME_NOIRE)
      {
        DessinerPionIa(ligne, colonne, COULEUR_PION_NOIR_IA, COULEUR_CONTOUR_PION_NOIR_IA);
        DessinerMarqueDameIa(ligne, colonne, COULEUR_PION_BLANC_IA);
      }
    }
  }
}

static void DessinerPionIa(uint32_t ligne, uint32_t colonne, uint32_t couleurRemplissage, uint32_t couleurContour)
{
  uint16_t centreX;
  uint16_t centreY;

  ObtenirCentreCaseIa(ligne, colonne, &centreX, &centreY);

  BSP_LCD_SetTextColor(couleurContour);
  BSP_LCD_FillCircle(centreX, centreY, (uint16_t)(RAYON_PION_IA + 1U));
  BSP_LCD_SetTextColor(couleurRemplissage);
  BSP_LCD_FillCircle(centreX, centreY, RAYON_PION_IA);
}

static void DessinerSelectionIa(const EtatDamesIa *etat)
{
  uint16_t x;
  uint16_t y;

  if (etat->selectionActive == 0U)
  {
    return;
  }

  x = (uint16_t)(PLATEAU_IA_X + (etat->colonneSelection * TAILLE_CASE_IA));
  y = (uint16_t)(PLATEAU_IA_Y + (etat->ligneSelection * TAILLE_CASE_IA));

  BSP_LCD_SetTextColor(COULEUR_SELECTION_IA);
  BSP_LCD_DrawRect(x, y, TAILLE_CASE_IA, TAILLE_CASE_IA);
  BSP_LCD_DrawRect((uint16_t)(x + 1U), (uint16_t)(y + 1U), TAILLE_CASE_IA - 2U, TAILLE_CASE_IA - 2U);
}

static void DessinerMarqueDameIa(uint32_t ligne, uint32_t colonne, uint32_t couleur)
{
  uint16_t centreX;
  uint16_t centreY;

  ObtenirCentreCaseIa(ligne, colonne, &centreX, &centreY);
  BSP_LCD_SetTextColor(couleur);
  BSP_LCD_FillCircle(centreX, centreY, 4U);
}

static void DessinerBoutonQuitterIa(void)
{
  BSP_LCD_SelectLayer(0);
  BSP_LCD_SetTextColor(COULEUR_BOUTON_QUITTER_IA);
  BSP_LCD_FillRect(BOUTON_QUITTER_IA_X, BOUTON_QUITTER_IA_Y,
                   BOUTON_QUITTER_IA_LARGEUR, BOUTON_QUITTER_IA_HAUTEUR);

  BSP_LCD_SetBackColor(COULEUR_BOUTON_QUITTER_IA);
  BSP_LCD_SetTextColor(COULEUR_TEXTE_QUITTER_IA);
  BSP_LCD_SetFont(&Font16);
  AfficherTexteCentreZoneIa(BOUTON_QUITTER_IA_X, (uint16_t)(BOUTON_QUITTER_IA_Y + 8U),
                            BOUTON_QUITTER_IA_LARGEUR, "Quitter");
}

static void DessinerInfosIa(const EtatDamesIa *etat)
{
  char ligneMode[40];
  char ligneEtat[40];

  BSP_LCD_SelectLayer(0);
  BSP_LCD_SetBackColor(COULEUR_FOND_ECRAN_IA);
  BSP_LCD_SetTextColor(COULEUR_INFOS_IA);
  BSP_LCD_SetTextColor(COULEUR_ZONE_INFOS_IA);
  BSP_LCD_FillRect(ZONE_INFOS_X, ZONE_INFOS_Y, ZONE_INFOS_LARGEUR, ZONE_INFOS_HAUTEUR);
  BSP_LCD_SetTextColor(COULEUR_INFOS_IA);

  BSP_LCD_SetFont(&Font16);
  BSP_LCD_DisplayStringAt(ZONE_INFOS_X, 26, (uint8_t *)"Jeu IA", LEFT_MODE);

  BSP_LCD_SetFont(&Font12);
  snprintf(ligneMode, sizeof(ligneMode), "Mode : %s",
           (etat->mode == DAMES_IA_MODE_IA_VS_IA) ? "IA vs IA" : "Joueur contre IA");
  if (etat->etatJeu.partieTerminee != 0U)
  {
    snprintf(ligneEtat, sizeof(ligneEtat), "Gagnant : %s",
             (etat->etatJeu.gagnant == REGLES_IA_JOUEUR_BLANC) ? "blancs" : "noirs");
  }
  else
  {
    snprintf(ligneEtat, sizeof(ligneEtat), "Coups joues : %u",
             (unsigned int)etat->etatJeu.nbCoupsJoues);
  }

  BSP_LCD_DisplayStringAt(ZONE_INFOS_X, 60, (uint8_t *)ligneMode, LEFT_MODE);
  BSP_LCD_DisplayStringAt(ZONE_INFOS_X, 84,
                          (uint8_t *)((etat->etatJeu.joueurCourant == REGLES_IA_JOUEUR_BLANC) ? "Tour : blancs" : "Tour : noirs"),
                          LEFT_MODE);
  BSP_LCD_DisplayStringAt(ZONE_INFOS_X, 108,
                          (uint8_t *)ligneEtat, LEFT_MODE);
}

static void ObtenirCentreCaseIa(uint32_t ligne, uint32_t colonne, uint16_t *x, uint16_t *y)
{
  *x = (uint16_t)(PLATEAU_IA_X + (colonne * TAILLE_CASE_IA) + (TAILLE_CASE_IA / 2U));
  *y = (uint16_t)(PLATEAU_IA_Y + (ligne * TAILLE_CASE_IA) + (TAILLE_CASE_IA / 2U));
}

static void AfficherTexteCentreZoneIa(uint16_t x, uint16_t y, uint16_t largeur, const char *texte)
{
  sFONT *policeCourante;
  uint16_t largeurTexte;
  uint16_t xTexte;

  policeCourante = BSP_LCD_GetFont();
  largeurTexte = (uint16_t)(strlen(texte) * policeCourante->Width);

  if (largeurTexte >= largeur)
  {
    xTexte = x;
  }
  else
  {
    xTexte = (uint16_t)(x + ((largeur - largeurTexte) / 2U));
  }

  BSP_LCD_DisplayStringAt(xTexte, y, (uint8_t *)texte, LEFT_MODE);
}

static uint8_t CoordonneesSontDansZoneIa(uint16_t x, uint16_t y,
                                         uint16_t zoneX, uint16_t zoneY,
                                         uint16_t largeur, uint16_t hauteur)
{
  return (uint8_t)((x >= zoneX) && (x < (zoneX + largeur)) &&
                   (y >= zoneY) && (y < (zoneY + hauteur)));
}

static uint8_t ConvertirCoordonneesEnCaseIa(uint16_t x, uint16_t y, uint8_t *ligne, uint8_t *colonne)
{
  if ((x < PLATEAU_IA_X) ||
      (x >= (PLATEAU_IA_X + (REGLES_IA_TAILLE_PLATEAU * TAILLE_CASE_IA))) ||
      (y < PLATEAU_IA_Y) ||
      (y >= (PLATEAU_IA_Y + (REGLES_IA_TAILLE_PLATEAU * TAILLE_CASE_IA))))
  {
    return 0U;
  }

  *colonne = (uint8_t)((x - PLATEAU_IA_X) / TAILLE_CASE_IA);
  *ligne = (uint8_t)((y - PLATEAU_IA_Y) / TAILLE_CASE_IA);
  return 1U;
}

static uint8_t JoueurHumainPeutJouerIa(const EtatDamesIa *etat)
{
  return (uint8_t)((etat->mode == DAMES_IA_MODE_JOUEUR_VS_IA) &&
                   (etat->etatJeu.partieTerminee == 0U) &&
                   (etat->etatJeu.joueurCourant == REGLES_IA_JOUEUR_BLANC));
}

static uint8_t PieceAppartientAuJoueurHumainIa(int8_t piece)
{
  return (uint8_t)((piece == REGLES_IA_PION_BLANC) || (piece == REGLES_IA_DAME_BLANCHE));
}

static void SelectionnerCaseIa(EtatDamesIa *etat, uint8_t ligne, uint8_t colonne)
{
  etat->selectionActive = 1U;
  etat->ligneSelection = ligne;
  etat->colonneSelection = colonne;
}

static void DeselectionnerCaseIa(EtatDamesIa *etat)
{
  etat->selectionActive = 0U;
  etat->ligneSelection = 0U;
  etat->colonneSelection = 0U;
}

static uint8_t TenterJouerCoupHumainIa(EtatDamesIa *etat, uint8_t ligneArrivee, uint8_t colonneArrivee)
{
  ListeCoupsIaStm listeCoups;
  uint32_t indexCoup;
  EtatJeuIaStm nouvelEtat;

  ReglesIaStm_GenererCoupsPossibles(&etat->etatJeu, &listeCoups);

  for (indexCoup = 0U; indexCoup < listeCoups.nbCoups; indexCoup++)
  {
    const CoupIaStm *coup = &listeCoups.coups[indexCoup];

    if ((coup->ligneDepart != etat->ligneSelection) ||
        (coup->colonneDepart != etat->colonneSelection) ||
        (coup->ligneArrivee != ligneArrivee) ||
        (coup->colonneArrivee != colonneArrivee))
    {
      continue;
    }

    if (ReglesIaStm_AppliquerCoup(&etat->etatJeu, coup, &nouvelEtat) == 0U)
    {
      return 0U;
    }

    etat->etatJeu = nouvelEtat;
    DeselectionnerCaseIa(etat);
    return 1U;
  }

  return 0U;
}
