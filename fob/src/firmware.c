#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"

#include "driverlib/eeprom.h"
#include "driverlib/flash.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "secrets.h"

#include "board_link.h"
#include "feature_list.h"
#include "uart.h"
#include "aes.h"

#define FOB_STATE_PTR 0x3FC00
#define FLASH_DATA_SIZE         \
  (sizeof(FLASH_DATA) % 4 == 0) \
      ? sizeof(FLASH_DATA)      \
      : sizeof(FLASH_DATA) + (4 - (sizeof(FLASH_DATA) % 4))
#define FLASH_PAIRED 0x01
#define FLASH_UNPAIRED 0xFF

/*** Macro Definitions ***/
// Definitions for unlock message location in EEPROM
#define UNLOCK_EEPROM_LOC 0x7C0
#define UNLOCK_EEPROM_SIZE 64

/*** Structure definitions ***/
// Defines a struct for the format of an enable message
typedef struct
{
  uint8_t car_id[8];
  uint8_t feature;
} ENABLE_PACKET;

// Defines a struct for storing the state in flash
typedef struct
{
  uint8_t car_id[8];
  uint8_t paired;
  uint8_t active_features; //form of 00000001 00000010 00000100
} FLASH_DATA;

/*** Function definitions ***/
// Core functions - all functionality supported by fob
void saveFobState(FLASH_DATA *flash_data);
void pairFob(FLASH_DATA *fob_state_ram);
void unlockCar(FLASH_DATA *fob_state_ram);
void enableFeature(FLASH_DATA *fob_state_ram);
void startCar(FLASH_DATA *fob_state_ram);

/**
 * @brief Main function for the fob example
 *
 * Listens over UART and SW1 for an unlock command. If unlock command presented,
 * attempts to unlock door. Listens over UART for pair command. If pair
 * command presented, attempts to either pair a new key, or be paired
 * based on firmware build.
 */
int main(void)
{
  FLASH_DATA fob_state_ram;
  memset(&fob_state_ram, 0, FLASH_DATA_SIZE);
  FLASH_DATA *fob_state_flash = (FLASH_DATA *)FOB_STATE_PTR; //this is stored in flash some information

// If paired fob, initialize the system information
#if PAIRED == 1
  if (fob_state_flash->paired == FLASH_UNPAIRED)
  {
    strncpy(fob_state_ram.car_id,CAR_ID,6);
    fob_state_ram.paired = FLASH_PAIRED;
    saveFobState(&fob_state_ram);
  }
#endif

  if (fob_state_flash->paired == FLASH_PAIRED)
  {
    memcpy(&fob_state_ram, fob_state_flash, FLASH_DATA_SIZE);
  }

  // This will run on first boot to initialize features
  if (fob_state_ram.active_features == 0xFF)
  {
    fob_state_ram.active_features = 0;
    saveFobState(&fob_state_ram);
  }

  // Initialize UART
  uart_init();

  // Initialize EEPROM
  SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
  EEPROMInit();

  // Initialize board link UART
  setup_board_link();

  // Setup SW1
  GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
  GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_4MA,GPIO_PIN_TYPE_STD_WPU);

  // Declare a buffer for reading and writing to UART
  uint8_t uart_buffer[10];
  uint8_t uart_buffer_index = 0;

  uint8_t previous_sw_state = GPIO_PIN_4;
  uint8_t debounce_sw_state = GPIO_PIN_4;
  uint8_t current_sw_state = GPIO_PIN_4;

  // Infinite loop for polling UART
  while (true)
  {

    // Non blocking UART polling
    if (uart_avail(HOST_UART))
    {
      uint8_t uart_char = (uint8_t)uart_readb(HOST_UART);

      if ((uart_char != '\r') && (uart_char != '\n') && (uart_char != '\0') &&
          (uart_char != 0xD))
      {
        uart_buffer[uart_buffer_index] = uart_char;
        uart_buffer_index++;
      }
      else
      {
        uart_buffer[uart_buffer_index] = 0x00;
        uart_buffer_index = 0;

        if (!(strcmp((char *)uart_buffer, "enable")))
        {
          enableFeature(&fob_state_ram);
        }
        else if (!(strncmp((char *)uart_buffer, "pair",4)))
        {
          if(fob_state_ram.paired == 1){
            char pin[10]; memset(pin,0,10);
            EEPROMRead((uint32_t *)pin, PIN , 8);
            strncpy(pin,pin,6);
            strncpy(uart_buffer,uart_buffer+4,6);
            if(!strncmp(pin,(char *)uart_buffer,6)){
              pairFob(&fob_state_ram);
            }
          }
          else{
            pairFob(&fob_state_ram);
          }
        }
      }
    }

    current_sw_state = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4);
    if ((current_sw_state != previous_sw_state) && (current_sw_state == 0))
    {
      // Debounce switch
      for (int i = 0; i < 10000; i++)
        ;
      debounce_sw_state = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4);
      if (debounce_sw_state == current_sw_state)
      {
        unlockCar(&fob_state_ram);
      }
    }
    previous_sw_state = current_sw_state;
  }
}

