/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma2d.h"
#include "i2c.h"
#include "ltdc.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define TAILLE_PLATEAU             10U

typedef enum
{
  CASE_VIDE = 0,
  PION_BLANC,
  PION_NOIR,
  DAME_BLANCHE,
  DAME_NOIRE
} TypeCase;

typedef enum
{
  JOUEUR_BLANC = 0,
  JOUEUR_NOIR
} TypeJoueur;

typedef struct
{
  uint8_t ligne;
  uint8_t colonne;
} PositionCase;

typedef struct
{
  TypeCase plateau[TAILLE_PLATEAU][TAILLE_PLATEAU];
  TypeJoueur joueurCourant;
  uint8_t selectionActive;
  PositionCase caseSelectionnee;
} EtatPartie;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TAILLE_CASE                24U
#define PLATEAU_X                  16U
#define PLATEAU_Y                  16U
#define TAILLE_PIXEL_PLATEAU       (TAILLE_PLATEAU * TAILLE_CASE)
#define EPAISSEUR_BORDURE_PLATEAU  2U
#define NB_LIGNES_PIONS            4U
#define RAYON_PION                 9U

#define COULEUR_CASE_CLAIRE        ((uint32_t)0xFFF1E3C6)
#define COULEUR_CASE_FONCEE        ((uint32_t)0xFF8B5A2B)
#define COULEUR_FOND_ECRAN         LCD_COLOR_WHITE
#define COULEUR_PION_BLANC         ((uint32_t)0xFFF7F3EB)
#define COULEUR_PION_NOIR          ((uint32_t)0xFF303030)
#define COULEUR_CONTOUR_PION_BLANC LCD_COLOR_BLACK
#define COULEUR_CONTOUR_PION_NOIR  LCD_COLOR_WHITE
#define COULEUR_SELECTION_CASE     LCD_COLOR_YELLOW
#define COULEUR_INFOS_JEU          LCD_COLOR_DARKBLUE
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static EtatPartie etatPartie;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void InitialiserPlateau(EtatPartie *etat);
static void PlacerPionsInitiaux(EtatPartie *etat);
static void InitialiserPartie(EtatPartie *etat);
static uint8_t CaseEstJouable(uint8_t ligne, uint8_t colonne);
static uint8_t CaseContientPieceDuJoueur(const EtatPartie *etat, uint8_t ligne, uint8_t colonne);
static uint8_t ConvertirCoordonneesEnCase(uint16_t xTactile, uint16_t yTactile, PositionCase *caseTouchee);
static void DeselectionnerCase(EtatPartie *etat);
static void SelectionnerCase(EtatPartie *etat, uint8_t ligne, uint8_t colonne);
static void DessinerPlateau(void);
static void ObtenirCentreCase(uint32_t ligne, uint32_t colonne, uint16_t *x, uint16_t *y);
static void DessinerPion(uint32_t ligne, uint32_t colonne, uint32_t couleurRemplissage, uint32_t couleurContour);
static void DessinerPions(const EtatPartie *etat);
static void DessinerSelection(const EtatPartie *etat);
static void DessinerInfosJeu(const EtatPartie *etat);
static void DessinerElementsJeu(const EtatPartie *etat);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void InitialiserPlateau(EtatPartie *etat)
{
  uint32_t ligne;
  uint32_t colonne;

  for (ligne = 0; ligne < TAILLE_PLATEAU; ligne++)
  {
    for (colonne = 0; colonne < TAILLE_PLATEAU; colonne++)
    {
      etat->plateau[ligne][colonne] = CASE_VIDE;
    }
  }
}

static void PlacerPionsInitiaux(EtatPartie *etat)
{
  uint32_t ligne;
  uint32_t colonne;

  for (ligne = 0; ligne < NB_LIGNES_PIONS; ligne++)
  {
    for (colonne = 0; colonne < TAILLE_PLATEAU; colonne++)
    {
      if (((ligne + colonne) % 2U) != 0U)
      {
        etat->plateau[ligne][colonne] = PION_BLANC;
      }
    }
  }

  for (ligne = TAILLE_PLATEAU - NB_LIGNES_PIONS; ligne < TAILLE_PLATEAU; ligne++)
  {
    for (colonne = 0; colonne < TAILLE_PLATEAU; colonne++)
    {
      if (((ligne + colonne) % 2U) != 0U)
      {
        etat->plateau[ligne][colonne] = PION_NOIR;
      }
    }
  }
}

static void InitialiserPartie(EtatPartie *etat)
{
  InitialiserPlateau(etat);
  PlacerPionsInitiaux(etat);

  etat->joueurCourant = JOUEUR_BLANC;
  etat->selectionActive = 0U;
  etat->caseSelectionnee.ligne = 0U;
  etat->caseSelectionnee.colonne = 0U;
}

