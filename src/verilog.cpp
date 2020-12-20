#include <verilog.h>
#include <stdlib.h>
#include <vector>
#include <set>
#include <iostream>
#include <string> 
#include <stdexcept>
#include <cmath>
#include <stdio.h>

using namespace std;

wire wire_name(wire full_name){
    /*------------------------------------------------------------------------------------------
    This is a very usefull function to extract the wire name from a wire declaration string.

    Params : full_name (wire), full name including width. Example : "[9:0] my_wire"
    Returns : Parsed string, if input contains infomration about widths. Example : "my_wire"                      
    ------------------------------------------------------------------------------------------*/
    string delim = "] ";
    size_t index;
    if(string::npos != (index = full_name.find(delim, 0))){
        return full_name.substr(index + 2);
    }
    else{
        return full_name;
    }
}

string Chip::generate_verilog(){
    /*------------------------------------------------------------------------------------------
    This function generates a formated output string verilog code.       

    Params : None
    Returns : code (string), output verilog code                          
    ------------------------------------------------------------------------------------------*/
    string code = "// Generated code\n// Sooryakiran P\n// ME17B174\n";
    set<string> headers = this->define_headers();
    for (set<string>::iterator i = headers.begin(); i != headers.end(); i++){
        code += "\n" + *i + "\n";
    }
    return code;
}


set<string> Chip::define_headers(){
    /*------------------------------------------------------------------------------------------
    This function recursively iterates through all the submodules if any to generate a set of
    module definitions.         

    Params : None
    Returns : all_headers (set<string>), a set of all module and submodule definitions.                          
    ------------------------------------------------------------------------------------------*/
    set<string> all_headers;
    all_headers.insert(this->definition);

    for(vector<Chip>::iterator sub = this->submodules.begin();
                               sub != this->submodules.end();
                               sub++){
        set<string> sub_header = sub->define_headers();
        all_headers.insert(sub_header.begin(), sub_header.end());
    }

    return all_headers;
}

int Chip::num_transistors(){
    /*------------------------------------------------------------------------------------------
    This function recursively iterates through all the submodules and behaviours if any to count
    the number of transistors required to create an instance.

    Params : None
    Returns : total (int), transitor count.                         
    ------------------------------------------------------------------------------------------*/
    int total = this->transistors;
    
    for(vector<Chip>::iterator sub = this->submodules.begin();
                               sub != this->submodules.end();
                               sub++){

        total += sub->num_transistors();
    }

    for(vector<BehaviourModel>::iterator sub = this->behaviours.begin();
                                         sub != this->behaviours.end();
                                         sub ++){
        total += sub->num_transistors;
    }

    return total;
}

void Chip::declare(wire e, int type){
    /*------------------------------------------------------------------------------------------

    This function is used to populate the Chip ports when declared.
    example :   declare("my_input", CHIP_INPUTS);
                declare("my_output", CHIP_OUTPUT); 
                declare("my_output_reg", CHIP_OUTPUT_REG);
    

    Param : c (wire), a port/wire name.
    Param : type (int), port type, 0 for CHIP_INPUTS, 1 for CHIP_OUTPUTS, 2 for CHIP_INPUT_REG 
            and 3 for CHIP_OUTPUT_REG.
    Returns : None
    ------------------------------------------------------------------------------------------*/
   
    string pre = "";
    if(this->n_bits > 1){
        pre = "[" + to_string(this->n_bits -1) + ": 0] ";
    } 
    
    if(type == CHIP_INPUTS){
        this->input_names.push_back(pre + e);
        this->is_input_reg.push_back(false);
    }
    else if(type == CHIP_OUTPUTS){
        this->output_names.push_back(pre + e);
        this->is_output_reg.push_back(false);
    }
    else if(type == CHIP_INPUTS_REG){
        this->input_names.push_back(pre + e);
        this->is_input_reg.push_back(true);
    }
    else if(type == CHIP_OUTPUTS_REG){
        this->output_names.push_back(pre + e);
        this->is_output_reg.push_back(true);
    }
    else{
        throw invalid_argument( "Invalid type for port." );
    }
}

void Chip::declare(wire e, int type, int width){
    /*------------------------------------------------------------------------------------------

    This function is used to populate the Chip ports when declared.
    example :   declare("my_input", CHIP_INPUTS);
                declare("my_output", CHIP_OUTPUT); 
                declare("my_output_reg", CHIP_OUTPUT_REG);
    

    Param : c (wire), a port/wire name.
    Param : type (int), port type, 0 for CHIP_INPUTS, 1 for CHIP_OUTPUTS, 2 for CHIP_INPUT_REG 
            and 3 for CHIP_OUTPUT_REG.
    Param : width (int), Bit width of port. If no width is specified, it is assumed to be of the
            parent Chip's width.
    Returns : None
    ------------------------------------------------------------------------------------------*/ 
    
    string pre = "";
    if(width>1){
        pre = "[" + to_string(width -1) + ": 0] ";
    }
    
    if(type == CHIP_INPUTS){
        this->input_names.push_back(pre + e);
        this->is_input_reg.push_back(false);
    }
    else if(type == CHIP_OUTPUTS){
        this->output_names.push_back(pre + e);
        this->is_output_reg.push_back(false);
    }
    else if(type == CHIP_INPUTS_REG){
        this->input_names.push_back(pre + e);
        this->is_input_reg.push_back(true);
    }
    else if(type == CHIP_OUTPUTS_REG){
        this->output_names.push_back(pre + e);
        this->is_output_reg.push_back(true);
    }
    else{
        throw invalid_argument( "Invalid type for port." );
    }
}

void Chip::add_submodule(Chip sub){
    /*------------------------------------------------------------------------------------------
    Adds a new chip as a submodule for the current chip.

    Param : sub (Chip), a submodule chip.
    Returns : None
    ------------------------------------------------------------------------------------------*/
    this->submodules.push_back(sub);
}

void Chip::add_submodule(BehaviourModel behaviour){
    /*------------------------------------------------------------------------------------------
    Adds a new Behaviou Model into the current chip.

    Param : behaviour (BehaviourModel), a behaviour model.
    Returns : None
    ------------------------------------------------------------------------------------------*/

    this->behaviours.push_back(behaviour);
}

void Chip::add_wire(wire new_wire){
    /*------------------------------------------------------------------------------------------
    Adds a new wire to the current chip.

    Param : new_wire (wire), a wire.
    Returns : None
    ------------------------------------------------------------------------------------------*/
    this->wires.push_back(new_wire);
}

void Chip::add_reg(reg new_reg){
    /*------------------------------------------------------------------------------------------
    Adds a new reg to the current chip.

    Param : new_reg (reg), a reg.
    Returns : None
    ------------------------------------------------------------------------------------------*/
    this->regs.push_back(new_reg);
}


void Chip::verilog(string e){
    /*------------------------------------------------------------------------------------------
    Embeds native Verilog code

    Param : e (string), code to be embedded.
    Returns : None
    ------------------------------------------------------------------------------------------*/
    this->extras.push_back(e);
}

