/*---------------------------------------------------------
            Version 2 for Assignement 2 Updates
            ===================================

I have made slight changes to this code from Assignement 1

1. Added support for behaviour modelling.
2. Verilog 'reg' can be made.
3. Multi width flipflops.
4. Multi width inputs and outputs
5. Pipelined CLA
6. Carry Saver Adder
7. Pipelined Wallace Tree Multiplier

---------------------------------------------------------*/

/*-------------------------------------------------------
                Headers for Verilog Modules
                ===========================
                        ME17B174

Every Module in verilog can be expressed as a derrivative
of Chip class. Chip class provides methods for automatic-
ally recursing through submodules and generating module -
definitions, adding input ports, output ports and wires.

A Chip can also contain a Behaviour model as its submodule.

Basic Modules like logic gates, flipflops are already im-
plimented.


---------------------------------------------------------*/

#ifndef VERILOG_H
#define VERILOG_H

#define CHIP_INPUTS 0
#define CHIP_OUTPUTS 1
#define CHIP_INPUTS_REG 2
#define CHIP_OUTPUTS_REG 3

#define BLOCK true
#define N_BLOCK false

#define wire string
#define reg string

#include <constants.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <set>
#include <cmath>

using namespace std;

wire wire_name(wire full_name);

class BehaviourModel{
    /*------------------------------------------------------------------------------------------
                                        The Behaviour Model
                                        ===================

    
    One major addition to the previous version (Assignment 1) is the implimentation of Behavour 
    Modelling. This enables easier modelling of complex circuits like Flipflops with fewer lines
    of code.
 
    Every model contains a set of sensitivity lists (which can be wires, regs or can contain ver-
    log statements like posedge wire). 

    The statements are added using the add() funtion. Delays can also be given as inputs.

    Example Usage:

        BehaviourModel my_behaviour({"posedge clk", "a"});
        my_behaviour.add("b", "#10 ~a", N_BLOCK) // For Nonblocking
        my_behaviour.add("c", "#10 b&a", BLOCK) // For Blocking statements

    As of now, the number of transistors need to be given manually, or you can ignore it if you
    are not bookkeeping.

    Usefull methods:
        1. add(...) - Used to add a new statement as shown above
        2. code() - Returns the verilog code for the behaviour as string.
                End users do not have to use this function, as the Chip module automatically calls
                this function.
    
    A Behaviour can be attached to the parent chip while constructing,
    For example,

        FLIP_FLOP::FLIP_FLOP(string name, vector<wire> input_wires, wire output_wire){
            this->name = name;
            this->inputs = input_wires;
            this->outputs.push_back(output_wire);

            this->declare("A", CHIP_INPUTS);
            this->declare("CLK", CHIP_INPUTS);
            this->declare("OUT", CHIP_OUTPUTS_REG);                     // Now you can have a 'reg' as port

            BehaviourModel flipflop({"posedge CLK"});
            flipflop.add("OUT", "#" + to_string(FLIP_FLOP_DELAY) + " A", N_BLOCK);
            flipflop.num_transistors = FLIP_FLOP_TRANSISTORS; 

                                                                         /
            this->add_submodule(flipflop);                              --------- SEE THIS LINE
                                                                         \  
            
            
            this->definition = this->auto_gen("module FLIP_FLOP");
            this->generate = "FLIP_FLOP " + this->name + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ", " + this->outputs.at(0) + " );";

        }

    

    -----------------------------------------------------------------------------------------*/
    public:
        BehaviourModel(vector<string> sense_list);
        ~BehaviourModel(){};
        int num_transistors = 0;

        string code();
        void add(string lhs, string rhs, bool is_blocking);

    protected:
        vector<string> statements;
        vector<string> sensitivity_list;
};


