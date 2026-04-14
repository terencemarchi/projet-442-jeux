#ifndef TEST_UART_H
#define TEST_UART_H

#include "stdint.h"
#include "usart.h"

void TestUart_Initialiser(void);
uint8_t TestUart_EnvoyerMessage(const char *message);
uint8_t TestUart_MessageRecuEstPret(void);
uint8_t TestUart_RecupererDernierMessageRecu(char *message, uint16_t tailleMessage);
void TestUart_AcquitterDernierMessageRecu(void);
void TestUart_TraiterReceptionInterruption(UART_HandleTypeDef *huart);

#endif /* TEST_UART_H */