string Chip::auto_gen(string head){
    /*------------------------------------------------------------------------------------------
    Automatically generates the chip module header definition

    Param : head (string), a module name.
    Returns : head_def

    Example if head = "module MY_CHIP"
    ------------------------------------------------------------------------------------------*/

    string head_def = head + " (";

    /*------------------------------------------------------------------------------------------
    head_def will be "module MY_CHIP("

    The next stage checks for any input declarations and head def will be appended
    ------------------------------------------------------------------------------------------*/

    for(vector<string>::iterator i = this->input_names.begin(); i != this->input_names.end(); i++){
        if(i!=this->input_names.begin()){
            head_def += ", ";
        }
        string input = *i;
        head_def += wire_name(input);
    }

    /*------------------------------------------------------------------------------------------
    The next stage checks for any output declarations and head def will be appended
    ------------------------------------------------------------------------------------------*/
    for(vector<string>::iterator i = this->output_names.begin(); i != this->output_names.end(); i++){
        string input = *i;
        head_def += ", " + wire_name(input);
    }
    head_def += ");";

    /*------------------------------------------------------------------------------------------
    At this stage head_def will be "module MY_CHIP(my_input_1, my_input_2, my_output);"
    ------------------------------------------------------------------------------------------*/

    // string pre = "";
    // if(this->n_bits > 1){
    //     pre = "[" + to_string(this->n_bits -1) + ": 0] ";
    // } 
    
    head_def += "\n\t// Transistor count : " + to_string(this->num_transistors()) + "\n";
    /*------------------------------------------------------------------------------------------
    In the next stage we again write these inputs and outputs inside the module. We also checked
    for the bit width to make appropriate declarations.
    ------------------------------------------------------------------------------------------*/
    head_def += "\n\t// Inputs\n\n";

    for(int i=0; i < this->input_names.size(); i++){
        if(this->is_input_reg.at(i))
            head_def += "\tinput reg "  + this->input_names.at(i) + ";\n";
        else
            head_def += "\tinput "  + this->input_names.at(i) + ";\n";
    }

    head_def += "\n\t// Outputs\n\n";
    for(int i=0; i < this->output_names.size(); i++){
        if(this->is_output_reg.at(i))
            head_def += "\toutput reg "  + this->output_names.at(i) + ";\n";
        else
            head_def += "\toutput "  + this->output_names.at(i) + ";\n";
    }

    /*------------------------------------------------------------------------------------------
    At this stage head_def will be assuming 8 bit width
    """
    module MY_CHIP(my_input_1, my_input_2, my_output);
        // Inputs
        input [7:0] my_input_1;
        input [7:0] my_input_2;
        // Outputs
        output [7:0] my_output;
    """
    ------------------------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------------------------
    Now we add any wires and reg declared.
    ------------------------------------------------------------------------------------------*/
    head_def += "\n\t// Wires\n\n";
    for(vector<string>::iterator i = this->wires.begin(); i != this->wires.end(); i++){
        head_def += "\twire " + *i + ";\n";
    }

    head_def += "\n\t// Regs\n\n";
    for(vector<string>::iterator i = this->regs.begin(); i != this->regs.end(); i++){
        head_def += "\treg " + *i + ";\n";
    }

    /*------------------------------------------------------------------------------------------
    Embed any verilog codes.
    ------------------------------------------------------------------------------------------*/
    head_def += "\n\t// Extras\n\n";
    for(vector<string>::iterator i = this->extras.begin(); i != this->extras.end(); i++){
        head_def += "\t"+ *i + "\n";
    }

    /*------------------------------------------------------------------------------------------
    At this stage head_def will be something like
    """
    module MY_CHIP(my_input_1, my_input_2, my_output);
        // Inputs
        input [7:0] my_input_1;
        input [7:0] my_input_2;

        // Outputs
        output [7:0] my_output;

        // Wires
        wire my_wire_1;
        wire my_wire_2;
        wire [7:0] my_wide_wire;

        // Extras
        assign my_wire_1 = 0;

    """
    ------------------------------------------------------------------------------------------*/

    /*------------------------------------------------------------------------------------------
    Now add submodule instances.
    ------------------------------------------------------------------------------------------*/

    head_def += "\n\t// Sub Modules\n\n";
    for(vector<Chip>::iterator i = this->submodules.begin(); i != this->submodules.end(); i++){
        head_def += "\t" + i->generate + "\n";
    }

    /*------------------------------------------------------------------------------------------
    Now we add the behaviours specified
    ------------------------------------------------------------------------------------------*/

    head_def += "\n\t// Behaviours\n\n";
    for(vector<BehaviourModel>::iterator i = this->behaviours.begin(); i != this->behaviours.end(); i++){
        string code = i->code();
        size_t index = 0;
        while(string::npos != (index = code.find('\n', index))){
            code.replace(index, 1, "\n\t");
            index +=2;
        }
        

        head_def += "\t" + code + "\n";
    }

    head_def += "\nendmodule\n";

    /*------------------------------------------------------------------------------------------
    At this stage head_def will be something like
    """
    module MY_CHIP(my_input_1, my_input_2, my_output);
        // Inputs
        input [7:0] my_input_1;
        input [7:0] my_input_2;

        // Outputs
        output [7:0] my_output;

        // Wires
        wire my_wire_1;
        wire my_wire_2;
        wire [7:0] my_wide_wire;

        // Regs
        reg a;

        // Extras
        assign my_wire_1 = 0;

        // Submodules
        AND_GATE my_and_1(my_input_1[0], my_input_2[0], my_wire_1);
        ...

        // Behaviours
        
        always @(posedge my_input_1);
        begin;
            a <= ~my_input_2;
        end;

        ...

    endmodule
    """
    ------------------------------------------------------------------------------------------*/
    return head_def;
}

/*------------------------------------------------------------------------------------------
                                    Behaviour Modelling
                                    ===================
Implimentation of the behaviour model.
------------------------------------------------------------------------------------------*/

BehaviourModel::BehaviourModel(vector<string> sense_list){
    /*------------------------------------------------------------------------------------------
    The class constructor. 

    Param: sense_list (vector<string>), Sensitivity list
    ------------------------------------------------------------------------------------------*/
    this->sensitivity_list = sense_list;
}

void BehaviourModel::add(string lhs, string rhs, bool is_blocking){
    /*------------------------------------------------------------------------------------------
    Adds a new statement to the behaviour model.

    Param: lhs(string), Left hand side of the statement.
    Param: rhs(string), Right hand side of the statment.
    Param: is_blocking(bool), BLOCK/N_BLOCK for blovking or non blocking statements
    ------------------------------------------------------------------------------------------*/
    string asg = (is_blocking)? " = " : " <= " ;
    string expr = lhs + asg + rhs;
    this->statements.push_back(expr);
}

