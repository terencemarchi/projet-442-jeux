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
#include "string.h"
#include "menu.h"
#include "dames.h"
#include "test_uart.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum
{
  ECRAN_ACCUEIL = 0,
  ECRAN_DAMES,
  ECRAN_BLUETOOTH,
  ECRAN_TEST_UART
} TypeEcran;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define COULEUR_FOND_BLUETOOTH    ((uint32_t)0xFFF8F3EA)
#define COULEUR_TITRE_BLUETOOTH   ((uint32_t)0xFF4F2F1A)
#define COULEUR_BOUTON_RETOUR     ((uint32_t)0xFFD9534F)
#define COULEUR_TEXTE_RETOUR      LCD_COLOR_WHITE

#define BOUTON_RETOUR_X           160U
#define BOUTON_RETOUR_Y           206U
#define BOUTON_RETOUR_LARGEUR     160U
#define BOUTON_RETOUR_HAUTEUR     36U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static TypeEcran ecranCourant = ECRAN_ACCUEIL;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void AfficherEcranDames(void);
static void AfficherEcranBluetooth(void);
static void AfficherEcranTestUart(void);
static void AfficherSousMenuDames(void);
static void AfficherEcranAccueil(void);
static void AfficherTexteCentreZone(uint16_t x, uint16_t y, uint16_t largeur, const char *texte);
static uint8_t CoordonneesSontDansZone(uint16_t x, uint16_t y, uint16_t zoneX, uint16_t zoneY, uint16_t largeur, uint16_t hauteur);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void AfficherEcranDames(void)
{
  ecranCourant = ECRAN_DAMES;
  Dames_AfficherNouvellePartie();
}

static void AfficherEcranBluetooth(void)
{
  ecranCourant = ECRAN_BLUETOOTH;

  BSP_LCD_SelectLayer(0);
  BSP_LCD_Clear(COULEUR_FOND_BLUETOOTH);

  BSP_LCD_SetFont(&Font24);
  BSP_LCD_SetTextColor(COULEUR_TITRE_BLUETOOTH);
  BSP_LCD_SetBackColor(COULEUR_FOND_BLUETOOTH);
  AfficherTexteCentreZone(0, 34, (uint16_t)BSP_LCD_GetXSize(), "Mode Bluetooth");

  BSP_LCD_SetFont(&Font16);
  AfficherTexteCentreZone(0, 96, (uint16_t)BSP_LCD_GetXSize(), "Implementation a venir");

  BSP_LCD_SetFont(&Font12);
  AfficherTexteCentreZone(0, 126, (uint16_t)BSP_LCD_GetXSize(), "Le menu est pret pour ce mode");

  BSP_LCD_SetTextColor(COULEUR_BOUTON_RETOUR);
  BSP_LCD_FillRect(BOUTON_RETOUR_X, BOUTON_RETOUR_Y, BOUTON_RETOUR_LARGEUR, BOUTON_RETOUR_HAUTEUR);

  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_DrawRect(BOUTON_RETOUR_X, BOUTON_RETOUR_Y, BOUTON_RETOUR_LARGEUR, BOUTON_RETOUR_HAUTEUR);

  BSP_LCD_SetFont(&Font12);
  BSP_LCD_SetTextColor(COULEUR_TEXTE_RETOUR);
  BSP_LCD_SetBackColor(COULEUR_BOUTON_RETOUR);
  AfficherTexteCentreZone(BOUTON_RETOUR_X, (uint16_t)(BOUTON_RETOUR_Y + 10U), BOUTON_RETOUR_LARGEUR, "Retour menu");

  BSP_LCD_SelectLayer(1);
  BSP_LCD_Clear(0x00000000);
}

static void AfficherEcranTestUart(void)
{
  ecranCourant = ECRAN_TEST_UART;
  TestUart_Afficher();
}

static void AfficherSousMenuDames(void)
{
  ecranCourant = ECRAN_ACCUEIL;
  Menu_AfficherSousMenuDames();
}

static void AfficherEcranAccueil(void)
{
  ecranCourant = ECRAN_ACCUEIL;
  Menu_Reinitialiser();
  Menu_Afficher();
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

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  TS_StateTypeDef etatTactile = {0};
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
  BSP_LCD_Init();
  BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
  BSP_LCD_LayerDefaultInit(1, LCD_FB_START_ADDRESS+ BSP_LCD_GetXSize()*BSP_LCD_GetYSize()*4);
  BSP_LCD_DisplayOn();
  AfficherEcranAccueil();
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
      if (ecranCourant == ECRAN_ACCUEIL)
      {
        MenuAction actionMenu;

        actionMenu = Menu_GererTouch(etatTactile.touchX[0], etatTactile.touchY[0]);

        if (actionMenu == MENU_ACTION_LANCER_DAMES_LOCAL)
        {
          AfficherEcranDames();
        }
        else if (actionMenu == MENU_ACTION_LANCER_DAMES_BLUETOOTH)
        {
          AfficherEcranBluetooth();
        }
        else if (actionMenu == MENU_ACTION_LANCER_TEST_UART)
        {
          AfficherEcranTestUart();
        }
      }
      else if (ecranCourant == ECRAN_DAMES)
      {
        if (Dames_GererTouch(etatTactile.touchX[0], etatTactile.touchY[0]) == DAMES_ACTION_QUITTER)
        {
          AfficherEcranAccueil();
        }
      }
      else if (ecranCourant == ECRAN_BLUETOOTH)
      {
        if (CoordonneesSontDansZone(etatTactile.touchX[0], etatTactile.touchY[0],
                                    BOUTON_RETOUR_X, BOUTON_RETOUR_Y,
                                    BOUTON_RETOUR_LARGEUR, BOUTON_RETOUR_HAUTEUR) != 0U)
        {
          AfficherSousMenuDames();
        }
      }
      else if (ecranCourant == ECRAN_TEST_UART)
      {
        if (TestUart_GererTouch(etatTactile.touchX[0], etatTactile.touchY[0]) == TEST_UART_ACTION_QUITTER)
        {
          AfficherEcranAccueil();
        }
      }
    }

    if (ecranCourant == ECRAN_TEST_UART)
    {
      TestUart_MettreAJour();
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
