#include "test_uart.h"

#include "stdio.h"
#include "string.h"
#include "usart.h"
#include "stm32746g_discovery_lcd.h"

#define COULEUR_FOND_TEST_UART      ((uint32_t)0xFFF8F3EA)
#define COULEUR_TITRE_TEST_UART     ((uint32_t)0xFF4F2F1A)
#define COULEUR_CARTE_TEST_UART     ((uint32_t)0xFFE4D1B0)
#define COULEUR_BORDURE_TEST_UART   ((uint32_t)0xFF7B4F29)
#define COULEUR_BOUTON_RETOUR       ((uint32_t)0xFFD9534F)
#define COULEUR_TEXTE_RETOUR        LCD_COLOR_WHITE
#define COULEUR_TEXTE_INFO          LCD_COLOR_DARKBLUE
#define COULEUR_STATUT_OK           LCD_COLOR_DARKGREEN
#define COULEUR_STATUT_ERREUR       LCD_COLOR_RED

#define CARTE_ROLE_X                100U
#define CARTE_ROLE_LARGEUR          280U
#define CARTE_ROLE_HAUTEUR          56U
#define CARTE_EMETTEUR_Y            82U
#define CARTE_RECEPTEUR_Y           150U

#define BOUTON_RETOUR_X             160U
#define BOUTON_RETOUR_Y             214U
#define BOUTON_RETOUR_LARGEUR       160U
#define BOUTON_RETOUR_HAUTEUR       36U

#define PERIODE_EMISSION_MS         1000U
#define TAILLE_MESSAGE_TEST         5U
#define TAILLE_BUFFER_RECEPTION     32U

typedef enum
{
  ECRAN_TEST_UART_CHOIX = 0,
  ECRAN_TEST_UART_EMETTEUR,
  ECRAN_TEST_UART_RECEPTEUR
} TypeEcranTestUart;

typedef struct
{
  TypeEcranTestUart ecranCourant;
  uint32_t nbMessagesEmis;
  uint32_t nbMessagesRecus;
  uint32_t dernierEnvoiMs;
  uint8_t indexReception;
  uint8_t messageValide;
  uint8_t receptionArmee;
  uint8_t octetRecu;
  char bufferReception[TAILLE_BUFFER_RECEPTION];
  char dernierMessage[TAILLE_BUFFER_RECEPTION];
} EtatTestUart;

static EtatTestUart etatTestUart;
static const char MESSAGE_TEST[TAILLE_MESSAGE_TEST + 1U] = "TEST\n";

static void AfficherChoixRole(void);
static void AfficherEcranEmetteur(void);
static void AfficherEcranRecepteur(void);
static void AfficherEcranCourant(void);
static void AfficherTexteCentreZone(uint16_t x, uint16_t y, uint16_t largeur, const char *texte);
static uint8_t CoordonneesSontDansZone(uint16_t x, uint16_t y, uint16_t zoneX, uint16_t zoneY, uint16_t largeur, uint16_t hauteur);
static void DessinerCarte(uint16_t x, uint16_t y, uint16_t largeur, uint16_t hauteur, const char *titre, const char *sousTitre);
static void DessinerBoutonRetour(const char *texte);
static void ReinitialiserReception(void);
static void DemarrerReceptionUart(void);
static void TraiterOctetRecu(uint8_t octetRecu);

void TestUart_Afficher(void)
{
  memset(&etatTestUart, 0, sizeof(etatTestUart));
  etatTestUart.ecranCourant = ECRAN_TEST_UART_CHOIX;
  AfficherEcranCourant();
}

