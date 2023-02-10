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
#define SECREAT_KEY_LOC 0x4C0
#define NOUNCE_EEPROM_LOC 0x3C0
#define UNLOCK_EEPROM_SIZE 64
#define TIMEOUT 100000

/*** Function definitions ***/
// Core functions - unlockCar and startCar
void unlockCar(void);
void startCar(void);

// Helper functions - sending ack messages
void sendAckSuccess(void);
void sendAckFailure(void);

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

  // Initialize UART peripheral
  uart_init();

  // Initialize board link UART
  setup_board_link();

  //generate secret from EEPROM and overwrite
  TCAesKeySched_t s=generate_encrypt_key(SECREAT_KEY_LOC)

  while (true) {
    unlockCar(s);
  }
}

/**
 * @brief Function that handles unlocking of car
 */
void unlockCar(TCAesKeySched_t s) {
  // Create a message struct variable for receiving data
  MESSAGE_PACKET message;
  uint8_t buffer[256];
  message.buffer = buffer;
  uint32_t nonce;
  EEPROMRead(&nonce, NOUNCE_EEPROM_LOC , 4); //last arg must be multip of 4

  // Receive packet with some error checking
  receive_board_message_by_type(&message, UNLOCK_SYN);

  //do decrption and extract the number check if its valid
  if(!decrypt_n_compare(message.buffer,s,nonce)){
    sendAckFailure(nonce); //do we have to keep those, the success one? Or just timeout? 
    return;
  };

  //send UNLOCK ACK and do counter ++, no need to write to eeprom yet, for time reason
  //some fuction need to be written here <here>

  receive_board_message_by_type(&message,UNLOCK_FIN,TIMEOUT);

  if(!decrypt_n_compare(message.buffer,s,nonce+2)){
    sendAckFailure(nonce);
    return;
  };

  // Pad payload to a string
  message.buffer[message.message_len] = 0;

  // If the data transfer is the password, unlock
  if (!strcmp((char *)(message.buffer), (char *)pass)) {
    
    nonce=nonce+3;
    sendAckSuccess(nonce);
    EEPROMProgram(&nonce, NOUNCE_EEPROM_LOC , 4); //last arg must be multip of 4
    //the fob needs to wait a bit before doing start car sending!!!
    startCar();


  } else {
    //we don't send the ACKFAIL in this case, can result in DDOS by unsync the two device. 
    EEPROMProgram(&nonce, NOUNCE_EEPROM_LOC , 4); //last arg must be multip of 4
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


  // Receive start message
  receive_board_message_by_type(&message, START_MAGIC);

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

/**
 * @brief Function to send successful ACK message
 */
void sendAckSuccess(uint32_t nonce;) { //this is a good final way to tell the fob to increase its counter
  // Create packet for successful ack and send
  // also needs to encrypt the buffer message. 

  MESSAGE_PACKET message;
  //encrypt nonce and send it off for future sync

  uint8_t buffer[32];
  message.buffer = buffer;
  message.magic = ACK_MAGIC;
  buffer[0] = ACK_SUCCESS;
  message.message_len = 1;

  send_board_message(&message);
}