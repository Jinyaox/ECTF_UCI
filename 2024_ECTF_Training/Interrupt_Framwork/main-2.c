/******************************************************************************
 * Copyright (C) 2023 Maxim Integrated Products, Inc., All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated
 * Products, Inc. shall not be used except as stated in the Maxim Integrated
 * Products, Inc. Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products, Inc. retains all
 * ownership rights.
 *
 ******************************************************************************/

/**
 * @file    main.c
 * @brief   UART!
 * @details This example demonstrates the UART Loopback Test.
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mxc_device.h"
#include "mxc_delay.h"
#include "nvic_table.h"
#include "board.h"
#include "gpio.h"

void GPIO0_handler()
{
    MXC_GPIO_ClearFlags(MXC_GPIO0,0xFFFFFFFF);//let's just do this for now
}

/******************************************************************************/
int main(void){

    mxc_gpio_cfg_t gpio_out;
    mxc_gpio_cfg_t gpio_in;
    //a simple GPIO example

    gpio_out.port = MXC_GPIO1; // int the MAX 78000.h file
    gpio_out.mask = MXC_GPIO_PIN_6; //in the gpio.h file
    gpio_out.pad = MXC_GPIO_PAD_NONE; //determine pull up or down
    gpio_out.func = MXC_GPIO_FUNC_OUT; // in or out or whatever the fuck it is
    gpio_out.vssel = MXC_GPIO_VSSEL_VDDIO; //3,3V
    gpio_out.drvstr = MXC_GPIO_DRVSTR_1; //what is that??
    MXC_GPIO_Config(&gpio_out);

    //setup the GPIO pin for interrupt handling

    gpio_in.port = MXC_GPIO0; // int the MAX 78000.h file
    gpio_in.mask = MXC_GPIO_PIN_19; //in the gpio.h file now for GPIO pin 2-2
    gpio_in.pad = MXC_GPIO_PAD_PULL_DOWN; //determine pull up or down
    gpio_in.func = MXC_GPIO_FUNC_IN; // in or out or whatever the fuck it is
    gpio_in.vssel = MXC_GPIO_VSSEL_VDDIO; //3,3V
    gpio_in.drvstr = MXC_GPIO_DRVSTR_1; //what is that??
    MXC_GPIO_Config(&gpio_in);
    MXC_GPIO_IntConfig(&gpio_in,MXC_GPIO_INT_RISING); //configure this to the rising edge
    MXC_NVIC_SetVector(GPIO0_IRQn, GPIO0_handler);
    NVIC_EnableIRQ(GPIO0_IRQn);
    MXC_GPIO_EnableInt(MXC_GPIO0,MXC_GPIO_PIN_19);


    while(1){
        //MXC_GPIO_OutSet(gpio_out.port, gpio_out.mask);
        // MXC_Delay(1000);
        // MXC_GPIO_OutClr(gpio_out.port, gpio_out.mask);
    }
}