string BehaviourModel::code(){
    /*------------------------------------------------------------------------------------------
    TReturns the verilog code for the current behaviour as string.

    Returns: outs(string), verilog code for the current behaviour.
    ------------------------------------------------------------------------------------------*/
    string outs = "always @(";
    
    for(vector<string>::iterator i = this->sensitivity_list.begin(); i != this->sensitivity_list.end(); i++){
        if(i != this->sensitivity_list.begin()){
            outs += " or ";
        }
        outs += *i;
    }
    outs += ")\nbegin";

    for(vector<string>::iterator i = this->statements.begin(); i != this->statements.end(); i++){
        outs += "\n\t" + *i + ";";
    }
    outs +="\nend";
    return outs;
}


/*------------------------------------------------------------------------------------------
                                    JOINING 2 WIRES
                                    ===============
    Disclaimer : IThe trick to join 2 wires is taken from this page. 
    https://groups.google.com/g/comp.lang.verilog/c/b3-6XMA8KA4/m/b-7zIz0bW6gJ
------------------------------------------------------------------------------------------*/

JOIN::JOIN(string name, wire a, wire b){
    this->name = name;
    this->inputs.push_back(a);
    this->inputs.push_back(b);
    this->transistors = 0;
    this->definition = "module JOIN(a, a);\n"
                       "\t inout a;\n"
                       "endmodule";
    this->generate = "JOIN " + this->name + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ");";
}

JOIN_N_BIT::JOIN_N_BIT(string name, wire a, wire b, int n_bits){
    this->name = name;
    this->inputs.push_back(a);
    this->inputs.push_back(b);
    this->transistors = 0;
    this->n_bits = n_bits;
    this->definition = "module JOIN_" + to_string(n_bits) + "_BIT (a, a);\n"
                       "\t inout [" + to_string(n_bits - 1) + ":0] a;\n"
                       "endmodule";
    this->generate = "JOIN_" + to_string(n_bits) + "_BIT " + this->name + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ");";
}


/*------------------------------------------------------------------------------------------
                                        AND GATE
                                        ========
------------------------------------------------------------------------------------------*/

AND::AND(string name, vector<wire> input_wires, wire output_wire){
    this->name = name;
    this->inputs = input_wires;
    this->outputs.push_back(output_wire);
    this->transistors = AND_TRANSISTORS;
    this->definition = "module AND_GATE(a, b, c);\n"
                       "\t input a, b;\n"
                       "\t output c;\n"
                       "\t assign #" + to_string(AND_DELAY) + " c = a & b;\n"
                       "endmodule";
    this->generate = "AND_GATE " + this->name + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ", " + this->outputs.at(0) + ");";
}


/*------------------------------------------------------------------------------------------
                                    AND GATE N BIT
                                    ==============
------------------------------------------------------------------------------------------*/

AND_N_BIT::AND_N_BIT(string name, vector<wire> input_wires, wire output_wire, int n_bits){
    this->name = name;
    this->inputs = input_wires;
    this->outputs.push_back(output_wire);
    this->transistors = AND_TRANSISTORS*n_bits;
    this->n_bits = n_bits;

    string array_def = "[" + to_string(n_bits -1) + ":0]";
    this->definition = "module AND_GATE_" + to_string(n_bits) + "_BIT(a, b, c);\n"
                       "\t input " + array_def + " a, b;\n"
                       "\t output " + array_def + " c;\n"
                       "\t assign #" + to_string(AND_DELAY) + " c = a & b;\n"
                       "endmodule";    

    this->generate =  "AND_GATE_" + to_string(this->n_bits) + "_BIT " + this->name + + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ", " + this->outputs.at(0) + ");";
}


/*------------------------------------------------------------------------------------------
                                        XOR GATE
                                        ========
------------------------------------------------------------------------------------------*/

XOR::XOR(string name, vector<wire> input_wires, wire output_wire){
    this->name = name;
    this->inputs = input_wires;
    this->outputs.push_back(output_wire);
    this->transistors = XOR_TRANSISTORS;
    this->definition = "module XOR_GATE(a, b, c);\n"
                    "\t input a, b;\n"
                    "\t output c;\n"
                    "\t assign #" + to_string(XOR_DELAY) + " c = a ^ b;\n"
                    "endmodule";

    this->generate =  "XOR_GATE " + this->name + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ", " + this->outputs.at(0) + ");";
}

/*------------------------------------------------------------------------------------------
                                    XOR GATE N BIT
                                    ==============
------------------------------------------------------------------------------------------*/

XOR_N_BIT::XOR_N_BIT(string name, vector<wire> input_wires, wire output_wire, int n_bits){
    this->name = name;
    this->inputs = input_wires;
    this->outputs.push_back(output_wire);
    this->transistors = XOR_TRANSISTORS*n_bits;
    this->n_bits = n_bits;

    string array_def = "[" + to_string(n_bits -1) + ":0]";
    this->definition = "module XOR_GATE_" + to_string(n_bits) + "_BIT(a, b, c);\n"
                       "\t input " + array_def + " a, b;\n"
                       "\t output " + array_def + " c;\n"
                       "\t assign #" + to_string(XOR_DELAY) + "  c = a ^ b;\n"
                       "endmodule";    

    this->generate =  "XOR_GATE_" + to_string(this->n_bits) + "_BIT " + this->name + + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ", " + this->outputs.at(0) + ");";
}


/*------------------------------------------------------------------------------------------
                                        OR GATE
                                        =======
------------------------------------------------------------------------------------------*/

OR::OR(string name, vector<wire> input_wires, wire output_wire){
    this->name = name;
    this->inputs = input_wires;
    this->outputs.push_back(output_wire);
    this->transistors = OR_TRANSISTORS;
    this->definition = "module OR_GATE(a, b, c);\n"
                    "\t input a, b;\n"
                    "\t output c;\n"
                    "\t assign #" + to_string(OR_DELAY) + "  c = a | b;\n"
                    "endmodule";

    this->generate =  "OR_GATE " + this->name + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ", " + this->outputs.at(0) + ");";
}

/*------------------------------------------------------------------------------------------
                                    OR GATE N BIT
                                    =============
------------------------------------------------------------------------------------------*/

OR_N_BIT::OR_N_BIT(string name, vector<wire> input_wires, wire output_wire, int n_bits){
    this->name = name;
    this->inputs = input_wires;
    this->outputs.push_back(output_wire);
    this->transistors = OR_TRANSISTORS*n_bits;
    this->n_bits = n_bits;

    string array_def = "[" + to_string(n_bits -1) + ":0]";
    this->definition = "module OR_GATE_" + to_string(n_bits) + "_BIT(a, b, c);\n"
                       "\t input " + array_def + " a, b;\n"
                       "\t output " + array_def + " c;\n"
                       "\t assign #" + to_string(OR_DELAY) + "  c = a | b;\n"
                       "endmodule";    

    this->generate =  "OR_GATE_" + to_string(this->n_bits) + "_BIT " + this->name + + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ", " + this->outputs.at(0) + ");";
}


/*------------------------------------------------------------------------------------------
                                        NAND GATE
                                        =========
------------------------------------------------------------------------------------------*/

