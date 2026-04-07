#include "menu.h"

#include "string.h"
#include "stm32746g_discovery_lcd.h"

#define COULEUR_ACCUEIL_FOND   ((uint32_t)0xFFF8F3EA)
#define COULEUR_CARTE_JEU      ((uint32_t)0xFFE4D1B0)
#define COULEUR_CARTE_BORDURE  ((uint32_t)0xFF7B4F29)
#define COULEUR_TITRE_ACCUEIL  ((uint32_t)0xFF4F2F1A)

#define CARTE_JEU_X            120U
#define CARTE_JEU_Y            86U
#define CARTE_JEU_LARGEUR      240U
#define CARTE_JEU_HAUTEUR      90U

static uint8_t CoordonneesSontDansZone(uint16_t x, uint16_t y,
                                       uint16_t zoneX, uint16_t zoneY,
                                       uint16_t largeur, uint16_t hauteur);
static void AfficherTexteCentreZone(uint16_t x, uint16_t y, uint16_t largeur, const char *texte);

void Menu_Afficher(void)
{
  BSP_LCD_SelectLayer(0);
  BSP_LCD_Clear(COULEUR_ACCUEIL_FOND);

  BSP_LCD_SetFont(&Font24);
  BSP_LCD_SetTextColor(COULEUR_TITRE_ACCUEIL);
  BSP_LCD_SetBackColor(COULEUR_ACCUEIL_FOND);
  AfficherTexteCentreZone(0, 24, (uint16_t)BSP_LCD_GetXSize(), "JEUX");

  BSP_LCD_SetTextColor(COULEUR_CARTE_BORDURE);
  BSP_LCD_FillRect(CARTE_JEU_X, CARTE_JEU_Y, CARTE_JEU_LARGEUR, CARTE_JEU_HAUTEUR);

  BSP_LCD_SetTextColor(COULEUR_CARTE_JEU);
  BSP_LCD_FillRect((uint16_t)(CARTE_JEU_X + 3U), (uint16_t)(CARTE_JEU_Y + 3U),
                   (uint16_t)(CARTE_JEU_LARGEUR - 6U), (uint16_t)(CARTE_JEU_HAUTEUR - 6U));

  BSP_LCD_SetFont(&Font20);
  BSP_LCD_SetTextColor(COULEUR_TITRE_ACCUEIL);
  BSP_LCD_SetBackColor(COULEUR_CARTE_JEU);
  AfficherTexteCentreZone(CARTE_JEU_X, (uint16_t)(CARTE_JEU_Y + 18U), CARTE_JEU_LARGEUR, "Jeu de dames");

  BSP_LCD_SetFont(&Font12);
  AfficherTexteCentreZone(CARTE_JEU_X, (uint16_t)(CARTE_JEU_Y + 52U), CARTE_JEU_LARGEUR, "Toucher pour jouer");

  BSP_LCD_SelectLayer(1);
  BSP_LCD_Clear(0x00000000);
}

MenuAction Menu_GererTouch(uint16_t x, uint16_t y)
{
  if (CoordonneesSontDansZone(x, y, CARTE_JEU_X, CARTE_JEU_Y, CARTE_JEU_LARGEUR, CARTE_JEU_HAUTEUR) != 0U)
  {
    return MENU_ACTION_JEU_DAMES;
  }

  return MENU_ACTION_AUCUNE;
}

static uint8_t CoordonneesSontDansZone(uint16_t x, uint16_t y,
                                       uint16_t zoneX, uint16_t zoneY,
                                       uint16_t largeur, uint16_t hauteur)
{
  return (uint8_t)((x >= zoneX) && (x < (zoneX + largeur)) &&
                   (y >= zoneY) && (y < (zoneY + hauteur)));
}

static void AfficherTexteCentreZone(uint16_t x, uint16_t y, uint16_t largeur, const char *texte)
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
