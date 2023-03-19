/**
 * @file board_link.h
 * @author Frederich Stine
 * @brief Firmware UART interface implementation.
 * @date 2023
 *
 * This source file is part of an example system for MITRE's 2023 Embedded
 * System CTF (eCTF). This code is being provided only for educational purposes
 * for the 2023 MITRE eCTF competition, and may not meet MITRE standards for
 * quality. Use this code at your own risk!
 *
 * @copyright Copyright (c) 2023 The MITRE Corporation
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_uart.h"

#include "driverlib/eeprom.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "board_link.h"
#include "feature_list.h"

//Encryption library including
#include "aes.h"

/**
 * @brief Set the up board link object
 *
 * UART 1 is used to communicate between boards
 */
void setup_board_link(void) {
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1); //this is just clock gating
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

  GPIOPinConfigure(GPIO_PB0_U1RX);
  GPIOPinConfigure(GPIO_PB1_U1TX);

  GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  // Configure the UART for 115,200, 8-N-1 operation.
  UARTConfigSetExpClk(
      BOARD_UART, SysCtlClockGet(), 115200,
      (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

  while (UARTCharsAvail(BOARD_UART)) {
    UARTCharGet(BOARD_UART);
  }
}

/**
 * @brief Send a message between boards
 *
 * @param message pointer to message to send
 * @return uint32_t the number of bytes sent
 */
uint32_t send_board_message(MESSAGE_PACKET *message) {
  UARTCharPut(BOARD_UART, message->magic);
  UARTCharPut(BOARD_UART, message->message_len);

  for (int i = 0; i < message->message_len; i++) {
    UARTCharPut(BOARD_UART, message->buffer[i]);
  }

  return message->message_len;
}




/**
 * @brief Receive a message between boards
 *
 * @param message pointer to message where data will be received
 * @return uint32_t the number of bytes received - 0 for error
 */
uint32_t receive_board_message(MESSAGE_PACKET *message, uint8_t type) {

  if(uart_avail(BOARD_UART)){
    message->magic = (uint8_t)UARTCharGet(BOARD_UART);
    message->message_len = (uint8_t)UARTCharGet(BOARD_UART);
    for (int i = 0; i < message->message_len; i++) {
      message->buffer[i] = (uint8_t)UARTCharGet(BOARD_UART);
    }
  }

  if (message->magic != type) {
    return 0;
  }

  return message->message_len;
}




/**
 * @brief Function that retreives messages until the specified message is found
 *
 * @param message pointer to message where data will be received
 * @param type the type of message to receive
 * @return uint32_t the number of bytes received
 */
uint32_t receive_board_message_by_type(MESSAGE_PACKET *message, uint8_t type, int timeout) {
  if(timeout<0){
    do {
      receive_board_message(message,type);
    } while (message->magic != type);
  }
  else{
    do {
      receive_board_message(message, type);
      timeout--;
    } while ((message->magic != type)&&(timeout!=0));
    if(message->magic != type){
      message->message_len=0;//basically saying not okay 
    }
  }
  return message->message_len;
}

void generate_encrypt_key(struct tc_aes_key_sched_struct* s, uint32_t secret_loc){
  //NUM_OF_NIST_KEYS 16 extracting 16 bytes from the secret location
  uint8_t nist_key[16];
  EEPROMRead((uint32_t *) nist_key, secret_loc , 16); //now we get the key, maybe
  tc_aes128_set_encrypt_key(s, nist_key);
}

void encrypt_n_send(struct tc_aes_key_sched_struct *s, MESSAGE_PACKET *message, uint8_t feature, uint8_t type)
{
  message->magic = type;
  message->buffer[strlen(message->buffer)]=feature;
  tc_aes_encrypt(message->buffer, message->buffer, s);
  message->message_len=strlen((const char*) message->buffer);
  send_board_message(message);
}