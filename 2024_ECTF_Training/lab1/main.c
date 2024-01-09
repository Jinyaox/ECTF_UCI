#include "tm4c123gh6pm.h"
#include "string.h"
#include "stdint.h"
#include "stdbool.h"
#include "driverlib/flash.h"

//flash memory starts at 0 to 0x0003FFFF



void GPIOA_SETUP(){
  SYSCTL_RCGCGPIO_R|=SYSCTL_RCGCGPIO_R0; //we enable clock gating for A
  while(!((SYSCTL_RCGCGPIO_R & SYSCTL_RCGCGPIO_R0)==SYSCTL_RCGCGPIO_R0)); //wait for the time to stable
  UART0_CTL_R &=(~UART_CTL_UARTEN);
  GPIO_PORTA_DEN_R=0x03U;
  GPIO_PORTA_AFSEL_R=0x03U; //this is the alternative function control function.
  GPIO_PORTA_PCTL_R=0x11; //configure the pins as the UART

  //end of GPIOA configuration
}

void UART_SETUP(){
  SYSCTL_RCGCUART_R|=0x01; //enable the first UART clock gating so we can use uart 0, this gets to communicate with the PC.
  while(!((SYSCTL_RCGCUART_R & SYSCTL_RCGCUART_R0)==SYSCTL_RCGCUART_R0)); //wait for UART to setup
  GPIOA_SETUP();
  UART0_CTL_R &=(~UART_CTL_UARTEN);

  //set baud rate
  //UART Baud Rate =  ( f / 16 x baud divisor) freq is 16MHZ
  //Baud divisor = 1000000/9600 = 104.1667
  // 0.166 x 64 + 0.5 = 11 so this is the fraction we need
  UART0_IBRD_R=104;
  UART0_FBRD_R=11;

  //set bit, parity, stopbit, ~FIFO
  UART0_CC_R=0; //use default clock the system's clock
  UART0_LCRH_R=0x60; // we set 8 bits, no parity and 1 stop bit, also no FIFO (don't worry about that)
  UART0_CTL_R|=(UART_CTL_RXE|UART_CTL_TXE|UART_CTL_UARTEN);
}

/*
 * set up the buttons and its respective interrupts
 */
void button_setup(){

    //set up the GPIOF button and make it configurable
    SYSCTL_RCGCGPIO_R |=(1<<5);
    GPIO_PORTF_LOCK_R =0x4C4F434B;
    GPIO_PORTF_CR_R =0x01;
    GPIO_PORTF_LOCK_R = 0;


    //regular setup routine for buttons
    GPIO_PORTF_DIR_R &= ~(1<<4)|~(1<<0);
    GPIO_PORTF_DEN_R |= (1<<4)|(1<<0);

    //enable pull up for interrupt detection
    GPIO_PORTF_PUR_R |= (1<<4)|(1<<0);

    //setup interrupt
    GPIO_PORTF_IS_R &= ~(1<<4)|~(1<<0);        /* make bit 4, 0 edge sensitive */
    GPIO_PORTF_IBE_R &=~(1<<4)|~(1<<0);         /* trigger is controlled by IEV */
    GPIO_PORTF_IEV_R &= ~(1<<4)|~(1<<0);        /* falling edge trigger */
    GPIO_PORTF_ICR_R |= (1<<4)|(1<<0);          /* clear any prior interrupt */
    GPIO_PORTF_IM_R |= (1<<4)|(1<<0);          /* unmask interrupt */

    //setup the interrupt handler
    NVIC_EN0_R |=(1<<30); //enable IRQ30

}


//char UART_RX(void)  {
//  char data;
//  while((UART0_FR_R & (1<<4)) != 0); /* wait until Rx buffer is not full */
//  data = UART0_DR_R ;  /* before giving it another byte */
//  return (unsigned char) data;
//}



int main(void)
{
    UART_SETUP();
    //We are setting up interrupts here
    UART0_IM_R  = 0x0010; //make bit 4 RX active for receiving
    UART0_ICR_R &= ~(0x010); //clear any existing interrupt
    NVIC_EN0_R |= 0x0020;//enable UART 0 handler now we are done


    button_setup();



    //we want to setup TIVA wire flash write? Maybe

    while(1){
        ;
        //clearly a stub, but you can play around with it
    }
	return 0;
}
