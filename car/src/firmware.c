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

/*** Structure definitions ***/
// Structure of start_car packet FEATURE_DATA
typedef struct {
  uint8_t car_id[8];
  uint8_t num_active;
  uint8_t features[NUM_FEATURES];
} FEATURE_DATA;

/*** Macro Definitions ***/
// Definitions for unlock message location in EEPROM
#define UNLOCK_EEPROM_LOC 0x7C0
#define UNLOCK_EEPROM_SIZE 64

/*** Function definitions ***/
// Core functions - unlockCar and startCar
void unlockCar(void);
void startCar(struct tc_aes_key_sched_struct* s);

// Declare password
const uint8_t pass[] = PASSWORD;
const uint8_t car_id[] = CAR_ID;

/**
 * @brief Main function for the car example
 *
 * Initializes the RF module and waits for a successful unlock attempt.
 * If successful prints out the unlock flag.
 */
int main(void) {
  // Ensure EEPROM peripheral is enabled
  SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
  EEPROMInit();
  uint32_t nonce;

  // Initialize UART peripheral
  uart_init();

  // Initialize board link UART
  setup_board_link();

  while (true) {
    uint8_t *arr[8]; memset(arr,0,8);
    strncpy(arr,(uint8_t*) &nonce,4);
    regular_send(char* arr,uint8_t type);
    nonce++; //this is just a stub for debugging
    unlockCar();
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
  message.buffer = buffer;
  struct tc_aes_key_sched_struct s;
  memset(message.buffer,0,256);

  receive_board_message_by_type(&message, UNLOCK, TIMEOUT);
  generate_encrypt_key(&s, CAR_SECRET_LOC);
  if(decrypt_n_compare(buffer,&s,CAR_UNLOCK_ID)){
    memset(&s,0,sizeof(struct tc_aes_key_sched_struct));
    startCar(buffer+20);
  }
  else{
    memset(&s,0,sizeof(struct tc_aes_key_sched_struct));
    memset(buffer,0,256);

  }

  //at this point:
  // 1. the buffer is fully decrypted
  // 2. the key is no longer active and memory cleared
  // 3. if decrypt_n_compare works, (first 20 bytes are unlock info, rest are feature info)
  // 4. if decrypt_n_compare not work, clear the whole buffer, go back to while loop
}


/**
 * @brief Function that handles starting of car - feature list
 */
void startCar(char* buffer) {

  FEATURE_DATA *feature_info = (FEATURE_DATA *)buffer;

  // Verify correct car id
  if (strcmp((char *)car_id, (char *)feature_info->car_id)) {
    return;
  }

  // Print out features for all active features
  for (int i = 0; i < feature_info->num_active; i++) {
    uint8_t eeprom_message[64];

    uint32_t offset = feature_info->features[i] * FEATURE_SIZE;

    EEPROMRead((uint32_t *)eeprom_message, FEATURE_END - offset, FEATURE_SIZE);

    uart_write(HOST_UART, eeprom_message, FEATURE_SIZE);
  }
}