TestUartAction TestUart_GererTouch(uint16_t x, uint16_t y)
{
  if (etatTestUart.ecranCourant == ECRAN_TEST_UART_CHOIX)
  {
    if (CoordonneesSontDansZone(x, y, CARTE_ROLE_X, CARTE_EMETTEUR_Y, CARTE_ROLE_LARGEUR, CARTE_ROLE_HAUTEUR) != 0U)
    {
      etatTestUart.ecranCourant = ECRAN_TEST_UART_EMETTEUR;
      etatTestUart.dernierEnvoiMs = HAL_GetTick();
      AfficherEcranCourant();
      return TEST_UART_ACTION_AUCUNE;
    }

    if (CoordonneesSontDansZone(x, y, CARTE_ROLE_X, CARTE_RECEPTEUR_Y, CARTE_ROLE_LARGEUR, CARTE_ROLE_HAUTEUR) != 0U)
    {
      etatTestUart.ecranCourant = ECRAN_TEST_UART_RECEPTEUR;
      ReinitialiserReception();
      DemarrerReceptionUart();
      AfficherEcranCourant();
      return TEST_UART_ACTION_AUCUNE;
    }
  }

  if (CoordonneesSontDansZone(x, y, BOUTON_RETOUR_X, BOUTON_RETOUR_Y,
                              BOUTON_RETOUR_LARGEUR, BOUTON_RETOUR_HAUTEUR) != 0U)
  {
    return TEST_UART_ACTION_QUITTER;
  }

  return TEST_UART_ACTION_AUCUNE;
}

void TestUart_MettreAJour(void)
{
  if (etatTestUart.ecranCourant == ECRAN_TEST_UART_EMETTEUR)
  {
    if ((HAL_GetTick() - etatTestUart.dernierEnvoiMs) >= PERIODE_EMISSION_MS)
    {
      if (HAL_UART_Transmit(&huart7, (uint8_t *)MESSAGE_TEST, TAILLE_MESSAGE_TEST, 100U) == HAL_OK)
      {
        etatTestUart.nbMessagesEmis++;
        etatTestUart.dernierEnvoiMs = HAL_GetTick();
        AfficherEcranEmetteur();
      }
    }
  }
}

static void AfficherChoixRole(void)
{
  BSP_LCD_SelectLayer(0);
  BSP_LCD_Clear(COULEUR_FOND_TEST_UART);

  BSP_LCD_SetFont(&Font24);
  BSP_LCD_SetTextColor(COULEUR_TITRE_TEST_UART);
  BSP_LCD_SetBackColor(COULEUR_FOND_TEST_UART);
  AfficherTexteCentreZone(0, 24U, (uint16_t)BSP_LCD_GetXSize(), "Test UART7");

  DessinerCarte(CARTE_ROLE_X, CARTE_EMETTEUR_Y, CARTE_ROLE_LARGEUR, CARTE_ROLE_HAUTEUR,
                "Emetteur", "Envoie TEST toutes les secondes");
  DessinerCarte(CARTE_ROLE_X, CARTE_RECEPTEUR_Y, CARTE_ROLE_LARGEUR, CARTE_ROLE_HAUTEUR,
                "Recepteur", "Attend TEST depuis l'autre carte");
  DessinerBoutonRetour("Retour menu");

  BSP_LCD_SelectLayer(1);
  BSP_LCD_Clear(0x00000000);
}

static void AfficherEcranEmetteur(void)
{
  char texte[48];

  BSP_LCD_SelectLayer(0);
  BSP_LCD_Clear(COULEUR_FOND_TEST_UART);

  BSP_LCD_SetFont(&Font24);
  BSP_LCD_SetTextColor(COULEUR_TITRE_TEST_UART);
  BSP_LCD_SetBackColor(COULEUR_FOND_TEST_UART);
  AfficherTexteCentreZone(0, 24U, (uint16_t)BSP_LCD_GetXSize(), "UART7 - Emetteur");

  BSP_LCD_SetFont(&Font16);
  BSP_LCD_SetTextColor(COULEUR_TEXTE_INFO);
  BSP_LCD_DisplayStringAt(56U, 94U, (uint8_t *)"Message envoye :", LEFT_MODE);
  BSP_LCD_DisplayStringAt(56U, 126U, (uint8_t *)"Nombre d'envois :", LEFT_MODE);
  BSP_LCD_DisplayStringAt(56U, 158U, (uint8_t *)"Liaison : UART7", LEFT_MODE);

  BSP_LCD_SetTextColor(COULEUR_STATUT_OK);
  BSP_LCD_DisplayStringAt(260U, 94U, (uint8_t *)"TEST", LEFT_MODE);
  snprintf(texte, sizeof(texte), "%lu", (unsigned long)etatTestUart.nbMessagesEmis);
  BSP_LCD_DisplayStringAt(260U, 126U, (uint8_t *)texte, LEFT_MODE);

  DessinerBoutonRetour("Retour menu");

  BSP_LCD_SelectLayer(1);
  BSP_LCD_Clear(0x00000000);
}

