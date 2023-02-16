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
#define SECRET_KEY_LOC 0x4C0
#define NOUNCE1_EEPROM_LOC 0x3C0
#define NOUNCE2_EEPROM_LOC 0x2C0
#define UNLOCK_EEPROM_SIZE 64
#define TIMEOUT 100000

/*** Function definitions ***/
// Core functions - unlockCar and startCar
void unlockCar(void);
void startCar(void);

// Helper functions - sending ack messages
void sendAckSuccess(void);
void sendAckFailure(void);

// Declare password (Hey Kush can you figure out where these two things come into play?)
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

  // Initialize UART peripheral
  uart_init();

  // Initialize board link UART
  setup_board_link();

  while (true) {
    unlockCar();
  }
}

/**
 * @brief Function that handles unlocking of car
 */
void unlockCar() {
  // Create a message struct variable for receiving data
  MESSAGE_PACKET message;
  uint8_t buffer[256];
  uint32_t nonce;
  bool car;
  message.buffer = buffer;
  TCAesKeySched_t s=generate_encrypt_key(SECRET_KEY_LOC);
  memset(message.buffer,0,256);

  // Receive packet with some error checking
  receive_board_message_by_type(&message, UNLOCK_SYN,-1); // Conversation started by the pair fob
  if(message.dev==0){
    nonce=EEPROMRead((uint32_t *) nonce, NOUNCE1_EEPROM_LOC , 4);
    car=0;
  }
  else{
    nonce=EEPROMRead((uint32_t *) nonce, NOUNCE2_EEPROM_LOC , 4);
    car=1;
  }

  //do decrption and extract the number check if its valid
  if(!decrypt_n_compare(message.buffer,s,SECRET_KEY_LOC,nonce)){ // compare the nonce from the fob
    return;
  };
  memset(message.buffer,0,256);

  //send UNLOCK ACK and do counter ++, no need to write to eeprom yet, for time reason
  encrypt_n_send(SECRET_KEY_LOC, s, nonce+1, UNLOCK_ACK); // Send the next nonce in the series to the fob
  memset(message.buffer,0,256);

  //receive the final message for unlock
  receive_board_message_by_type(&message,UNLOCK_FIN,TIMEOUT); // Receive the unlock car command + nonce 

  if(!decrypt_n_compare(message.buffer,s,SECRET_KEY_LOC,nonce+2)){
    encrypt_n_send(SECRET_KEY_LOC, s, nonce, ACK_FAIL);
    memset(message.buffer,0,256);
  }
  else{//it works unlock
    nonce=nonce+3;
    encrypt_n_send(SECRET_KEY_LOC s, nonce, ACK_SUCCESS); //fob updates the nonce and send start car magic
    if(car==0){
      EEPROMProgram(&nonce, NOUNCE1_EEPROM_LOC , 4); //last arg must be multip of 4
    }
    else{
      EEPROMProgram(&nonce, NOUNCE2_EEPROM_LOC , 4); //last arg must be multip of 4
    }
    memset(message.buffer,0,256);
    startCar();
  }
}


/**
 * @brief Function that handles starting of car - feature list
 */
void startCar(TCAesKeySched_t s) {
  // Create a message struct variable for receiving data
  MESSAGE_PACKET message;
  uint8_t buffer[256];
  message.buffer = buffer;


  // Receive start message but with a time out!!
  receive_board_message_by_type(&message, START_MAGIC,TIMEOUT);

  //decrypt the message buffer and compare everything (use their pre-made compare procedures)
  tc_aes_decrypt(buffer,buffer,s);

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