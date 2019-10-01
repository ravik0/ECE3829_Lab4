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
 * helloworld.c: simple test application
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
    u32 horz = 9;
    u32 vert = 7;
    u32 blockPos = 0;
    u16 moved = 0;
    u16 seg_out = 0;
    u8 rxbuf[10];

    //PART 2
    u16 led_switched = 0;
    u8 LED = 1;

    data = XIOModule_Initialize(&iomodule, XPAR_IOMODULE_0_DEVICE_ID);
    data = XIOModule_Start(&iomodule);

    while(1) {
    	//TODO: Debounce keys properly
    	//MOVING BLOCK
    	data = XIOModule_Recv(&iomodule, rxbuf, 1);
    	if(moved == 0) {
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
    	if(rxbuf[0] != 'u' && rxbuf[0] != 'd' && rxbuf[0] != 'l' && rxbuf[0] != 'r') moved = 0;
    	if(horz == 20) horz = 0;
    	else if(horz == -1) horz = 19;
    	if(vert == 15) vert = 0;
    	else if(vert == -1) vert = 14;
    	blockPos = blockPos | horz;
    	blockPos = blockPos << 4;
    	blockPos = blockPos | vert;
    	XIOModule_DiscreteWrite(&iomodule, 1, blockPos);
    	//SEGMENT
        u16 D = horz >= 10 ? 1 : 0;
        u16 C = horz >= 10 ? horz-10 : horz;
        u16 B = vert >= 10 ? 1 : 0;
        u16 A = vert >= 10 ? vert-10 : vert;
    	seg_out |= D;
    	seg_out <<= 4;
    	seg_out |= C;
    	seg_out <<= 4;
    	seg_out |= B;
    	seg_out <<= 4;
    	seg_out |= A;
    	XIOModule_DiscreteWrite(&iomodule, 2, seg_out);

    	//DISPLAY ON TERMINAL
    	data = XIOModule_DiscreteRead(&iomodule, 1);
    	if(data) {
    	    xil_printf("ECE3829 Design of a MicroBlaze MCS with VGA Text Display, Ravi Kirschner and Jonathan Lee\n\r");
    		xil_printf("Current position: X: %d, Y: %d\n\r", horz, vert);
    	}

    	//LED DRIVER
    	data = XIOModule_DiscreteRead(&iomodule, 2);
    	if(data && !led_switched) {
    		LED <<= 1;
    		if(LED == 16) LED = 1;
    		led_switched = 1;
    	}
    	if(!data) led_switched = 0;
		XIOModule_DiscreteWrite(&iomodule, 3, LED);

    	blockPos = 0;
    	rxbuf[0] = 0;
    	seg_out = 0;
    }


    cleanup_platform();
    return 0;
}