NAND::NAND(string name, vector<wire> input_wires, wire output_wire){
    this->name = name;
    this->inputs = input_wires;
    this->outputs.push_back(output_wire);
    this->transistors = NAND_TRANSISTORS;
    this->definition = "module NAND_GATE(a, b, c);\n"
                    "\t input a, b;\n"
                    "\t output c;\n"
                    "\t assign #" + to_string(NAND_DELAY) + "  c = ~(a & b);\n"
                    "endmodule";

    this->generate = "NAND_GATE " + this->name + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ", " + this->outputs.at(0) + ");";
}


/*------------------------------------------------------------------------------------------
                                    NAND GATE N BIT
                                    ===============
------------------------------------------------------------------------------------------*/

NAND_N_BIT::NAND_N_BIT(string name, vector<wire> input_wires, wire output_wire, int n_bits){
    this->name = name;
    this->inputs = input_wires;
    this->outputs.push_back(output_wire);
    this->transistors = NAND_TRANSISTORS*n_bits;
    this->n_bits = n_bits;

    string array_def = "[" + to_string(n_bits -1) + ":0]";
    this->definition = "module NAND_GATE_" + to_string(n_bits) + "_BIT(a, b, c);\n"
                       "\t input " + array_def + " a, b;\n"
                       "\t output " + array_def + " c;\n"
                       "\t assign #" + to_string(NAND_DELAY) + "  c = ~(a & b);\n"
                       "endmodule";    

    this->generate = "NAND_GATE_" + to_string(this->n_bits) + "_BIT " + this->name + + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ", " + this->outputs.at(0) + ");";
}


/*------------------------------------------------------------------------------------------
                                        NOR GATE
                                        ========
------------------------------------------------------------------------------------------*/

NOR::NOR(string name, vector<wire> input_wires, wire output_wire){
    this->name = name;
    this->inputs = input_wires;
    this->outputs.push_back(output_wire);
    this->transistors = NOR_TRANSISTORS;
    this->definition = "module NOR_GATE(a, b, c);\n"
                    "\t input a, b;\n"
                    "\t output c;\n"
                    "\t assign #" + to_string(NOR_DELAY) + "  c = ~(a | b);\n"
                    "endmodule";

    this->generate = "NOR_GATE " + this->name + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ", " + this->outputs.at(0) + ");";
}

/*------------------------------------------------------------------------------------------
                                    NOR GATE N BIT
                                    ==============
------------------------------------------------------------------------------------------*/

NOR_N_BIT::NOR_N_BIT(string name, vector<wire> input_wires, wire output_wire, int n_bits){
    this->name = name;
    this->inputs = input_wires;
    this->outputs.push_back(output_wire);
    this->transistors = NOR_TRANSISTORS*n_bits;
    this->n_bits = n_bits;

    string array_def = "[" + to_string(n_bits -1) + ":0]";
    this->definition = "module NOR_GATE_" + to_string(n_bits) + "_BIT(a, b, c);\n"
                       "\t input " + array_def + " a, b;\n"
                       "\t output " + array_def + " c;\n"
                       "\t assign #" + to_string(NOR_DELAY) + "  c = ~(a | b);\n"
                       "endmodule";    

    this->generate =  "NOR_GATE_" + to_string(this->n_bits) + "_BIT " + this->name + + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ", " + this->outputs.at(0) + ");";
}


/*------------------------------------------------------------------------------------------
                                        NOT GATE
                                        ========
------------------------------------------------------------------------------------------*/

NOT::NOT(string name, wire input_wire, wire output_wire){
    this->name = name;
    this->inputs.push_back(input_wire);
    this->outputs.push_back(output_wire);
    this->transistors = NOT_TRANSISTORS;
    this->definition = "module NOT_GATE(in, out);\n"
                    "\t input in;\n"
                    "\t output out;\n"
                    "\t assign #" + to_string(NOT_DELAY) + "  out = ~in;\n"
                    "endmodule";

    this->generate =  "NOT_GATE " + this->name + " (" + this->inputs.at(0) + ", " + this->outputs.at(0) + ");";
}

/*------------------------------------------------------------------------------------------
                                    NOT GATE N BIT
                                    ==============
------------------------------------------------------------------------------------------*/

NOT_N_BIT::NOT_N_BIT(string name, wire input_wire, wire output_wire, int n_bits){
    this->name = name;
    this->inputs.push_back(input_wire);
    this->outputs.push_back(output_wire);
    this->transistors = NOT_TRANSISTORS*n_bits;
    this->n_bits = n_bits;

    string array_def = "[" + to_string(n_bits -1) + ":0]";
    this->definition = "module NOT_GATE_" + to_string(n_bits) + "_BIT(in, out);\n"
                       "\t input " + array_def + " in;\n"
                       "\t output " + array_def + " out;\n"
                       "\t assign #" + to_string(NOT_DELAY) + "  out = ~in;\n"
                       "endmodule";    

    this->generate =  "NOT_GATE_" + to_string(this->n_bits) + "_BIT " + this->name + + " (" + this->inputs.at(0) + ", " + this->outputs.at(0) + ");";
}

/*------------------------------------------------------------------------------------------
                                    FULL ADDER CIRCUIT
                                    ==================

This is an example of how we can construct a Module by wiring together individual submodules.
------------------------------------------------------------------------------------------*/

FULL_ADDER::FULL_ADDER(string name, vector<wire> input_wires, vector<wire> output_wires){
    /*------------------------------------------------------------------------------------------
    First we set up inputs and output vectors. The Chip parent expects everything to be of type
    vector<string>, so we set up that accordingly. 

    In our case the input_wires will be {"A", "B", "Cin"}
    and the output_wires will be {"S", "Cout"}  
    ------------------------------------------------------------------------------------------*/
    this->name = name;
    this->inputs = input_wires;
    this->outputs = output_wires;


    /*------------------------------------------------------------------------------------------
    Now we declare input and output ports. Note that these are generic input ports and instance
    independent where as the previous 'input_wires' will be different for each instance. 
    ------------------------------------------------------------------------------------------*/
    this->declare("A", CHIP_INPUTS);
    this->declare("B", CHIP_INPUTS);
    this->declare("Cin", CHIP_INPUTS);
    this->declare("S", CHIP_OUTPUTS);
    this->declare("Cout", CHIP_OUTPUTS);

    /*------------------------------------------------------------------------------------------
    Now we can wire the circuit for S = A xor B xor C;
    For that we create 2 instances of XOR gates and a wire. 
    ------------------------------------------------------------------------------------------*/
    wire temp_xor_1 = "temp_xor_1";
    XOR xor_1("FA_XOR_1",
            {"A", "B"},
            temp_xor_1);
    XOR xor_2("FA_XOR_2",
            {"Cin", temp_xor_1},
            "S");

    /*------------------------------------------------------------------------------------------
    Then we register the wires and submodules so that the auto_gen function can automatically
    generate code.
    ------------------------------------------------------------------------------------------*/
    this->add_wire(temp_xor_1);
    this->add_submodule(xor_1);
    this->add_submodule(xor_2);

    /*------------------------------------------------------------------------------------------
    Here we wire Cout = AB + BC + AC.
    We need 3 and gates and 3 wires from it. 
    We also need 2 or gates and 1 wire for it. 
    ------------------------------------------------------------------------------------------*/
    wire temp_and_1 = "temp_and_1";
    wire temp_and_2 = "temp_and_2";
    wire temp_and_3 = "temp_and_3";
    wire temp_or_1 = "temp_or_1";

    AND and_1("FA_AND_1",
                   {"A", "B"},
                   temp_and_1);
    AND and_2("FA_AND_2",
                   {"B", "Cin"},
                   temp_and_2);
    AND and_3("FA_AND_3",
                   {"Cin", "A"},
                   temp_and_3);
    
    OR or_1("FA_OR_1",
                 {temp_and_1, temp_and_2},
                 temp_or_1);
    OR or_2("FA_OR_2",
                 {temp_or_1, temp_and_3},
                 "Cout");
    
    this->add_wire(temp_and_1);
    this->add_wire(temp_and_2);
    this->add_wire(temp_and_3);
    this->add_wire(temp_or_1);
    this->add_submodule(and_1);
    this->add_submodule(and_2);
    this->add_submodule(and_3);
    this->add_submodule(or_1);
    this->add_submodule(or_2);

    /*------------------------------------------------------------------------------------------
    After building the module we can use the auto_gen() funtion to automatically generate the m-
    odule definition.
    ------------------------------------------------------------------------------------------*/
    this->definition = this->auto_gen("module FULL_ADDER");

    /*------------------------------------------------------------------------------------------
    Then we can instruct how each instance has to be generated.
    ------------------------------------------------------------------------------------------*/
    this->generate =  "FULL_ADDER " + this->name  + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ", " + this->inputs.at(2) + ", " + this->outputs.at(0) + "," + this->outputs.at(1) + ");";
}