static void AfficherEcranRecepteur(void)
{
  char texte[48];

  BSP_LCD_SelectLayer(0);
  BSP_LCD_Clear(COULEUR_FOND_TEST_UART);

  BSP_LCD_SetFont(&Font24);
  BSP_LCD_SetTextColor(COULEUR_TITRE_TEST_UART);
  BSP_LCD_SetBackColor(COULEUR_FOND_TEST_UART);
  AfficherTexteCentreZone(0, 24U, (uint16_t)BSP_LCD_GetXSize(), "UART7 - Recepteur");

  BSP_LCD_SetFont(&Font16);
  BSP_LCD_SetTextColor(COULEUR_TEXTE_INFO);
  BSP_LCD_DisplayStringAt(40U, 88U, (uint8_t *)"Dernier message :", LEFT_MODE);
  BSP_LCD_DisplayStringAt(40U, 122U, (uint8_t *)"Messages recus :", LEFT_MODE);
  BSP_LCD_DisplayStringAt(40U, 156U, (uint8_t *)"Etat :", LEFT_MODE);

  BSP_LCD_SetTextColor(etatTestUart.messageValide != 0U ? COULEUR_STATUT_OK : COULEUR_STATUT_ERREUR);
  BSP_LCD_DisplayStringAt(220U, 88U, (uint8_t *)etatTestUart.dernierMessage, LEFT_MODE);
  snprintf(texte, sizeof(texte), "%lu", (unsigned long)etatTestUart.nbMessagesRecus);
  BSP_LCD_DisplayStringAt(220U, 122U, (uint8_t *)texte, LEFT_MODE);
  BSP_LCD_DisplayStringAt(220U, 156U,
                          (uint8_t *)(etatTestUart.messageValide != 0U ? "Message recu" : "En attente"),
                          LEFT_MODE);

  DessinerBoutonRetour("Retour menu");

  BSP_LCD_SelectLayer(1);
  BSP_LCD_Clear(0x00000000);
}

