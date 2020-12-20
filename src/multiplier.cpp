#include <verilog.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <string> 
#include <stdexcept>
#include <stdio.h>
#include <bits/stdc++.h> 

using namespace std;

/*------------------------------------------------------------------------------------------
                            WALLACE TREE MULTIPLIER WITH PIPELINING
                            =======================================
------------------------------------------------------------------------------------------*/


WALLACE_TREE_MULTIPLIER_PIPELINED::WALLACE_TREE_MULTIPLIER_PIPELINED(string name,
                                                                    vector<wire> input_wires,
                                                                    wire output_wire,
                                                                    int n_bits,
                                                                    int pipeline_k){

    /*------------------------------------------------------------------------------------------
    First we do the basic setups and input/output declaration.
    Inputs:
        A : n_bit input number
        B : n_bit input number
        CLK : 1 bit clock
        P : 2 x n_bit output product
    ------------------------------------------------------------------------------------------*/
    this->name = name;
    this->inputs = input_wires;
    this->outputs.push_back(output_wire);
    this->n_bits = n_bits;

    this->declare("A", CHIP_INPUTS);
    this->declare("B", CHIP_INPUTS);
    this->declare("CLK", CHIP_INPUTS, 1);
    this->declare("P", CHIP_OUTPUTS, 2*n_bits);

    
    /*------------------------------------------------------------------------------------------
    Now we compute the first set of partial products. Partial products are stored in a vector of
    type cutwire

    For convinience during wiring, we declared a struct WALLACE_TREE_MULTIPLIER_PIPELINED::CutWire
    containing information about shift and wire length.

            typedef struct cut_wire{
                wire wire_id;
                int shift=0;
                int length=0;
            } CutWire;

    ------------------------------------------------------------------------------------------*/
    vector<WALLACE_TREE_MULTIPLIER_PIPELINED::CutWire> partial_products;

    for(int n=0; n<n_bits; n++){
        /*------------------------------------------------------------------------------------------
        For each bit compute the initial partial products and append it to the partial products 
        vector.
        ------------------------------------------------------------------------------------------*/
        wire partial_product = "WTM_PP_0_" + to_string(n);
        this->add_wire("[" + to_string(n_bits - 1) + ":0] " + partial_product);
        for(int i=0; i <n_bits; i++){
            AND curr_and("WTM_PP_AND_" + to_string(n) + "_" + to_string(i),
                        {"A[" + to_string(i) + "]", "B[" + to_string(n) + "]"},
                        partial_product + "[" + to_string(i) + "]");
            this->add_submodule(curr_and);
        }

        WALLACE_TREE_MULTIPLIER_PIPELINED::CutWire current_cutwire;
        current_cutwire.wire_id = partial_product;
        current_cutwire.shift = n;
        current_cutwire.length= n_bits;

        partial_products.push_back(current_cutwire);
    }

    int current_level = 1;

    /*------------------------------------------------------------------------------------------
    As long as there are more than 2 entries in the partial products, we wire the next level.
    ------------------------------------------------------------------------------------------*/
    while(partial_products.size()>2){
        /*------------------------------------------------------------------------------------------
        We declare a vector to hold next level partial products
        ------------------------------------------------------------------------------------------*/
        vector<WALLACE_TREE_MULTIPLIER_PIPELINED::CutWire> next_level_partial_products;

        /*------------------------------------------------------------------------------------------
        For each groups of 3 possible, we wire Carry Save Adder (csa) for the next level. 
        
        See WALLACE_TREE_MULTIPLIER_PIPELINED::make_csa(...) for mode details.
        ------------------------------------------------------------------------------------------*/
        for(int i=0; i < partial_products.size()/3; i++){
            
            WALLACE_TREE_MULTIPLIER_PIPELINED::CutWire o1, o2;
            make_csa("WTM_LEVEL_" + to_string(current_level) + "_GROUP_" + to_string(i),
                    partial_products.at(i*3),
                    partial_products.at(i*3+1),
                    partial_products.at(i*3+2),
                    &o1, &o2);
            
            next_level_partial_products.push_back(o1);
            next_level_partial_products.push_back(o2);
        }

        /*------------------------------------------------------------------------------------------
        If there were any left over partial products (not included in any groups of 3), we just add
        them to the next level.
        ------------------------------------------------------------------------------------------*/
        for(int i=(partial_products.size()/3)*3; i<partial_products.size(); i++){
            next_level_partial_products.push_back(partial_products.at(i));
        }


        /*------------------------------------------------------------------------------------------
        We update the partial products for the next level of iteration.
        ------------------------------------------------------------------------------------------*/
        partial_products = next_level_partial_products;


        /*------------------------------------------------------------------------------------------
        If current level is a multiple of k, pass the partial products through a FlipFlop
        ------------------------------------------------------------------------------------------*/        
        if(current_level != 0 && current_level%pipeline_k==0){
            for(int i=0; i< partial_products.size(); i++){
                wire curr_output_wire = "WTM_LEVEL_" + to_string(current_level) + "_FLIP_FLOP_" + to_string(i) + "_WIRE";
                this->add_wire("[" + to_string(partial_products.at(i).length - 1) + ":0] " + curr_output_wire);
                
                FLIP_FLOP_N_BIT curr_flipflop("WTM_LEVEL_" + to_string(current_level) + "_FLIP_FLOP_" + to_string(i),
                                             {partial_products.at(i).wire_id , "CLK"},
                                             curr_output_wire, partial_products.at(i).length);
                
                partial_products.at(i).wire_id = curr_output_wire;
                this->add_submodule(curr_flipflop);               
            }
        }

        current_level++;
    }

    /*------------------------------------------------------------------------------------------
    Now there are just 2 entries remaining in the partial products. We read the entries and con-
    vert bot of them to a normal wire.
    ------------------------------------------------------------------------------------------*/ 
    CutWire final_cut_1, final_cut_2;
    final_cut_1 = partial_products.at(0);
    final_cut_2 = partial_products.at(1);

    wire final_1 = "WTM_FINAL_1";
    wire final_2 = "WTM_FINAL_2";

    this->add_wire("[" + to_string(2*this->n_bits - 1) + ":0]" + final_1);
    this->add_wire("[" + to_string(2*this->n_bits - 1) + ":0]" + final_2);

    /*------------------------------------------------------------------------------------------
    Pad zeros as needed to the Least Significant Bits.
    ------------------------------------------------------------------------------------------*/ 
    if(final_cut_1.shift > 0){
        this->verilog("assign " + final_1 + "[" + to_string(final_cut_1.shift - 1) + ":0] = 0;" );
    }
    if(final_cut_2.shift > 0){
        this->verilog("assign " + final_2 + "[" + to_string(final_cut_2.shift - 1) + ":0] = 0;" );
    }

    /*------------------------------------------------------------------------------------------
    Pad zeros as needed to the Most Significant Bits.
    ------------------------------------------------------------------------------------------*/ 
    if(final_cut_1.length < 2*n_bits){
        this->verilog("assign " + final_1 + "[" + to_string(2*n_bits - 1) + ":" + to_string(final_cut_1.length) + "] = 0;");
    }
    if(final_cut_2.length < 2*n_bits){
        this->verilog("assign " + final_2 + "[" + to_string(2*n_bits - 1) + ":" + to_string(final_cut_2.length) + "] = 0;");
    }
    
    /*------------------------------------------------------------------------------------------
    Join the new wires to the partial products wire.
    ------------------------------------------------------------------------------------------*/ 
    JOIN_N_BIT joint_1("WTM_FINAL_JOINT_1",
                       final_1 + "["+ to_string(min(final_cut_1.length + final_cut_1.shift - 1, 2*n_bits -1)) + ":" + to_string(final_cut_1.shift) + "]",
                       final_cut_1.wire_id + "[" + to_string(min(final_cut_1.length, 2*n_bits) -1)  + ":0]", min(final_cut_1.length, 2*n_bits));

    JOIN_N_BIT joint_2("WTM_FINAL_JOINT_2",
                       final_2 + "["+ to_string(min(final_cut_2.length + final_cut_2.shift - 1, 2*n_bits -1)) + ":" + to_string(final_cut_2.shift) + "]",
                       final_cut_2.wire_id + "[" + to_string(min(final_cut_2.length, 2*n_bits) -1)  + ":0]", min(final_cut_2.length, 2*n_bits));

    
    /*------------------------------------------------------------------------------------------
    Add a CLA at the end to sum thr last two partial products. Note that the CLA has a register
    at its end, so we do not need to add a register at the end.
    ------------------------------------------------------------------------------------------*/ 
    CARRY_LOOK_AHEAD_ADDER_PIPELINED final_cla("WTM_CLA",
                                    {final_1, final_2, "CLK"},
                                    "P",
                                    2*n_bits, pipeline_k);

    this->add_submodule(joint_1);
    this->add_submodule(joint_2);   
    this->add_submodule(final_cla);


    /*------------------------------------------------------------------------------------------
    Now we use the auto_gen function to create the module definition.
    ------------------------------------------------------------------------------------------*/ 
    this->definition = this->auto_gen("module WALLACE_TREE_MULTIPLIER_" + to_string(n_bits) + "_BIT");
    this->generate = "WALLACE_TREE_MULTIPLIER_" + to_string(n_bits) + "_BIT " + this->name + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ", " + this->outputs.at(0) + ");";
}