/*------------------------------------------------------------------------------------------
                                CARRY RIPPLE ADDER N BIT
                                ========================

Constructing the Carry Ripple adder by stacking full adders.
------------------------------------------------------------------------------------------*/

CARRY_RIPPLE_ADDER::CARRY_RIPPLE_ADDER(string name, vector<wire> input_wires, wire output_wire, int n_bits){
    /*------------------------------------------------------------------------------------------
    First we set up inputs and output vectors. The Chip parent expects everything to be of type
    vector<string>, so we set up that accordingly. 

    We also set up the bit width and name of the instance.    
    ------------------------------------------------------------------------------------------*/
    this->name = name;
    this->inputs = input_wires;
    this->outputs.push_back(output_wire);
    this->n_bits = n_bits;

    /*------------------------------------------------------------------------------------------
    Now we declare input and output ports. Note that these are generic input ports and instance
    independent where as the previous 'input_wires' will be different for each instance. 
    ------------------------------------------------------------------------------------------*/
    this->declare("A", CHIP_INPUTS);
    this->declare("B", CHIP_INPUTS);
    this->declare("out", CHIP_OUTPUTS);

    /*------------------------------------------------------------------------------------------
    We define a wire as carry wire. Note that we need to specify the bit width for wires only an
    d not input/output. This is because the bit width of input/output are expected to be equal to
    the bit width. 
    ------------------------------------------------------------------------------------------*/
    wire carry_wire = "CRA_carry_wire";
    this->add_wire("[" + to_string(n_bits) + ":0] " + carry_wire);

    /*------------------------------------------------------------------------------------------
    We need to assign the first bit of the carry wire to be zero. We use verilog("") function to
    embed verilog code inside our module.
    ------------------------------------------------------------------------------------------*/
    this->verilog("assign " + carry_wire + "[0] = 0;");

    /*------------------------------------------------------------------------------------------
    Now we configure wiring for each bits
    ------------------------------------------------------------------------------------------*/
    for(int i =0; i< n_bits; i++){
        string slice = "["+ to_string(i) + "]";
        string next_slice = "[" + to_string(i+1) + "]";

        FULL_ADDER fa("CRA_FA_" + to_string(i),
                      {"A" + slice, "B" + slice, carry_wire + slice},
                      {"out" + slice, carry_wire + next_slice});
        this->add_submodule(fa);
    }

    /*------------------------------------------------------------------------------------------
    After building the module we can use the auto_gen() funtion to automatically generate the m-
    odule definition.
    ------------------------------------------------------------------------------------------*/
    this->definition = this->auto_gen("module CARRY_RIPPLE_ADDER_" + to_string(this->n_bits) + "_BIT");

    /*------------------------------------------------------------------------------------------
    Then we can instruct how each instance has to be generated.
    ------------------------------------------------------------------------------------------*/
    this->generate =  "CARRY_RIPPLE_ADDER_" + to_string(this->n_bits) + "_BIT " + this->name  + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ", " + this->outputs.at(0) + ");";
}

/*------------------------------------------------------------------------------------------
                            CARRY LOOK AHEAD ADDER N BIT
                            ============================

This is an implimentation of the CLA adder.
------------------------------------------------------------------------------------------*/