/**
 * @brief Function that carries out pairing of the fob
 *
 * @param fob_state_ram pointer to the current fob state in ram
 */
void pairFob(FLASH_DATA *fob_state_ram) //if it is paried and what features are active 
{
  MESSAGE_PACKET message;
  char buffer[256];
  if (fob_state_ram->paired == FLASH_PAIRED)
  {
    struct tc_aes_key_sched_struct s;
    generate_encrypt_key(&s, HOST_FOB_SECT);
    message.buffer=buffer;
    message.magic=PAIR_MAGIC;
    memset(message.buffer,0,256);
    EEPROMRead((uint32_t *)message.buffer, AES_SECRET_LOC , 16);
    char* temp=message.buffer+16;
    EEPROMRead((uint32_t *)temp, PIN , 8);
    temp=temp+6;
    temp[0] = fob_state_ram->active_features;
    temp=temp+1;
    strncpy(temp,fob_state_ram->car_id,6);
    temp=temp+6;
    temp[0] = '\0';
    tc_aes_encrypt(message.buffer, message.buffer, &s);
    message.message_len=strlen((const char*) message.buffer)+strlen((const char*) (message.buffer+(strlen((const char*) message.buffer)+1)))+1;
    for(int i=0;i<10000;i++){
      __asm(" NOP\n");
    }
    send_board_message(&message);
    memset(&s,0,sizeof(struct tc_aes_key_sched_struct)); 
    memset(message.buffer,0,256);
  }
  else
  {
    struct tc_aes_key_sched_struct s;
    generate_encrypt_key(&s, HOST_FOB_SECT); 
    message.buffer=buffer;
    memset(message.buffer,0,256);
    receive_board_message_by_type(&message, PAIR_MAGIC,-1);
    tc_aes_decrypt(message.buffer, message.buffer, &s);
    EEPROMProgram((uint32_t *)message.buffer, AES_SECRET_LOC, 16);
    EEPROMProgram((uint32_t *)message.buffer+16, PIN, 8);
    fob_state_ram->active_features = message.buffer[22];
    strncpy(fob_state_ram->car_id, message.buffer+23,6);
    fob_state_ram->paired = FLASH_PAIRED;
    uart_write(HOST_UART, (uint8_t *)"Paired", 6);
    memset(&s,0,sizeof(struct tc_aes_key_sched_struct)); 
    memset(message.buffer,0,256); 
    saveFobState(fob_state_ram);
  }
}


/**
 * @brief Function that handles enabling a new feature on the fob
 *
 * @param fob_state_ram pointer to the current fob state in ram
 */
void enableFeature(FLASH_DATA *fob_state_ram)
{
  if (fob_state_ram->paired == FLASH_PAIRED)
  {
    uint8_t buffer[64];
    memset(buffer,0,64);
    for (int i = 0; i < 16; i++) {
      buffer[i] = (uint8_t)UARTCharGet(HOST_UART);
    }

    struct tc_aes_key_sched_struct s;
    generate_encrypt_key(&s, AES_SECRET_LOC);
    tc_aes_decrypt(buffer, buffer, &s);
    if(strncmp(fob_state_ram->car_id,buffer+1,6)!=0){
      return;
    }
    fob_state_ram->active_features=buffer[0];

    saveFobState(fob_state_ram);
    uart_write(HOST_UART, (uint8_t *)"Enabled", 7);
  }
}

/**
 * @brief Function that handles the fob unlocking a car
 *
 * @param fob_state_ram pointer to the current fob state in ram
 */
void unlockCar(FLASH_DATA *fob_state_ram)
{
  if (fob_state_ram->paired == FLASH_PAIRED)
  {
    uint8_t feature_info = fob_state_ram->active_features;
    // Create a message struct variable for receiving data
    MESSAGE_PACKET message;
    char buffer[8];
    message.magic= 0;
    message.buffer = buffer;
    struct tc_aes_key_sched_struct s;
    memset(message.buffer, 0, 8);

    receive_board_message_by_type(&message, NONCE_MAGIC, -1);
    if (message.message_len != 0){
      generate_encrypt_key(&s, AES_SECRET_LOC);
      encrypt_n_send(&s, &message, feature_info, UNLOCK_MAGIC);
    }
    memset(&s,0,sizeof(struct tc_aes_key_sched_struct));
    memset(message.buffer, 0, 8);
  }
}


/**
 * @brief Function that erases and rewrites the non-volatile data to flash
 *
 * @param info Pointer to the flash data ram
 */
void saveFobState(FLASH_DATA *flash_data)
{
  FlashErase(FOB_STATE_PTR);
  FlashProgram((uint32_t *)flash_data, FOB_STATE_PTR, FLASH_DATA_SIZE);
}
