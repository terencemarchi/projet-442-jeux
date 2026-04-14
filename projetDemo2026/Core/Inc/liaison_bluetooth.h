#ifndef LIAISON_BLUETOOTH_H
#define LIAISON_BLUETOOTH_H

#include "stdint.h"
#include "usart.h"

void LiaisonBluetooth_Initialiser(void);
uint8_t LiaisonBluetooth_EnvoyerMessage(const char *message);
uint8_t LiaisonBluetooth_MessageRecuEstPret(void);
uint8_t LiaisonBluetooth_RecupererDernierMessageRecu(char *message, uint16_t tailleMessage);
void LiaisonBluetooth_AcquitterDernierMessageRecu(void);
void LiaisonBluetooth_TraiterReceptionInterruption(UART_HandleTypeDef *huart);

#endif /* LIAISON_BLUETOOTH_H */