CARRY_LOOK_AHEAD_ADDER::CARRY_LOOK_AHEAD_ADDER(string name, vector<wire> input_wires, wire output_wire, int n_bits){
    /*------------------------------------------------------------------------------------------
    We do the basic setups.
    ------------------------------------------------------------------------------------------*/
    this->name = name;
    this->inputs = input_wires;
    this->outputs.push_back(output_wire);
    this->n_bits = n_bits;

    /*------------------------------------------------------------------------------------------
    We declare input and output ports.
    ------------------------------------------------------------------------------------------*/
    this->declare("A", CHIP_INPUTS);
    this->declare("B", CHIP_INPUTS);
    this->declare("out", CHIP_OUTPUTS);

    /*------------------------------------------------------------------------------------------
    Find the number of levels for our Prefix computation
    ------------------------------------------------------------------------------------------*/
    int levels = ceil(log2(n_bits + 1));

    /*------------------------------------------------------------------------------------------
    Wire naming convention
    ======================
    State at level L bit N will be [1:0]s_level_L_bit_N

    We construct a matrix of wire names for easy representation.
    wire_matrix[L][N] points to the state at level L, bit N.
    We initialize the first state to be kill.
    ------------------------------------------------------------------------------------------*/
    wire always_zero_wire = "s_level_0_bit_0";
    this->add_wire("[1:0] " + always_zero_wire);
    this->verilog("assign " + always_zero_wire + " = 0;");

    wire wire_matrix[levels + 1][n_bits + 1];
    wire_matrix[0][0] = always_zero_wire;

    /*------------------------------------------------------------------------------------------
    We calculate the level 0 states and fill the first row of the matrix.
    ------------------------------------------------------------------------------------------*/
    for(int i=1; i<= n_bits; i++){
        string curr_slice = "[" + to_string(i - 1) + "]";
        
        wire current_wire = "s_level_0_bit_" + to_string(i);
        this->add_wire("[1:0] " + current_wire);
        AND and_pre("CLA_and_pre_bit_" + to_string(i),
                    {"A" + curr_slice, "B" + curr_slice},
                    current_wire + "[1]");
        OR or_pre("CLA_or_pre_bit_" + to_string(i),
                 {"A" + curr_slice, "B" + curr_slice},
                 current_wire + "[0]");
        wire_matrix[0][i] = current_wire;
        this->add_submodule(and_pre);
        this->add_submodule(or_pre);
    }
    
    /*------------------------------------------------------------------------------------------
    States are now stored in "[1: 0] s_level_0_bit_n".

    Now we make another wire to store final carry results. Note that the bit width of the carry 
    wire is one more than the bit width of the adder. The last bit is in carry_final is ignored
    as of now, but can help us know if there is any overflow.

    We also assign the 0th bit of carry_final to be zero.
    ------------------------------------------------------------------------------------------*/
    wire carry_final = "carry_final";
    this->add_wire("[" + to_string(n_bits) + ":0] " + carry_final);
    verilog("assign carry_final[0] = 0;");
    
    /*------------------------------------------------------------------------------------------
    Now we wire up the prefix computation
    ------------------------------------------------------------------------------------------*/
    for(int level=0; level<levels; level++){
        /*------------------------------------------------------------------------------------------
        In each level, ignore the part that already got computed and just drop down the states,
        compute the new states for others. done_bits is the number of bits from the end who is done.
        ------------------------------------------------------------------------------------------*/
        int done_bits = pow(2, level);
        for(int bit = 0; bit <= n_bits; bit++){
            /*------------------------------------------------------------------------------------------
            For each bit:
            ------------------------------------------------------------------------------------------*/
            if (bit < done_bits){
                /*------------------------------------------------------------------------------------------
                Just drop down the values for bits that are done
                ------------------------------------------------------------------------------------------*/
                wire_matrix[level + 1][bit] = wire_matrix[level][bit];
            }
            else{
                /*------------------------------------------------------------------------------------------
                Compute the new states and connect  with the new wires.

                First we add a new wire and put it in our wire_matrix. Then we do the star(*) operation.
                ------------------------------------------------------------------------------------------*/
                wire new_wire = "s_level_" + to_string(level + 1) + "_bit_" + to_string(bit);
                this->add_wire("[1:0] " + new_wire);
                wire_matrix[level + 1][bit] = new_wire;

                if(pow(2, level + 1) <= bit){
                    CLA_STAR star_operator("CLA_level_" + to_string(level+1) + "_bit_" + to_string(bit),
                                        {wire_matrix[level][bit - done_bits] + "[0]", wire_matrix[level][bit - done_bits] + "[1]", wire_matrix[level][bit] + "[0]", wire_matrix[level][bit] + "[1]"},
                                        {wire_matrix[level+1][bit] + "[0]", wire_matrix[level+1][bit] + "[1]"}
                                        );
                    this->add_submodule(star_operator);
                }
                else{
                    /*------------------------------------------------------------------------------------------
                    If no more computations are required for the current bit, we join the wire with carry_final[N]
                    ------------------------------------------------------------------------------------------*/
                    CLA_STAR star_operator("CLA_level_" + to_string(level+1) + "_bit_" + to_string(bit),
                                        {wire_matrix[level][bit - done_bits] + "[0]", wire_matrix[level][bit - done_bits] + "[1]", wire_matrix[level][bit] + "[0]", wire_matrix[level][bit] + "[1]"},
                                        {carry_final + "[" + to_string(bit) + "]", wire_matrix[level+1][bit] + "[1]"}
                                        );
                    JOIN joint("CLA_WIRE_JOINT_" + to_string(level+1) + "_bit_" + to_string(bit),
                              carry_final + "[" + to_string(bit) + "]",
                              wire_matrix[level+1][bit] + "[0]");
                    
                    this->add_submodule(joint);
                    this->add_submodule(star_operator);
                }
            }
        }
    }

    /*------------------------------------------------------------------------------------------
    Now we have everything, n bit A, n bit B, n bit carry_final. We do A xor B xor carry_final
    to get the sum.
    ------------------------------------------------------------------------------------------*/
    wire xor_temp_wire = "CLA_xor_temp_wire";
    this->add_wire("[" + to_string(n_bits -1) + ":0] " + xor_temp_wire);

    XOR_N_BIT xor_1("CLA_XOR_1",
                    {"A", "B"},
                    xor_temp_wire,
                    n_bits);

    XOR_N_BIT xor_2("CLA_XOR_2",
                    {xor_temp_wire, carry_final + "[" + to_string(n_bits-1) + ":0]" },
                    "out",
                    n_bits);
    
    this->add_submodule(xor_1);
    this->add_submodule(xor_2);

    /*------------------------------------------------------------------------------------------
    The wiring is complete, now we use auto_gen to automaically generate the definitions.
    ------------------------------------------------------------------------------------------*/
    this->definition = this->auto_gen("module CARRY_LOOK_AHEAD_ADDER_" + to_string(n_bits) + "_BIT");
    this->generate =  "CARRY_LOOK_AHEAD_ADDER_" + to_string(this->n_bits) + "_BIT " + this->name  + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ", " + this->outputs.at(0) + ");";
}

/*------------------------------------------------------------------------------------------
                                The star(*) operation
                                =====================
------------------------------------------------------------------------------------------*/

CLA_STAR::CLA_STAR(string name, vector<wire> input_wires, vector<wire> output_wires){
    /*------------------------------------------------------------------------------------------
    We do the basic setups.
    ------------------------------------------------------------------------------------------*/
    this->name = name;
    this->inputs = input_wires;
    this->outputs = output_wires;

    /*------------------------------------------------------------------------------------------
    We declare the inputs.
    Here we have 4 inputs and 2 outputs, 
        IN  - PS0 - Previous State[0]
        IN  - PS1 - Previous State[1]
        IN  - CS0 - Current State[0]
        IN  - CS1 - Current State[1]
        OUT - S0  - Next State[0]
        OUT - S1  - Next State[1]

    States:
        00  - Kill
        01  - Propagate
        11  - Generate
    ------------------------------------------------------------------------------------------*/
    this->declare("PS0", CHIP_INPUTS);
    this->declare("PS1", CHIP_INPUTS);
    this->declare("CS0", CHIP_INPUTS);
    this->declare("CS1", CHIP_INPUTS);
    this->declare("S0", CHIP_OUTPUTS);
    this->declare("S1", CHIP_OUTPUTS);

    /*------------------------------------------------------------------------------------------
    Now we do the wiring for,
        S0 = CS1 || (PS0 && CS0)
        S1 = CS1 || (PS1 && CS0)
    ------------------------------------------------------------------------------------------*/
    wire temp_and_0 = "temp_and_0";
    wire temp_and_1 = "temp_and_1";

    AND and_0("CLA_STAR_and_0",
             {"PS0", "CS0"},
             temp_and_0);
    OR or_0("CLA_STAR_or_0",
            {temp_and_0, "CS1"},
            "S0");
    
    AND and_1("CLA_STAR_and_1",
             {"PS1", "CS0"},
             temp_and_1);
    OR or_1("CLA_STAR_or_1",
           {temp_and_1, "CS1"},
           "S1");
    
    /*------------------------------------------------------------------------------------------
    We add these submodules and wires back to parent chip.
    ------------------------------------------------------------------------------------------*/
    this->add_wire(temp_and_0);
    this->add_wire(temp_and_1);
    this->add_submodule(and_0);
    this->add_submodule(and_1);
    this->add_submodule(or_0);
    this->add_submodule(or_1);
    
    /*------------------------------------------------------------------------------------------
    We use auto_gen() function to automatically generate module definintions
    ------------------------------------------------------------------------------------------*/
    this->definition = this->auto_gen("module CLA_STAR");
    this->generate =  "CLA_STAR " + this->name  + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ", " + this->inputs.at(2) + ", " + this->inputs.at(3) + ", " + this->outputs.at(0) + ", " + this->outputs.at(1) + ");";
}