static uint8_t CaseEstJouable(uint8_t ligne, uint8_t colonne)
{
  return (uint8_t)(((ligne + colonne) % 2U) != 0U);
}

static uint8_t CaseContientPieceDuJoueur(const EtatPartie *etat, uint8_t ligne, uint8_t colonne)
{
  TypeCase typeCase = etat->plateau[ligne][colonne];

  if (etat->joueurCourant == JOUEUR_BLANC)
  {
    return (uint8_t)((typeCase == PION_BLANC) || (typeCase == DAME_BLANCHE));
  }

  return (uint8_t)((typeCase == PION_NOIR) || (typeCase == DAME_NOIRE));
}

static uint8_t ConvertirCoordonneesEnCase(uint16_t xTactile, uint16_t yTactile, PositionCase *caseTouchee)
{
  if ((xTactile < PLATEAU_X) || (xTactile >= (PLATEAU_X + TAILLE_PIXEL_PLATEAU)) ||
      (yTactile < PLATEAU_Y) || (yTactile >= (PLATEAU_Y + TAILLE_PIXEL_PLATEAU)))
  {
    return 0U;
  }

  caseTouchee->colonne = (uint8_t)((xTactile - PLATEAU_X) / TAILLE_CASE);
  caseTouchee->ligne = (uint8_t)((yTactile - PLATEAU_Y) / TAILLE_CASE);

  return 1U;
}

static void DeselectionnerCase(EtatPartie *etat)
{
  etat->selectionActive = 0U;
  etat->caseSelectionnee.ligne = 0U;
  etat->caseSelectionnee.colonne = 0U;
}

static void SelectionnerCase(EtatPartie *etat, uint8_t ligne, uint8_t colonne)
{
  etat->selectionActive = 1U;
  etat->caseSelectionnee.ligne = ligne;
  etat->caseSelectionnee.colonne = colonne;
}

static void DessinerPlateau(void)
{
  uint32_t ligne;
  uint32_t colonne;
  uint16_t x;
  uint16_t y;

  BSP_LCD_SelectLayer(0);
  BSP_LCD_Clear(COULEUR_FOND_ECRAN);

  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_FillRect(PLATEAU_X - EPAISSEUR_BORDURE_PLATEAU,
                   PLATEAU_Y - EPAISSEUR_BORDURE_PLATEAU,
                   TAILLE_PIXEL_PLATEAU + (2U * EPAISSEUR_BORDURE_PLATEAU),
                   TAILLE_PIXEL_PLATEAU + (2U * EPAISSEUR_BORDURE_PLATEAU));

  for (ligne = 0; ligne < TAILLE_PLATEAU; ligne++)
  {
    for (colonne = 0; colonne < TAILLE_PLATEAU; colonne++)
    {
      x = PLATEAU_X + (uint16_t)(colonne * TAILLE_CASE);
      y = PLATEAU_Y + (uint16_t)(ligne * TAILLE_CASE);

      BSP_LCD_SetTextColor(((ligne + colonne) % 2U) == 0U ? COULEUR_CASE_CLAIRE : COULEUR_CASE_FONCEE);
      BSP_LCD_FillRect(x, y, TAILLE_CASE, TAILLE_CASE);
    }
  }
}

static void ObtenirCentreCase(uint32_t ligne, uint32_t colonne, uint16_t *x, uint16_t *y)
{
  *x = (uint16_t)(PLATEAU_X + (colonne * TAILLE_CASE) + (TAILLE_CASE / 2U));
  *y = (uint16_t)(PLATEAU_Y + (ligne * TAILLE_CASE) + (TAILLE_CASE / 2U));
}

static void DessinerPion(uint32_t ligne, uint32_t colonne, uint32_t couleurRemplissage, uint32_t couleurContour)
{
  uint16_t centreX;
  uint16_t centreY;

  ObtenirCentreCase(ligne, colonne, &centreX, &centreY);

  BSP_LCD_SetTextColor(couleurRemplissage);
  BSP_LCD_FillCircle(centreX, centreY, RAYON_PION);
  BSP_LCD_SetTextColor(couleurContour);
  BSP_LCD_DrawCircle(centreX, centreY, RAYON_PION);
}

