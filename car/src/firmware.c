#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"

#include "driverlib/eeprom.h"
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

/*** Macro Definitions ***/
// Definit  ions for unlock message location in EEPROM
#define UNLOCK_EEPROM_LOC 0x7C0
#define UNLOCK_EEPROM_SIZE 64

/*** Function definitions ***/
// Core functions - unlockCar and startCar
void unlockCar(void);
void startCar(char* buffer);

// Declare password
const uint8_t car_id[] = CAR_ID;
bool reset_counter=false;

uint32_t nonce=5;

/**
 * @brief Main function for the car example
 *
 * Initializes the RF module and waits for a successful unlock attempt.
 * If successful prints out the unlock flag.
 */
int main(void) {
  reset:
  // Ensure EEPROM peripheral is enabled
  SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
  EEPROMInit();

  // Initialize UART peripheral
  uart_init();

  // Initialize board link UART
  setup_board_link();

  while (true) {
    char arr[8]; memset(arr,0,8);
    strncpy(arr,(char*) &nonce,4);
    regular_send(arr,NONCE_MAGIC);
    unlockCar();
    nonce++;
    if(!reset_counter){
      reset_counter=true;
      goto reset;
    }
  }
}



/**
 * @brief Function that handles unlocking of car
 */
void unlockCar() {
  //mask the interrupt for systick

  // Create a message struct variable for receiving data
  MESSAGE_PACKET message;
  uint8_t buffer[256];
  message.magic= 0;
  message.buffer = buffer;
  struct tc_aes_key_sched_struct s;
  memset(message.buffer,0,256);

  receive_board_message_by_type(&message, UNLOCK_MAGIC, TIMEOUT);
  generate_encrypt_key(&s, CAR_SECRET_LOC);
  if(decrypt_n_compare(buffer,&s,nonce)){
    memset(&s,0,sizeof(struct tc_aes_key_sched_struct));
    startCar((char*) buffer);
    memset(buffer,0,256);
  }
  else{
    memset(&s,0,sizeof(struct tc_aes_key_sched_struct));
    memset(buffer,0,256);
  }
}


/**
 * @brief Function that handles starting of car - feature list
 */
void startCar(char* buffer) {
  uint8_t feature_info = buffer[0];
  bool active[NUM_FEATURES]; memset(active,false,NUM_FEATURES);

  if(feature_info & 0x01){
    active[0]=true;
  }
  if(feature_info & 0x02){
    active[1]=true;
  }
  if(feature_info & 0x04){
    active[2]=true;
  }
  uint8_t unlock_message[64]; memset(unlock_message,0,64);
  EEPROMRead((uint32_t *)unlock_message, FEATURE_END, FEATURE_SIZE);
  uart_write(HOST_UART, unlock_message, 64);

  // Print out features for all active features
  for (int i = 0; i < NUM_FEATURES; i++) {
    if(active[i]){
      uint8_t eeprom_message[64];
      uint32_t offset = (i+1) * FEATURE_SIZE;
      EEPROMRead((uint32_t *)eeprom_message, FEATURE_END - offset, FEATURE_SIZE);
      uart_write(HOST_UART, eeprom_message, FEATURE_SIZE);
    }
  }
  reset_counter=false;
}