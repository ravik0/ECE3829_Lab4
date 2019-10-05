/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: Controls position & color of block on VGA display, as well as LED and seven segment display on Basys 3.
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 *
 *
 *   Developers: Ravi Kirschner and Jonathan Lee
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xiomodule.h"


int main()
{
    init_platform();
    u32 data;
    XIOModule iomodule;

    //PART 1
    u32 horz = 9; //initial positions of block
    u32 vert = 7;
    u32 blockPos = 0;
    u16 moved = 0;
    u16 seg_out = 0;
    u8 rxbuf[10]; //buffer for characters being received from keyboard

    //PART 2
    u16 led_switched = 0; //variable to make sure holding button doesn't change LED.
    u8 LED = 1; //current value of LED being output.

    //EC
    u32 color = 15; //default color value, 15 being pure green
    u8 colorbuff[4]; //color buffer to hold input before being shifted into color.

    data = XIOModule_Initialize(&iomodule, XPAR_IOMODULE_0_DEVICE_ID);
    data = XIOModule_Start(&iomodule);

    XIOModule_DiscreteWrite(&iomodule, 4, color); //send initial color.

    while(1) {
    	//MOVING BLOCK
    	data = XIOModule_Recv(&iomodule, rxbuf, 1);
    	if(moved == 0) {
    		//if not already moved, then check and see if key is a value we want.
    		//if so change the position and set moved to 1.
			if(rxbuf[0] == 'u') {
				vert--;
				moved = 1;
			}
			else if (rxbuf[0] == 'd') {
				vert++;
				moved = 1;
			}
			else if (rxbuf[0] == 'l') {
				horz--;
				moved = 1;
			}
			else if (rxbuf[0] == 'r') {
				horz++;
				moved = 1;
			}
    	}
    	if(rxbuf[0] != 'u' && rxbuf[0] != 'd' && rxbuf[0] != 'l' && rxbuf[0] != 'r') moved = 0; //if no button pressed, can move again.
    	//moved is meant to debounce keyboard so holding doesn't move.

    	//wrap around
    	if(horz == 20) horz = 0;
    	else if(horz == -1) horz = 19;
    	if(vert == 15) vert = 0;
    	else if(vert == -1) vert = 14;

		//SEND BLOCK DATA
    	blockPos |= horz; //put horz in the first 5 bits of blockPos (should never exceed 19)
    	blockPos <<= 4; //move horz to the left by 4 bits, so first 4 bits are free
    	blockPos |= vert; //put vert in the first 4 bits. 9 bits now occupied
    	XIOModule_DiscreteWrite(&iomodule, 1, blockPos); //send 9 bits

		//COLOR
		if(rxbuf[0] == 'c') { //check key input. if c, then we're going to change color of block
			color = 0; //reset color

			xil_printf("Please enter a red value from 0000 to 1111\n\r");
			int index = 0;
			while(index < 4) { //get 4 bit color
				char c = inbyte(); //get value
				if(c == '0' || c == '1') { //if we have valid input, set colorbuff[index] and increment index
					colorbuff[index] = c;
					index++;
				}
				else { //otherwise yell at user
					xil_printf("Please input 0 or 1! You input: %c\n\r", c);
				}
			}
			xil_printf("Red value is %c%c%c%c\n\r", colorbuff[0],colorbuff[1],colorbuff[2],colorbuff[3]); //tell user what they input
			for(int i = 0; i < 4; i++) {
				int toShift = 0; //bit to shift in, as char vers of 1 is 49. if colorbuff[i] is 1, then shift 1. otherwise 0.
				if(colorbuff[i] == '1') toShift = 1;
				color |= toShift; //or with color
				color <<= 1; //move color to the left by 1 bit, make room.
			}

			xil_printf("Please enter a blue value from 0000 to 1111\n\r");
			index = 0;
			while(index < 4) {
				char c = inbyte();
				if(c == '0' || c == '1') {
					colorbuff[index] = c;
					index++;
				}
				else {
					xil_printf("Please input 0 or 1! You input: %c\n\r", c);
				}
			}
			xil_printf("Blue value is %c%c%c%c\n\r", colorbuff[0],colorbuff[1],colorbuff[2],colorbuff[3]);
			for(int i = 0; i < 4; i++) {
				int toShift = 0;
				if(colorbuff[i] == '1') toShift = 1;
				color |= toShift;
				color <<= 1;
			}

			xil_printf("Please enter a green value from 0000 to 1111\n\r");
			index = 0;
			while(index < 4) {
				char c = inbyte();
				if(c == '0' || c == '1') {
					colorbuff[index] = c;
					index++;
				}
				else {
					xil_printf("Please input 0 or 1! You input: %c\n\r", c);
				}
			}
			xil_printf("Green value is %c%c%c%c\n\r", colorbuff[0],colorbuff[1],colorbuff[2],colorbuff[3]);
			for(int i = 0; i < 4; i++) {
				int toShift = 0;
				if(colorbuff[i] == '1') toShift = 1;
				color |= toShift;
				if(i != 3) color <<= 1; //do not shift the last bit, as we will have all 12.
			}
			XIOModule_DiscreteWrite(&iomodule, 4, color); //send 12 bit color.
		}

    	//SEGMENT
        u16 D = horz >= 10 ? 1 : 0; //horizontal tens place
        u16 C = horz >= 10 ? horz-10 : horz; //horizontal ones place
        u16 B = vert >= 10 ? 1 : 0; //vertical tens place
        u16 A = vert >= 10 ? vert-10 : vert; //vertical ones place
    	seg_out |= D; //or in new values, then shift to make room for next ones
    	seg_out <<= 4;
    	seg_out |= C;
    	seg_out <<= 4;
    	seg_out |= B;
    	seg_out <<= 4;
    	seg_out |= A;
    	XIOModule_DiscreteWrite(&iomodule, 2, seg_out); //send 16 bit segment

    	//DISPLAY ON TERMINAL
    	data = XIOModule_DiscreteRead(&iomodule, 1); //read value of display button
    	if(data) { //if so, display our name and current position of block
    	    xil_printf("ECE3829 Design of a MicroBlaze MCS with VGA Text Display, Ravi Kirschner and Jonathan Lee\n\r");
    		xil_printf("Current position: X: %d, Y: %d\n\r", horz, vert);
    	}

    	//LED DRIVER
    	data = XIOModule_DiscreteRead(&iomodule, 2); //read value of led button
    	if(data && !led_switched) { //if button and we haven't switched LED value
    		LED <<= 1; //shift the 1 bit to the left
    		if(LED == 16) LED = 1; //if its at the 5th bit (1st is LSB), then reset to 1.
    		led_switched = 1;
    	}
    	if(!data) led_switched = 0;
		XIOModule_DiscreteWrite(&iomodule, 3, LED); //write LED value.

		//reset values for next run.
    	blockPos = 0;
    	rxbuf[0] = 0;
    	seg_out = 0;
    	color = 0;
    }


    cleanup_platform();
    return 0;
}