class Chip{
    /*------------------------------------------------------------------------------------------
                                        The Chip Module
                                        ===============

    Every Module in verilog can be expressed as a derrivative of Chip class. Chip class provides
    methods for automatically recursing through submodules and generating module definitions, 
    adding input ports, output ports and wires. 

    The updated Chip modules can now have 'regs' and BehaviourModels

    Every instance of a Chip Module has:
        1. name (string) : A name
        2. submodules (vector<Chip>) : A list of submodules automatically added
        3. definition (string) : A module definition syntax string
                                example:

                                module AND_GATE(a, b, c);
                                    input a, b;
                                    output c;
                                    assign c = a & b;
                                endmodule

        4. generate (string) : A generate string (or module instantiation syntax string) 
                                example:
                                
                                AND_GATE and_instance(input_1, input_2, output);

        5. n_bits (int) : Bit width
        6. inputs, outputs, wires (vector<wire>) : Input/Output/Wires (Instance specific)
                                These are instance specific, example when you instantiate,
                                
                                    MyChip my_chip_1(inputs, outputs);

                                    inputs and outputs are instance specific
        
        7. input_names, output_names (vector<string>) : Generic input name (Static)
                                These are generic name used in module definition
                                example:
                                    MyChip my_chip_1(inputs, outputs);

                                will generate headers as:
                                    module MyChip (input_names, output_names);
                                        ...
                                    endmodule
        
        8. Transistors : Number of transistors used to make current module.
    
    Some usefull methods:
        1. add_submodule(Chip sub_chip) : Adds the passed chip as submodule

        2. add_wire(wire new_wire) : Adds the passed wire as wire

        3. declare(wire port, int type): Declares input and output ports
                example declare("my_input", CHIP_INPUTS);
                        declare("my_output", CHIP_OUTPUT);

        4. define_headers() : Returns a set of module definition by reecursively iterating
                through sub modules.

        5. auto_gen(string head) : Automatically generates the module definition.
                example:
                    CRA.autogen("MY_CRA_MODULE") returns:

                        module MY_CRAMODULE(...);
                            ...
                        endmodule;
                    
                    populated automatically.

        6. verilog(string e) : Embeds native verilog code specified.
                example
                    this->verilog("assign c = a~b;")
                will insert the code snippet in the module.

        7. generate_verilog() : Generates the verilog code for the current chip as string.

        8. num_transistors() : Returns the transistor count for one instance

        NOTE : All wires/ports/reg are of type string  
    -----------------------------------------------------------------------------------------*/

    public:
        string name;
        ~Chip(){};
        
        string definition;
        string generate;

        int transistors = 0;

        void add_submodule(Chip sub);
        void add_submodule(BehaviourModel behaviour);
        void add_wire(wire new_wire);
        void add_reg(reg new_reg);
        void declare(wire e, int type);
	    void declare(wire e, int type, int width);
        void verilog(string e);
        int num_transistors();

        set<string> define_headers();
        string auto_gen(string head);
        string generate_verilog();
        
    protected:
        vector<Chip> submodules;
        vector<wire> inputs;
        vector<wire> outputs;
        vector<wire> wires;
        vector<wire> extras;
        vector<wire> input_names;
        vector<bool> is_input_reg;
        vector<wire> output_names;
        vector<bool> is_output_reg; 
        vector<reg>  regs;
        vector<BehaviourModel> behaviours;

        int n_bits = 1;
};



/*------------------------------------------------------------------------------------------
                                    Some Basic Logic Gates
                                    ======================
------------------------------------------------------------------------------------------*/

class JOIN : public Chip{
    public:
        JOIN(string name, wire a, wire b);
};

class JOIN_N_BIT : public Chip{
    public:
        JOIN_N_BIT(string name, wire a, wire b, int n_bits);
};

class AND : public Chip{
    public:
        AND(string name, vector<wire> input_wires, wire output_wires);    
};

class AND_N_BIT : public Chip{
    public:
        
        AND_N_BIT(string name, vector<wire> input_wires, wire output_wire, int n_bits); 
};

class XOR : public Chip{
    public:
        XOR(wire name, vector<wire> input_wires, wire output_wire);  
};

class XOR_N_BIT : public Chip{
    public:     
        XOR_N_BIT(string name, vector<wire> input_wires, wire output_wire, int n_bits);
};

class OR : public Chip{
        public:
        OR(string name, vector<wire> input_wires, wire output_wire);  
};