/*------------------------------------------------------------------------------------------
                                Flipflops and Registors
                                =======================
------------------------------------------------------------------------------------------*/

FLIP_FLOP::FLIP_FLOP(string name, vector<wire> input_wires, wire output_wire){
    this->name = name;
    this->inputs = input_wires;
    this->outputs.push_back(output_wire);

    this->declare("A", CHIP_INPUTS);
    this->declare("CLK", CHIP_INPUTS);
    this->declare("OUT", CHIP_OUTPUTS_REG);

    BehaviourModel flipflop({"posedge CLK"});
    flipflop.add("OUT", "#" + to_string(FLIP_FLOP_DELAY) + " A", N_BLOCK);
    flipflop.num_transistors = FLIP_FLOP_TRANSISTORS; 

    this->add_submodule(flipflop);

    this->definition = this->auto_gen("module FLIP_FLOP");
    this->generate = "FLIP_FLOP " + this->name + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ", " + this->outputs.at(0) + " );";

}

FLIP_FLOP_N_BIT::FLIP_FLOP_N_BIT(string name, vector<wire> input_wires, wire output_wire, int n_bits){
    this->name = name;
    this->inputs = input_wires;
    this->outputs.push_back(output_wire);
    this->n_bits = n_bits;
    
    this->declare("A", CHIP_INPUTS);
    this->declare("CLK", CHIP_INPUTS, 1);
    this->declare("OUT", CHIP_OUTPUTS_REG);

    BehaviourModel flipflop({"posedge CLK"});
    flipflop.add("OUT", "#" + to_string(FLIP_FLOP_DELAY) + " A", N_BLOCK);
    flipflop.num_transistors = this->n_bits * FLIP_FLOP_TRANSISTORS;

    this->add_submodule(flipflop);

    this->definition = this->auto_gen("module FLIP_FLOP_" + to_string(n_bits) + "_BIT");
    this->generate = "FLIP_FLOP_" + to_string(n_bits) + "_BIT " + this->name + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ", " + this->outputs.at(0) + " );";

}

/*------------------------------------------------------------------------------------------
                        CARRY LOOK AHEAD ADDER WITH PIPELINING
                        ======================================

This is an implimentation of the CLA adder with pipelining.
------------------------------------------------------------------------------------------*/

