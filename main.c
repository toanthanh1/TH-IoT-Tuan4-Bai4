/***************************************************************************//**
 * @file main.c
 * @brief main() function.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "sl_component_catalog.h"
#include "sl_system_init.h"
#include "app.h"
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif
#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "sl_system_kernel.h"
#else // SL_CATALOG_KERNEL_PRESENT
#include "sl_system_process_action.h"
#endif // SL_CATALOG_KERNEL_PRESENT

#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"
#include "em_ldma.h"
#include "em_emu.h"

#include "em_timer.h"
#include <stdio.h> // Thêm dòng này

/**************************************************************************//**
 * DEFINE
 *****************************************************************************/

#define BSP_TXPORT gpioPortA
#define BSP_RXPORT gpioPortA
#define BSP_TXPIN 5
#define BSP_RXPIN 6
#define BSP_ENABLE_PORT gpioPortD
#define BSP_ENABLE_PIN 4
// LDMA channel for receive and transmit servicing
#define RX_LDMA_CHANNEL 0
#define TX_LDMA_CHANNEL 1

/**************************************************************************//**
 * STATIC VARIABLES
 *****************************************************************************/
// LDMA descriptor and transfer configuration structures for USART TX channel
LDMA_Descriptor_t ldmaTXDescriptor;
LDMA_TransferCfg_t ldmaTXConfig;

// LDMA descriptor and transfer configuration structures for USART RX channel
LDMA_Descriptor_t ldmaRXDescriptor;
LDMA_TransferCfg_t ldmaRXConfig;

// Size of the data buffers
#define BUFLEN  8

// Outgoing data
uint8_t outbuf[BUFLEN];

// Incoming data
uint8_t inbuf[BUFLEN];

// Data reception complete
bool rx_done;

// Số lượng ký tự tối đa nhận (có thể tăng lên nếu cần)
#define MAX_LEN 120 // thu voi 120 ky tu

uint8_t rx_buffer[MAX_LEN];
uint32_t rx_len = 0;

/**************************************************************************//**
 * @brief
 *    GPIO initialization
 *****************************************************************************/
void initGPIO(void)
{
  // Configure the USART TX pin to the board controller as an output
  GPIO_PinModeSet(BSP_TXPORT, BSP_TXPIN, gpioModePushPull, 1);

  // Configure the USART RX pin to the board controller as an input
  GPIO_PinModeSet(BSP_RXPORT, BSP_RXPIN, gpioModeInput, 0);

  /*
   * Configure the BCC_ENABLE pin as output and set high.  This enables
   * the virtual COM port (VCOM) connection to the board controller and
   * permits serial port traffic over the debug connection to the host
   * PC.
   *
   * To disable the VCOM connection and use the pins on the kit
   * expansion (EXP) header, comment out the following line.
   */
  GPIO_PinModeSet(BSP_ENABLE_PORT, BSP_ENABLE_PIN, gpioModePushPull, 1);
}
/**************************************************************************//**
 * @brief
 *    USART0 initialization
 *****************************************************************************/
void initUSART0(void)
{
  // Default asynchronous initializer (115.2 Kbps, 8N1, no flow control)
  USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;

  // Route USART0 TX and RX to the board controller TX and RX pins
  GPIO->USARTROUTE[0].TXROUTE = (BSP_TXPORT << _GPIO_USART_TXROUTE_PORT_SHIFT)
      | (BSP_TXPIN << _GPIO_USART_TXROUTE_PIN_SHIFT);
  GPIO->USARTROUTE[0].RXROUTE = (BSP_RXPORT << _GPIO_USART_RXROUTE_PORT_SHIFT)
      | (BSP_RXPIN << _GPIO_USART_RXROUTE_PIN_SHIFT);

  // Enable RX and TX signals now that they have been routed
  GPIO->USARTROUTE[0].ROUTEEN = GPIO_USART_ROUTEEN_RXPEN | GPIO_USART_ROUTEEN_TXPEN;

  // Configure and enable USART0
  USART_InitAsync(USART0, &init);
}