class OR_N_BIT : public Chip{
    public:
        OR_N_BIT(string name, vector<wire> input_wires, wire output_wire, int n_bits);
       
};

class NAND : public Chip{
    public:
        NAND(string name, vector<wire> input_wires, wire output_wire);
        
};

class NAND_N_BIT : public Chip{
    public:
        NAND_N_BIT(string name, vector<wire> input_wires, wire output_wire, int n_bits);
        
};

class NOR : public Chip{
    public:
        NOR(string name, vector<wire> input_wires, wire output_wire);  
};

class NOR_N_BIT : public Chip{
    public:
        NOR_N_BIT(string name, vector<wire> input_wires, wire output_wire, int n_bits);
};

class NOT : public Chip{
    public:
        NOT(string name, wire input_wire, wire output_wire); 
};

class NOT_N_BIT : public Chip{
    public:
        NOT_N_BIT(string name, wire input_wire, wire output_wire, int n_bits);
};


/*------------------------------------------------------------------------------------------
                                CARRY RIPPLE ADDER DEFINITION
                                =============================
                            See src/verilog.cpp for implimentation
------------------------------------------------------------------------------------------*/

class CARRY_RIPPLE_ADDER : public Chip{
    public:
        CARRY_RIPPLE_ADDER(string name, vector<wire> input_wires, wire output_wire, int n_bits);
};

class FULL_ADDER : public Chip{
    public:
        FULL_ADDER(string name, vector<wire> input_wires, vector<wire> ouput_wires);
};

/*------------------------------------------------------------------------------------------
                            CARRY LOOK AHEAD ADDER DEFINITION
                            =================================
                          See src/verilog.cpp for implimentation
------------------------------------------------------------------------------------------*/

class CARRY_LOOK_AHEAD_ADDER : public Chip{
    public:
        CARRY_LOOK_AHEAD_ADDER(string name, vector<wire> input_wires, wire output_wire, int n_bits);
};

class CLA_STAR : public Chip{
    public:
        CLA_STAR(string name, vector<wire> input_wires, vector<wire>output_wires);
};


/*------------------------------------------------------------------------------------------
                                Flipflops and Registors
                                =======================
------------------------------------------------------------------------------------------*/

class FLIP_FLOP : public Chip{
    public:
        FLIP_FLOP(string name, vector<wire> input_wires, wire output_wire);
};

class FLIP_FLOP_N_BIT : public Chip{
    public:
        FLIP_FLOP_N_BIT(string name, vector<wire> input_wires, wire output_wire, int n_bits);
};

/*------------------------------------------------------------------------------------------
                            CARRY LOOK AHEAD ADDER WITH PIPELINING
                            ======================================
------------------------------------------------------------------------------------------*/

class CARRY_LOOK_AHEAD_ADDER_PIPELINED : public Chip{
    public:
        CARRY_LOOK_AHEAD_ADDER_PIPELINED(string name, vector<wire> input_wires, wire output_wire, int n_bits, int pipeline_k);
};

/*------------------------------------------------------------------------------------------
                                    CARRY SAVE ADDER
                                    ================
------------------------------------------------------------------------------------------*/

class CARRY_SAVE_ADDER : public Chip{ 
    public:
        CARRY_SAVE_ADDER(string name, vector<wire> input_wires, vector<wire> output_wires, int n_bits);
};
#endif


/*------------------------------------------------------------------------------------------
                            WALLACE TREE MULTIPLIER WITH PIPELINING
                            =======================================
------------------------------------------------------------------------------------------*/

class WALLACE_TREE_MULTIPLIER_PIPELINED : public Chip{
    public:
        WALLACE_TREE_MULTIPLIER_PIPELINED(string name, vector<wire> input_wires, wire output_wire, int n_bits, int pipeline_k);
        typedef struct cut_wire{
            wire wire_id;
            int shift=0;
            int length=0;
        } CutWire;

        void make_csa(string name, CutWire w1, CutWire w2, CutWire w3, CutWire *o1, CutWire *o2);
};