void WALLACE_TREE_MULTIPLIER_PIPELINED::make_csa(string name, CutWire w1, CutWire w2, CutWire w3, CutWire *o1, CutWire *o2){
    /*------------------------------------------------------------------------------------------
    Given 3 input CutWires, this function wires a Carry Save Adder most optimally to output 2
    CutWires, one for the Sum and the other for the Carry.

    Param: name (string), a unbique identifier to remove any conflicts
    Param: w1, w2, w3 (CutWire), partial products.
    Param: o1, o2 (&CutWire), poiters to next level outputs, i.e Sum and Carry from this level.
    ------------------------------------------------------------------------------------------*/ 
    
    int new_shift, new_width;
    int l1, l2, l3;

    l1 = w1.length + w1.shift;
    l2 = w2.length + w2.shift;
    l3 = w3.length + w3.shift;

    /*------------------------------------------------------------------------------------------
    We calculate the shift and width for the next level 
    ------------------------------------------------------------------------------------------*/ 
    new_shift = min(min(w1.shift, w2.shift), w3.shift);
    new_width = max(max(l1, l2), l3) - new_shift;
     

    vector<int> lengths = {l1, l2, l3};
    sort(lengths.begin(), lengths.end());


    /*------------------------------------------------------------------------------------------
    Declare new wires with appropriate sizes for inputs to the CSA (Carry Save Adder).
    ------------------------------------------------------------------------------------------*/ 
    wire wire_1 = name + "_WIRE_1";
    wire wire_2 = name + "_WIRE_2";
    wire wire_3 = name + "_WIRE_3";

    this->add_wire("[" + to_string(new_width - 1) + ":0] " + wire_1);
    this->add_wire("[" + to_string(new_width - 1) + ":0] " + wire_2);
    this->add_wire("[" + to_string(new_width - 1) + ":0] " + wire_3);


    /*------------------------------------------------------------------------------------------
    Join the appropriate parts of the partial products to the new wires.
    ------------------------------------------------------------------------------------------*/ 
    JOIN_N_BIT join_1(name + "_JOIN_1",
                        wire_1 + "[" + to_string(l1 - new_shift - 1) + ":" + to_string(w1.shift - new_shift) + "]",
                        w1.wire_id,
                        w1.length);

    JOIN_N_BIT join_2(name + "_JOIN_2",
                        wire_2 + "[" + to_string(l2 - new_shift - 1) + ":" + to_string(w2.shift - new_shift) + "]",
                        w2.wire_id,
                        w2.length);        
    
    JOIN_N_BIT join_3(name + "_JOIN_3",
                        wire_3 + "[" + to_string(l3 - new_shift - 1) + ":" + to_string(w3.shift - new_shift) + "]",
                        w3.wire_id,
                        w3.length);

    this->add_submodule(join_1);
    this->add_submodule(join_2);
    this->add_submodule(join_3);

    /*------------------------------------------------------------------------------------------
    Pad zeros to the LSB.
    ------------------------------------------------------------------------------------------*/ 
    if(w1.shift - new_shift > 0){
        this->verilog("assign " + wire_1 + "[" + to_string(w1.shift - new_shift - 1) + ":0] = 0;" );
    }
    if(w2.shift - new_shift > 0){
        this->verilog("assign " + wire_2 + "[" + to_string(w2.shift - new_shift - 1) + ":0] = 0;" );
    }
    if(w3.shift - new_shift > 0){
        this->verilog("assign " + wire_3 + "[" + to_string(w3.shift - new_shift - 1) + ":0] = 0;" );
    }
    
    /*------------------------------------------------------------------------------------------
    Pad zeros to the MSB.
    ------------------------------------------------------------------------------------------*/ 
    if(l1 - new_shift < new_width){
        this->verilog("assign " + wire_1 + "[" + to_string(new_width - 1) + ":" + to_string(l1- new_shift) + "] = 0;");
    }
    if(l2 - new_shift < new_width){
        this->verilog("assign " + wire_2 + "[" + to_string(new_width - 1) + ":" + to_string(l2- new_shift) + "] = 0;");
    }
    if(l3 - new_shift < new_width){
        this->verilog("assign " + wire_3 + "[" + to_string(new_width - 1) + ":" + to_string(l3- new_shift) + "] = 0;");
    }


    /*------------------------------------------------------------------------------------------
    Lengths are sorted in ascending order, now we compute the output sizes.
    ------------------------------------------------------------------------------------------*/ 
    if(lengths.at(2) == lengths.at(1)){
        /*------------------------------------------------------------------------------------------
        Case:
                0XXXXXX...X
              + XXXXXXX...X
              + XXXXXXX...X
        
        Length of Sum = new_width
        Length of Carry = new_width + 1
        ------------------------------------------------------------------------------------------*/ 
        o1->wire_id = name + "_SUM_WIRE";
        o1->shift = new_shift;
        o1->length = new_width;

        o2->wire_id = name + "_CARRY_WIRE";
        o2->shift = new_shift;
        o2->length = new_width + 1;

        CARRY_SAVE_ADDER adder(name + "_CSA",
                              {wire_1, wire_2, wire_3},
                              {o1->wire_id, o2->wire_id + "[" + to_string(new_width - 1) + ":0]", o2->wire_id + "[" + to_string(new_width) + "]"}, new_width);

        this->add_submodule(adder);
    }
    else{
        /*------------------------------------------------------------------------------------------
        Case:
                00XXXXX...X
              + 0XXXXXX...X
              + XXXXXXX...X
        
        Length of Sum = new_width
        Length of Carry = new_width
        ------------------------------------------------------------------------------------------*/ 
        o1->wire_id = name + "_SUM_WIRE";
        o1->shift = new_shift;
        o1->length = new_width;

        o2->wire_id = name + "_CARRY_WIRE";
        o2->shift = new_shift;
        o2->length = new_width;

        wire dangle = name + "_DANGLE";
        this->add_wire(dangle);
        CARRY_SAVE_ADDER adder(name + "_CSA",
                              {wire_1, wire_2, wire_3},
                              {o1->wire_id, o2->wire_id, dangle}, new_width);

        this->add_submodule(adder);
    }

    this->add_wire("["+ to_string(o1->length - 1) +":0]"+ o1->wire_id);
    this->add_wire("["+ to_string(o2->length - 1) +":0]"+ o2->wire_id);
}