void initLDMA(void)
{
  // First, initialize the LDMA unit itself
  LDMA_Init_t ldmaInit = LDMA_INIT_DEFAULT;
  LDMA_Init(&ldmaInit);

  // Source is outbuf, destination is USART0_TXDATA, and length is BUFLEN
  ldmaTXDescriptor = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_M2P_BYTE(outbuf, &(USART0->TXDATA), BUFLEN);

  ldmaTXDescriptor.xfer.blockSize = ldmaCtrlBlockSizeUnit1;

  ldmaTXConfig = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_USART0_TXBL);

  // Source is USART0_RXDATA, destination is inbuf, and length is BUFLEN
  ldmaRXDescriptor = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_SINGLE_P2M_BYTE(&(USART0->RXDATA), inbuf, BUFLEN);

  ldmaRXDescriptor.xfer.blockSize = ldmaCtrlBlockSizeUnit1;
  
  ldmaRXConfig = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_USART0_RXDATAV);
}

void LDMA_IRQHandler()
{
  uint32_t flags = LDMA_IntGet();

  // Clear the transmit channel's done flag if set
  if (flags & (1 << TX_LDMA_CHANNEL)){
    LDMA_IntClear(1 << TX_LDMA_CHANNEL);
  }

  if (flags & (1 << RX_LDMA_CHANNEL)){
    LDMA_IntClear(1 << RX_LDMA_CHANNEL);
    rx_done = true;
  }
}

// Hàm đếm ký tự xuất hiện nhiều nhất
void find_max_char(uint8_t *buf, uint32_t len, char *result, uint32_t *count) {
  uint32_t freq[256] = {0};
  for (uint32_t i = 0; i < len; i++) {
    // Bỏ qua ký tự xuống dòng và ký tự rỗng
    if (buf[i] != '\r' && buf[i] != '\n' && buf[i] != 0) {
      freq[buf[i]]++;
    }
  }
  uint8_t max_char = 0;
  uint32_t max_count = 0;
  for (uint32_t i = 0; i < 256; i++) {
    if (freq[i] > max_count) {
      max_count = freq[i];
      max_char = (uint8_t)i;
    }
  }
  *result = (char)max_char;
  *count = max_count;
}

int main(void)
{
  sl_system_init();
  app_init();

#if defined(SL_CATALOG_KERNEL_PRESENT)
  sl_system_kernel_start();
#else // SL_CATALOG_KERNEL_PRESENT

  initGPIO();
  initUSART0();
  initLDMA();

  while (1) {
    rx_len = 0;
    while (1) {
        while (!(USART0->STATUS & USART_STATUS_RXDATAV));
        uint8_t c = USART_Rx(USART0);
        rx_buffer[rx_len++] = c;
        if (c == '\r' || c == '\n' || rx_len >= MAX_LEN) break;
    }

    // Xử lý dữ liệu như cũ
    char max_char;
    uint32_t max_count;
    find_max_char(rx_buffer, rx_len, &max_char, &max_count);

    char result_str[32];
    int n; // Khai báo n ở đây
    if (max_char >= 32 && max_char <= 126) {
      // Ký tự in được
      n = snprintf(result_str, sizeof(result_str), "%c: %lu\r\n", max_char, max_count);
    } else {
      // Ký tự không in được
      n = snprintf(result_str, sizeof(result_str), "0x%02X: %lu\r\n", (uint8_t)max_char, max_count);
    }

    for (int i = 0; i < n; i++) {
        while (!(USART0->STATUS & USART_STATUS_TXBL));
        USART_Tx(USART0, result_str[i]);
    }
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    sl_power_manager_sleep();
#endif
  }
#endif // SL_CATALOG_KERNEL_PRESENT
}
