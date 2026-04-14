#include "liaison_bluetooth.h"

#include "string.h"

#define TAILLE_BUFFER_RECEPTION_BLUETOOTH 160U

typedef struct
{
  uint8_t indexReception;
  uint8_t receptionArmee;
  uint8_t octetRecu;
  uint8_t messageRecuPret;
  char bufferReception[TAILLE_BUFFER_RECEPTION_BLUETOOTH];
  char dernierMessage[TAILLE_BUFFER_RECEPTION_BLUETOOTH];
} EtatLiaisonBluetooth;

static EtatLiaisonBluetooth etatLiaisonBluetooth;

static void ReinitialiserReceptionBluetooth(void);
static void DemarrerReceptionBluetooth(void);
static void TraiterOctetBluetoothRecu(uint8_t octetRecu);

void LiaisonBluetooth_Initialiser(void)
{
  memset(&etatLiaisonBluetooth, 0, sizeof(etatLiaisonBluetooth));
  ReinitialiserReceptionBluetooth();
  DemarrerReceptionBluetooth();
}

uint8_t LiaisonBluetooth_EnvoyerMessage(const char *message)
{
  uint16_t longueurMessage;

  if (message == NULL)
  {
    return 0U;
  }

  longueurMessage = (uint16_t)strlen(message);
  if (longueurMessage == 0U)
  {
    return 0U;
  }

  if (HAL_UART_Transmit(&huart6, (uint8_t *)message, longueurMessage, 100U) != HAL_OK)
  {
    return 0U;
  }

  return 1U;
}

uint8_t LiaisonBluetooth_MessageRecuEstPret(void)
{
  return etatLiaisonBluetooth.messageRecuPret;
}

uint8_t LiaisonBluetooth_RecupererDernierMessageRecu(char *message, uint16_t tailleMessage)
{
  size_t longueurMessage;

  if ((message == NULL) || (tailleMessage == 0U) || (etatLiaisonBluetooth.messageRecuPret == 0U))
  {
    return 0U;
  }

  longueurMessage = strlen(etatLiaisonBluetooth.dernierMessage);
  if (longueurMessage >= tailleMessage)
  {
    return 0U;
  }

  strncpy(message, etatLiaisonBluetooth.dernierMessage, tailleMessage - 1U);
  message[tailleMessage - 1U] = '\0';
  return 1U;
}

void LiaisonBluetooth_AcquitterDernierMessageRecu(void)
{
  etatLiaisonBluetooth.messageRecuPret = 0U;
}

void LiaisonBluetooth_TraiterReceptionInterruption(UART_HandleTypeDef *huart)
{
  if (huart->Instance != USART6)
  {
    return;
  }

  etatLiaisonBluetooth.receptionArmee = 0U;
  TraiterOctetBluetoothRecu(etatLiaisonBluetooth.octetRecu);
  DemarrerReceptionBluetooth();
}

static void ReinitialiserReceptionBluetooth(void)
{
  memset(etatLiaisonBluetooth.bufferReception, 0, sizeof(etatLiaisonBluetooth.bufferReception));
  memset(etatLiaisonBluetooth.dernierMessage, 0, sizeof(etatLiaisonBluetooth.dernierMessage));
  etatLiaisonBluetooth.indexReception = 0U;
  etatLiaisonBluetooth.messageRecuPret = 0U;
  etatLiaisonBluetooth.receptionArmee = 0U;
  etatLiaisonBluetooth.octetRecu = 0U;
}

static void DemarrerReceptionBluetooth(void)
{
  if (etatLiaisonBluetooth.receptionArmee != 0U)
  {
    return;
  }

  if (HAL_UART_Receive_IT(&huart6, &etatLiaisonBluetooth.octetRecu, 1U) == HAL_OK)
  {
    etatLiaisonBluetooth.receptionArmee = 1U;
  }
}

static void TraiterOctetBluetoothRecu(uint8_t octetRecu)
{
  if ((octetRecu == '\r') || (octetRecu == '\n'))
  {
    if (etatLiaisonBluetooth.indexReception == 0U)
    {
      return;
    }

    etatLiaisonBluetooth.bufferReception[etatLiaisonBluetooth.indexReception] = '\0';
    strncpy(etatLiaisonBluetooth.dernierMessage,
            etatLiaisonBluetooth.bufferReception,
            sizeof(etatLiaisonBluetooth.dernierMessage) - 1U);
    etatLiaisonBluetooth.dernierMessage[sizeof(etatLiaisonBluetooth.dernierMessage) - 1U] = '\0';
    etatLiaisonBluetooth.messageRecuPret = 1U;
    etatLiaisonBluetooth.indexReception = 0U;
    memset(etatLiaisonBluetooth.bufferReception, 0, sizeof(etatLiaisonBluetooth.bufferReception));
    return;
  }

  if (etatLiaisonBluetooth.indexReception >= (TAILLE_BUFFER_RECEPTION_BLUETOOTH - 1U))
  {
    etatLiaisonBluetooth.indexReception = 0U;
    memset(etatLiaisonBluetooth.bufferReception, 0, sizeof(etatLiaisonBluetooth.bufferReception));
  }

  etatLiaisonBluetooth.bufferReception[etatLiaisonBluetooth.indexReception] = (char)octetRecu;
  etatLiaisonBluetooth.indexReception++;
}