static void DessinerPions(const EtatPartie *etat)
{
  uint32_t ligne;
  uint32_t colonne;

  BSP_LCD_SelectLayer(1);

  for (ligne = 0; ligne < TAILLE_PLATEAU; ligne++)
  {
    for (colonne = 0; colonne < TAILLE_PLATEAU; colonne++)
    {
      switch (etat->plateau[ligne][colonne])
      {
        case PION_BLANC:
          DessinerPion(ligne, colonne, COULEUR_PION_BLANC, COULEUR_CONTOUR_PION_BLANC);
          break;

        case PION_NOIR:
          DessinerPion(ligne, colonne, COULEUR_PION_NOIR, COULEUR_CONTOUR_PION_NOIR);
          break;

        case DAME_BLANCHE:
          DessinerPion(ligne, colonne, COULEUR_PION_BLANC, COULEUR_CONTOUR_PION_BLANC);
          break;

        case DAME_NOIRE:
          DessinerPion(ligne, colonne, COULEUR_PION_NOIR, COULEUR_CONTOUR_PION_NOIR);
          break;

        case CASE_VIDE:
        default:
          break;
      }
    }
  }
}

static void DessinerSelection(const EtatPartie *etat)
{
  uint16_t x;
  uint16_t y;

  if (etat->selectionActive == 0U)
  {
    return;
  }

  x = (uint16_t)(PLATEAU_X + (etat->caseSelectionnee.colonne * TAILLE_CASE));
  y = (uint16_t)(PLATEAU_Y + (etat->caseSelectionnee.ligne * TAILLE_CASE));

  BSP_LCD_SetTextColor(COULEUR_SELECTION_CASE);
  BSP_LCD_DrawRect(x, y, TAILLE_CASE, TAILLE_CASE);
  BSP_LCD_DrawRect((uint16_t)(x + 1U), (uint16_t)(y + 1U), TAILLE_CASE - 2U, TAILLE_CASE - 2U);
}

static void DessinerInfosJeu(const EtatPartie *etat)
{
  char texte[40];

  BSP_LCD_SetFont(&Font12);
  BSP_LCD_SetTextColor(COULEUR_INFOS_JEU);
  BSP_LCD_SetBackColor(0x00000000);

  snprintf(texte, sizeof(texte), "Tour : %s", etat->joueurCourant == JOUEUR_BLANC ? "blanc" : "noir");
  BSP_LCD_DisplayStringAt(280, 24, (uint8_t *)texte, LEFT_MODE);
}

static void DessinerElementsJeu(const EtatPartie *etat)
{
  BSP_LCD_SelectLayer(1);
  BSP_LCD_Clear(0x00000000);

  DessinerPions(etat);
  DessinerSelection(etat);
  DessinerInfosJeu(etat);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  TS_StateTypeDef etatTactile = {0};
  PositionCase caseTouchee = {0};
  uint8_t tactileActifPrecedent = 0U;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC3_Init();
  MX_DMA2D_Init();
  MX_FMC_Init();
  MX_I2C1_Init();
  MX_I2C3_Init();
  MX_LTDC_Init();
  MX_RTC_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM5_Init();
  MX_TIM8_Init();
  MX_USART1_UART_Init();
  MX_USART6_UART_Init();
  MX_ADC1_Init();
  MX_DAC_Init();
  MX_UART7_Init();
  /* USER CODE BEGIN 2 */
  InitialiserPartie(&etatPartie);
  BSP_LCD_Init();
  BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
  BSP_LCD_LayerDefaultInit(1, LCD_FB_START_ADDRESS+ BSP_LCD_GetXSize()*BSP_LCD_GetYSize()*4);
  BSP_LCD_DisplayOn();
  DessinerPlateau();
  DessinerElementsJeu(&etatPartie);
  BSP_LCD_SelectLayer(1);
  BSP_LCD_SetFont(&Font12);
  BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
  BSP_LCD_SetBackColor(00);

  BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    BSP_TS_GetState(&etatTactile);

    if ((etatTactile.touchDetected != 0U) && (tactileActifPrecedent == 0U))
    {
      if (ConvertirCoordonneesEnCase(etatTactile.touchX[0], etatTactile.touchY[0], &caseTouchee) != 0U)
      {
        if ((etatPartie.selectionActive != 0U) &&
            (etatPartie.caseSelectionnee.ligne == caseTouchee.ligne) &&
            (etatPartie.caseSelectionnee.colonne == caseTouchee.colonne))
        {
          DeselectionnerCase(&etatPartie);
        }
        else if (CaseEstJouable(caseTouchee.ligne, caseTouchee.colonne) != 0U &&
                 CaseContientPieceDuJoueur(&etatPartie, caseTouchee.ligne, caseTouchee.colonne) != 0U)
        {
          SelectionnerCase(&etatPartie, caseTouchee.ligne, caseTouchee.colonne);
        }
        else
        {
          DeselectionnerCase(&etatPartie);
        }
      }
      else
      {
        DeselectionnerCase(&etatPartie);
      }

      DessinerElementsJeu(&etatPartie);
    }

    tactileActifPrecedent = (etatTactile.touchDetected != 0U) ? 1U : 0U;

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
