`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 09/30/2019 12:06:16 PM
// Design Name: 
// Module Name: lab4_top
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
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
    
    microblaze_mcs_0 u1(
        .Clk(clk_100M),
        .Reset(reset),
        .UART_rxd(rx),
        .UART_txd(tx),
        .GPIO1_tri_i(displayswitch),
        .GPIO2_tri_i(ledswitch),
        .GPIO1_tri_o(pos_out),
        .GPIO2_tri_o(seg_out),
        .GPIO3_tri_o(LED),
        .GPIO4_tri_o(color_out)
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
        .reset(reset),
        .SEG(SEG), 
        .ANODE(ANODE)
    );
    assign posVertical = pos_out[3:0];
    assign posHorizontal = pos_out[8:4];
    assign withinConstraint =( vcount >= 32 * posVertical && vcount <= 32 * posVertical + 32) 
                            && (hcount >= 32 * posHorizontal && hcount <= 32 * posHorizontal + 32);
                    
    assign vgaRed = blank ? 4'b0000 : withinConstraint ? color_out[11:8] : 4'b0000;
    assign vgaBlue = blank ? 4'b0000 : withinConstraint ? color_out[7:4] : 4'b0000;
    assign vgaGreen = blank ? 4'b0000 : withinConstraint ? color_out[3:0] : 4'b0000;
    assign A = seg_out[3:0];
    assign B = seg_out[7:4];
    assign C = seg_out[11:8];
    assign D = seg_out[15:12];
    
endmodule
