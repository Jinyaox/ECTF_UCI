#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/eeprom.h"
#include "utils/uartstdio.h"
 
 
#define E2PROM_TEST_ADRES 0x0000 
 
struct E2PROM
{
	uint8_t value1;
	uint8_t value2;
	uint16_t value3;
	uint8_t value4[12];
}; 
 
 
 
struct E2PROM e2prom_write_value = {5,7,9826, "Hello World"}; /* Write struct */
struct E2PROM e2prom_read_value =  {0,0,0,""}; /* Read struct */
 
 
int main(void) 
{
	uint32_t e2size,e2block;
 
	SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ); // islemcimizi 80 Mhz'e ayarlıyoruz.
 
	/* UART SETTINGS */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UARTStdioConfig(GPIO_PORTA_BASE,115200,SysCtlClockGet());
	/*******************************/
 
	/* EEPROM SETTINGS */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0); // EEPROM activate
	EEPROMInit(); // EEPROM start
	/*******************************/
 
	UARTprintf("EEPROM Test Program ,, Teknikyazi.com\r\n");
 
 
	e2size = EEPROMSizeGet(); // Get EEPROM Size 
	UARTprintf("EEPROM Size %d bytes\n", e2size);
 
	e2block = EEPROMBlockCountGet(); // Get EEPROM Block Count
	UARTprintf("EEPROM Blok Count: %d\n", e2block);
 
	UARTprintf("Write Try > Address %u: Struct : {%u,%u,%u,%s}\n", E2PROM_TEST_ADRES, e2prom_write_value.value1, e2prom_write_value.value2, e2prom_write_value.value3, e2prom_write_value.value4);
	EEPROMProgram((uint32_t *)&e2prom_write_value, E2PROM_TEST_ADRES, sizeof(e2prom_write_value)); //Write struct to EEPROM start from 0x0000
 
	EEPROMRead((uint32_t *)&e2prom_read_value, E2PROM_TEST_ADRES, sizeof(e2prom_read_value)); //Read from struct at EEPROM start from 0x0000
	UARTprintf("Read Try > Address %u: Struct : {%u,%u,%u,%s}\n", E2PROM_TEST_ADRES, e2prom_read_value.value1, e2prom_read_value.value2, e2prom_read_value.value3, e2prom_read_value.value4);
 
while(1)
 {
 }
 
}