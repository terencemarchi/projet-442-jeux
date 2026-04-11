#ifndef TEST_UART_H
#define TEST_UART_H

#include "stdint.h"

typedef enum
{
  TEST_UART_ACTION_AUCUNE = 0,
  TEST_UART_ACTION_QUITTER
} TestUartAction;

void TestUart_Initialiser(void);
void TestUart_Afficher(void);
TestUartAction TestUart_GererTouch(uint16_t x, uint16_t y);
void TestUart_MettreAJour(void);
uint8_t TestUart_EnvoyerMessage(const char *message);

#endif /* TEST_UART_H */