CARRY_LOOK_AHEAD_ADDER_PIPELINED::CARRY_LOOK_AHEAD_ADDER_PIPELINED(string name, vector<wire> input_wires, wire output_wire, int n_bits, int pipeline_k){
    /*------------------------------------------------------------------------------------------
    We do the basic setups.
    ------------------------------------------------------------------------------------------*/
    this->name = name;
    this->inputs = input_wires;
    this->outputs.push_back(output_wire);
    this->n_bits = n_bits;

    /*------------------------------------------------------------------------------------------
    We declare input and output ports.
    ------------------------------------------------------------------------------------------*/
    this->declare("A", CHIP_INPUTS);
    this->declare("B", CHIP_INPUTS);
    this->declare("CLK", CHIP_INPUTS, 1);
    this->declare("out", CHIP_OUTPUTS);

    /*------------------------------------------------------------------------------------------
    Find the number of levels for our Prefix computation
    ------------------------------------------------------------------------------------------*/
    int levels = ceil(log2(n_bits + 1));

    /*------------------------------------------------------------------------------------------
    Wire naming convention
    ======================
    State at level L bit N will be [1:0]s_level_L_bit_N

    We construct a matrix of wire names for easy representation.
    wire_matrix[L][N] points to the state at level L, bit N.
    We initialize the first state to be kill.
    ------------------------------------------------------------------------------------------*/
    wire always_zero_wire = "s_level_0_bit_0";
    this->add_wire("[1:0] " + always_zero_wire);
    this->verilog("assign " + always_zero_wire + " = 0;");

    wire wire_matrix[levels + 1][n_bits + 1];
    wire_matrix[0][0] = always_zero_wire;

    /*------------------------------------------------------------------------------------------
    We calculate the level 0 states and fill the first row of the matrix.
    ------------------------------------------------------------------------------------------*/
    for(int i=1; i<= n_bits; i++){
        string curr_slice = "[" + to_string(i - 1) + "]";
        
        wire current_wire = "s_level_0_bit_" + to_string(i);
        this->add_wire("[1:0] " + current_wire);
        AND and_pre("CLA_and_pre_bit_" + to_string(i),
                    {"A" + curr_slice, "B" + curr_slice},
                    current_wire + "[1]");
        OR or_pre("CLA_or_pre_bit_" + to_string(i),
                 {"A" + curr_slice, "B" + curr_slice},
                 current_wire + "[0]");
        wire_matrix[0][i] = current_wire;
        this->add_submodule(and_pre);
        this->add_submodule(or_pre);
    }
    
    /*------------------------------------------------------------------------------------------
    States are now stored in "[1: 0] s_level_0_bit_n".

    Now we make another wire to store final carry results. Note that the bit width of the carry 
    wire is one more than the bit width of the adder. The last bit is in carry_final is ignored
    as of now, but can help us know if there is any overflow.

    We also assign the 0th bit of carry_final to be zero.
    ------------------------------------------------------------------------------------------*/
    wire carry_final = "carry_final";
    this->add_wire("[" + to_string(n_bits) + ":0] " + carry_final);
    verilog("assign carry_final[0] = 0;");
    
    /*------------------------------------------------------------------------------------------
    Now we wire up the prefix computation
    ------------------------------------------------------------------------------------------*/
    int num_stages = 0;
    for(int level=0; level<levels; level++){
        if(level>0 && (level%pipeline_k == 0)) num_stages++;
        /*------------------------------------------------------------------------------------------
        In each level, ignore the part that already got computed and just drop down the states,
        compute the new states for others. done_bits is the number of bits from the end who is done.
        ------------------------------------------------------------------------------------------*/
        int done_bits = pow(2, level);
        for(int bit = 0; bit <= n_bits; bit++){
            /*------------------------------------------------------------------------------------------
            For each bit:
            ------------------------------------------------------------------------------------------*/
            if (bit < done_bits){
                /*------------------------------------------------------------------------------------------
                Just drop down the values for bits that are done
                ------------------------------------------------------------------------------------------*/
                wire_matrix[level + 1][bit] = wire_matrix[level][bit];
            }
            else{
                /*------------------------------------------------------------------------------------------
                Compute the new states and connect  with the new wires.

                First we add a new wire and put it in our wire_matrix. Then we do the star(*) operation.
                ------------------------------------------------------------------------------------------*/
                wire new_wire = "s_level_" + to_string(level + 1) + "_bit_" + to_string(bit);
                this->add_wire("[1:0] " + new_wire);
                wire_matrix[level + 1][bit] = new_wire;

                CLA_STAR star_operator("CLA_level_" + to_string(level+1) + "_bit_" + to_string(bit),
                                    {wire_matrix[level][bit - done_bits] + "[0]", wire_matrix[level][bit - done_bits] + "[1]", wire_matrix[level][bit] + "[0]", wire_matrix[level][bit] + "[1]"},
                                    {wire_matrix[level+1][bit] + "[0]", wire_matrix[level+1][bit] + "[1]"}
                                    );
                this->add_submodule(star_operator);
            }

            /*------------------------------------------------------------------------------------------
            For each bit in current level if level is multiple of k, we add a registor
            ------------------------------------------------------------------------------------------*/
            
            if(level>0 && (level%pipeline_k == 0)){

                /*------------------------------------------------------------------------------------------
                We take the "wire_matrix[level +1][bit]" wire from wire matrix
                We send it to a flipflop
                ANd the we take the output of the flipflop and put it to the wire matrix
                ------------------------------------------------------------------------------------------*/
                
                wire flipflop_out = "CLA_STAR_PIPELINED_FLIPFLOP_LEVEL_" + to_string(level) + "_BIT_" + to_string(bit) + "_reg";
                FLIP_FLOP_N_BIT curr_flipflop("CLA_STAR_PIPELINED_FLIPFLOP_LEVEL_" + to_string(level) + "_BIT_" + to_string(bit),
                                             {wire_matrix[level +1][bit], "CLK"},
                                             flipflop_out,
                                             2);
                wire_matrix[level+1][bit] = flipflop_out;
                this->add_wire("[1:0] " + flipflop_out);
                this->add_submodule(curr_flipflop);
            }

            /*------------------------------------------------------------------------------------------
            Now we join th elast wire and the carry_final wire
            ------------------------------------------------------------------------------------------*/
            if(level + 1 == levels){
                JOIN joint("CLA_WIRE_JOINT_" + to_string(level+1) + "_bit_" + to_string(bit),
                            carry_final + "[" + to_string(bit) + "]",
                            wire_matrix[level+1][bit] + "[0]");
                this->add_submodule(joint);
            }

        }
    }
    /*------------------------------------------------------------------------------------------
    Now we have everything, n bit A, n bit B, n bit carry_final. We do A xor B xor carry_final
    to get the sum.
    ------------------------------------------------------------------------------------------*/
    wire xor_temp_wire = "CLA_xor_temp_wire";
    this->add_wire("[" + to_string(n_bits -1) + ":0] " + xor_temp_wire);


    XOR_N_BIT xor_1("CLA_XOR_1",
                    {"A", "B"},
                    xor_temp_wire,
                    n_bits);

    /*------------------------------------------------------------------------------------------
    We need to add a few more registers as empty stages to balance out stuffs.
    ------------------------------------------------------------------------------------------*/
    for(int k=0; k < num_stages; k++){
        wire curr_wire = "CLA_xor_temp_wire_" + to_string(k);
        FLIP_FLOP_N_BIT curr_flip("CLA_XOR_EMPTY_STAGE_" + to_string(k),
                                 {xor_temp_wire, "CLK"},
                                 curr_wire, n_bits);
        this->add_wire("[" + to_string(n_bits-1) + ":0] " + curr_wire);
        this->add_submodule(curr_flip);
        xor_temp_wire = curr_wire;
    }
    
    wire final_output = "CLA_PIPELINE_FINAL";
    XOR_N_BIT xor_2("CLA_XOR_2",
                    {xor_temp_wire, carry_final + "[" + to_string(n_bits-1) + ":0]" },
                    final_output,
                    n_bits);
    this->add_wire("["+ to_string(this->n_bits - 1) +":0] " +final_output);
    this->add_submodule(xor_1);
    this->add_submodule(xor_2);

    FLIP_FLOP_N_BIT last_flipflop("CLA_PIPELINED_OUT",
                                {final_output, "CLK"},
                                "out", n_bits);

    this->add_submodule(last_flipflop);
    /*------------------------------------------------------------------------------------------
    The wiring is complete, now we use auto_gen to automaically generate the definitions.
    ------------------------------------------------------------------------------------------*/
    this->definition = this->auto_gen("module CARRY_LOOK_AHEAD_ADDER_PIPELINED_" + to_string(n_bits) + "_BIT_" + to_string(pipeline_k) + "_PIPELINED");
    this->generate =  "CARRY_LOOK_AHEAD_ADDER_PIPELINED_" + to_string(n_bits) + "_BIT_" + to_string(pipeline_k) + "_PIPELINED " + this->name  + " (" + this->inputs.at(0) + ", " + this->inputs.at(1) + ", " + this->inputs.at(2) + ", " + this->outputs.at(0) + ");";
}

/*------------------------------------------------------------------------------------------
                                    CARRY SAVE ADDER
                                    ================
------------------------------------------------------------------------------------------*/

CARRY_SAVE_ADDER::CARRY_SAVE_ADDER(string name, vector<wire> input_wires, vector<wire> output_wires, int n_bits){
    /*------------------------------------------------------------------------------------------
    A Carry Save Adder is constructed by stacking Full Adders together.
    ------------------------------------------------------------------------------------------*/
    this->name = name;
    this->inputs = input_wires;
    this->outputs = output_wires;
    this->n_bits = n_bits;

    this->declare("A", CHIP_INPUTS);
    this->declare("B", CHIP_INPUTS);
    this->declare("Cin", CHIP_INPUTS);
    this->declare("SUM", CHIP_OUTPUTS);
    this->declare("Cout", CHIP_OUTPUTS);
    this->declare("Overflow", CHIP_OUTPUTS, 1);

    this->verilog("assign Cout[0] = 0;");
    for(int n=0; n<n_bits; n++){
        string slice = "[" + to_string(n) + "]";
        if(n + 1 != n_bits){
            FULL_ADDER curr_full_adder("CSA_FA_" + to_string(n),
                                    {"A" + slice, "B" + slice, "Cin" + slice},
                                    {"SUM" + slice, "Cout[" + to_string(n+1) + "]"});
            this->add_submodule(curr_full_adder);
        }
        else{
            FULL_ADDER curr_full_adder("CSA_FA_" + to_string(n),
                                    {"A" + slice, "B" + slice, "Cin" + slice},
                                    {"SUM" + slice, "Overflow"});
            
            this->add_submodule(curr_full_adder);
        }
    }

    this->definition = this->auto_gen("module CARRY_SAVE_ADDER_" + to_string(n_bits) + "_BIT");
    this->generate = "CARRY_SAVE_ADDER_" + to_string(n_bits) + "_BIT " + this->name + "(" + this->inputs.at(0) + ", " + this->inputs.at(1) + ", " + this->inputs.at(2) + ", " + this->outputs.at(0) + ", " + this->outputs.at(1) + ", " + this->outputs.at(2) + ");";

}