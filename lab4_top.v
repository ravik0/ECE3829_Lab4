`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Engineer: Ravi Kirschner
// 
// Create Date: 09/30/2019 12:06:16 PM
// Design Name: Lab 4 Top File
// Module Name: lab4_top
// Project Name: Design of a MicroBlaze MCS with VGA Text Display
// Target Devices: Basys 3 & Microblaze MCS
// Description: This is the top module for Lab 4. It takes in the FPGA 100MHz clk, two seperate reset signals, two switches, and a UART input. It outputs
// VGA RGB, LED turnon values, UART output, hsync/vsync, and signals for the seven segment display. This module contains two MMCM clock outputs, one for
// the 100MHz clk going to the Microblaze, and the 25MHz going to the VGA controller. It also creates the Microblaze Microprocessor, which we control via
// software.
// 
//////////////////////////////////////////////////////////////////////////////////


module lab4_top(
    input clk,
    input reset,
    input vgareset,
    input ledswitch,
    input displayswitch,
    output tx,
    input rx,
    output [3:0] LED,
    output [3:0] vgaRed,
    output [3:0] vgaBlue,
    output [3:0] vgaGreen,
    output hSync,
    output vSync,
    output [6:0] SEG,
    output [3:0] ANODE
    );
    

    
    wire [10:0] hcount; //Horizontal/vertical count of pixel
    wire [10:0] vcount;
    wire blank;
    wire locked; //Locked signal to tell whether the MMCM is generating clock or not. Not currently used.
    wire clk_100M;
    wire clk_25M;
    wire [8:0] pos_out;
    wire [15:0] seg_out;
    wire [11:0] color_out;
    
    wire [3:0] posVertical;
    wire [4:0] posHorizontal;
    wire withinConstraint;
    
    wire [3:0] A, B, C, D;
    
    clk_wiz_0 mmcm_inst
    (
        // Clock out ports  
        .clk_100M(clk_100M),
        .clk_25M(clk_25M),
        // Status and control signals               
        .reset(vgareset), 
        .locked(locked),
        // Clock in ports
        .clk_in1(clk)
    );
    
    microblaze_mcs_0 u1( //the microblaze
        .Clk(clk_100M), 
        .Reset(reset), //seperate reset from the VGA and seven seg.
        .UART_rxd(rx), //UART input & output
        .UART_txd(tx),
        .GPIO1_tri_i(displayswitch), //GPIO Input 1: Display name/lab/position on terminal. 1 bit push-button input
        .GPIO2_tri_i(ledswitch), //GPIO Input 2: Switch LED value. 1 bit push-button input
        .GPIO1_tri_o(pos_out), //GPIO Output 1: Output current position, 9 bits. [8:4] is horizontal, [3:0] is vertical.
        .GPIO2_tri_o(seg_out), //GPIO Output 2: Outputs values for seven-seg to display, 16 bits. [15:12] is D value, [11:8] C, [7:4] B, [3:0] A.
        .GPIO3_tri_o(LED), //GPIO Output 3: Outputs values to turn on/off LEDs. 4 bits. Only one bit will ever be 1.
        .GPIO4_tri_o(color_out) //GPIO Output 4: Outputs value for RGB of block, 12 bits. [11:8] is red, [7:4] blue, and [3:0] green.
        );
        
        
    vga_controller_640_60 u2(
        .rst(vgareset), 
        .pixel_clk(clk_25M), 
        .HS(hSync), 
        .VS(vSync), 
        .hcount(hcount), 
        .vcount(vcount), 
        .blank(blank)
    );
    
    seven_seg u3(
        .clk(clk_25M), 
        .A(A), 
        .B(B), 
        .C(C), 
        .D(D), 
        .reset(vgareset),
        .SEG(SEG), 
        .ANODE(ANODE)
    );
    
    assign posVertical = pos_out[3:0]; //horizontal and vertical values to calculate withinConstraint
    assign posHorizontal = pos_out[8:4];
    assign withinConstraint =( vcount >= 32 * posVertical && vcount <= 32 * posVertical + 32) 
                            && (hcount >= 32 * posHorizontal && hcount <= 32 * posHorizontal + 32);
                            //withinConstraint tells the monitor whether to draw the block in that pixel or not.
      
    //Block being assigned RGB values. Default value is color_out = 12'b000000001111, so green default.              
    assign vgaRed = blank ? 4'b0000 : withinConstraint ? color_out[11:8] : 4'b0000;
    assign vgaBlue = blank ? 4'b0000 : withinConstraint ? color_out[7:4] : 4'b0000;
    assign vgaGreen = blank ? 4'b0000 : withinConstraint ? color_out[3:0] : 4'b0000;
   
    //Segments being assigned.
    assign A = seg_out[3:0]; //rightmost value
    assign B = seg_out[7:4];
    assign C = seg_out[11:8];
    assign D = seg_out[15:12]; //leftmost value
    
endmodule