`timescale 1ns / 1ps
// Engineer: Ravi Kirschner and Jonathan Lee
// 
// Create Date: 09/01/2019 11:55:04 AM
// Design Name: Seven Segment Decoder
// Module Name: seven_seg
// Project Name: DAC Waveform and Moving Block
// Target Devices: Basys 3
// Description: This module takes 4 4-bit inputs and cycles through the anodes at 800Hz, which is derived from the 25MHz from the Top File. Because of the way
// the seven seg board operates, SEG only contains one of the four 4-bit inputs decoded at a time. It is cycled through at 800Hz, similar to the anodes. The anodes
// and SEG change in tune with each other so a different number is displayed on each of the four seven segment displays on the board.
// 
//////////////////////////////////////////////////////////////////////////////////

module seven_seg(
    input clk, //25M clk
    input[3:0] A,
    input[3:0] B,
    input[3:0] C,
    input[3:0] D,
    input reset,
    output reg [6:0] SEG,
    output reg [3:0] ANODE
    );
    
    wire clk_en; //this is a pulse for the 800Hz clock. It does not need to be a square wave.
    reg [14:0] counter; //15 bit bus to hold up to 31250. 2^15 = 32768, 32768 > 31250.
    always @ (posedge clk, posedge reset)
        if(reset) counter <= 15'b0;
        else if(counter == 31250-1) counter <= 15'b0;
        else counter <= counter + 1'b1;
    assign clk_en = counter == 31250-1;
    
    reg [3:0] chosenSwitches; //current set of values to be decoded
    reg [1:0] SEL = 2'b00; //current selected anode
    
    always @(posedge clk)
        if(clk_en) SEL <= SEL + 1'b1; //if the 800Hz clk pings, increment SEL
    
    always @(posedge clk)   
        if(clk_en) begin
            case(SEL)
                0: chosenSwitches = A;
                1: chosenSwitches = B;
                2: chosenSwitches = C;
                3: chosenSwitches = D;
                default: chosenSwitches = 4'b0000;
            endcase //depending on SEL, change the set of values to be decoded
            case(SEL)
                0: ANODE = 4'b1110;
                1: ANODE = 4'b1101;
                2: ANODE = 4'b1011;
                3: ANODE = 4'b0111;
                default: ANODE = 4'b1111;
            endcase //depending on SEL, change the current anode that is being activated
        end
        
    parameter zero = 7'b0000001, 
        one = 7'b1001111, 
        two = 7'b0010010, 
        three = 7'b0000110, 
        four = 7'b1001100,
        five = 7'b0100100, 
        six = 7'b0100000, 
        seven = 7'b0001111, 
        eight = 7'b0000000, 
        nine = 7'b0000100, 
        As = 7'b0001000,
        Bs = 7'b1100000,
        Cs = 7'b0110001, 
        Ds = 7'b1000010, 
        Es = 7'b0110000,
        Fs = 7'b0111000; //Creating constants for each of the hex numbers 0-F
    
    always@(chosenSwitches)
        begin
            case(chosenSwitches) //depending on the number that is in the chosen set of numbers, we'll tell SEG which segments to turn on/off to display that number.
                0: SEG=zero;
                1: SEG=one;
                2: SEG=two;
                3: SEG=three;
                4: SEG=four;
                5: SEG=five;
                6: SEG=six;
                7: SEG=seven;
                8: SEG=eight;
                9: SEG=nine;
                10: SEG=As;
                11: SEG=Bs;
                12: SEG=Cs;
                13: SEG=Ds;
                14: SEG=Es;
                15: SEG=Fs;
                default: SEG = 7'b0;
            endcase
        end
endmodule