static void AfficherEcranCourant(void)
{
  if (etatTestUart.ecranCourant == ECRAN_TEST_UART_EMETTEUR)
  {
    AfficherEcranEmetteur();
  }
  else if (etatTestUart.ecranCourant == ECRAN_TEST_UART_RECEPTEUR)
  {
    AfficherEcranRecepteur();
  }
  else
  {
    AfficherChoixRole();
  }
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

static uint8_t CoordonneesSontDansZone(uint16_t x, uint16_t y, uint16_t zoneX, uint16_t zoneY, uint16_t largeur, uint16_t hauteur)
{
  return (uint8_t)((x >= zoneX) && (x < (zoneX + largeur)) &&
                   (y >= zoneY) && (y < (zoneY + hauteur)));
}

static void DessinerCarte(uint16_t x, uint16_t y, uint16_t largeur, uint16_t hauteur, const char *titre, const char *sousTitre)
{
  BSP_LCD_SetTextColor(COULEUR_BORDURE_TEST_UART);
  BSP_LCD_FillRect(x, y, largeur, hauteur);

  BSP_LCD_SetTextColor(COULEUR_CARTE_TEST_UART);
  BSP_LCD_FillRect((uint16_t)(x + 3U), (uint16_t)(y + 3U),
                   (uint16_t)(largeur - 6U), (uint16_t)(hauteur - 6U));

  BSP_LCD_SetTextColor(COULEUR_TITRE_TEST_UART);
  BSP_LCD_SetBackColor(COULEUR_CARTE_TEST_UART);
  BSP_LCD_SetFont(&Font16);
  AfficherTexteCentreZone(x, (uint16_t)(y + 8U), largeur, titre);

  BSP_LCD_SetFont(&Font12);
  AfficherTexteCentreZone(x, (uint16_t)(y + 30U), largeur, sousTitre);
}

static void DessinerBoutonRetour(const char *texte)
{
  BSP_LCD_SetTextColor(COULEUR_BOUTON_RETOUR);
  BSP_LCD_FillRect(BOUTON_RETOUR_X, BOUTON_RETOUR_Y, BOUTON_RETOUR_LARGEUR, BOUTON_RETOUR_HAUTEUR);

  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_DrawRect(BOUTON_RETOUR_X, BOUTON_RETOUR_Y, BOUTON_RETOUR_LARGEUR, BOUTON_RETOUR_HAUTEUR);

  BSP_LCD_SetFont(&Font12);
  BSP_LCD_SetTextColor(COULEUR_TEXTE_RETOUR);
  BSP_LCD_SetBackColor(COULEUR_BOUTON_RETOUR);
  AfficherTexteCentreZone(BOUTON_RETOUR_X, (uint16_t)(BOUTON_RETOUR_Y + 10U), BOUTON_RETOUR_LARGEUR, texte);
}

static void ReinitialiserReception(void)
{
  memset(etatTestUart.bufferReception, 0, sizeof(etatTestUart.bufferReception));
  memset(etatTestUart.dernierMessage, 0, sizeof(etatTestUart.dernierMessage));
  etatTestUart.indexReception = 0U;
  etatTestUart.nbMessagesRecus = 0U;
  etatTestUart.messageValide = 0U;
  etatTestUart.receptionArmee = 0U;
  etatTestUart.octetRecu = 0U;
}

static void DemarrerReceptionUart(void)
{
  if (etatTestUart.ecranCourant != ECRAN_TEST_UART_RECEPTEUR)
  {
    return;
  }

  if (HAL_UART_Receive_IT(&huart7, &etatTestUart.octetRecu, 1U) == HAL_OK)
  {
    etatTestUart.receptionArmee = 1U;
  }
}

static void TraiterOctetRecu(uint8_t octetRecu)
{
  if ((octetRecu == '\r') || (octetRecu == '\n'))
  {
    if (etatTestUart.indexReception == 0U)
    {
      return;
    }

    etatTestUart.bufferReception[etatTestUart.indexReception] = '\0';
    strncpy(etatTestUart.dernierMessage, etatTestUart.bufferReception, sizeof(etatTestUart.dernierMessage) - 1U);
    etatTestUart.nbMessagesRecus++;
    etatTestUart.messageValide = (uint8_t)(strcmp(etatTestUart.bufferReception, "TEST") == 0);
    etatTestUart.indexReception = 0U;
    memset(etatTestUart.bufferReception, 0, sizeof(etatTestUart.bufferReception));
    AfficherEcranRecepteur();
    return;
  }

  if (etatTestUart.indexReception >= (TAILLE_BUFFER_RECEPTION - 1U))
  {
    etatTestUart.indexReception = 0U;
    memset(etatTestUart.bufferReception, 0, sizeof(etatTestUart.bufferReception));
  }

  etatTestUart.bufferReception[etatTestUart.indexReception] = (char)octetRecu;
  etatTestUart.indexReception++;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance != UART7)
  {
    return;
  }

  etatTestUart.receptionArmee = 0U;

  if (etatTestUart.ecranCourant == ECRAN_TEST_UART_RECEPTEUR)
  {
    TraiterOctetRecu(etatTestUart.octetRecu);
    DemarrerReceptionUart();
  }
}
