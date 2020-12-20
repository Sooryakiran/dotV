#include <iostream>
#include <verilog.h>
#include <stdio.h>
#include <fstream> 
#include <string.h>

using namespace std;

int generate_file(string file_name, string contents){
    /*------------------------------------------------------------------------------------------
    Writes/Rewrites the given contents to the given file. 

    Param : file_name (string), file_name of the output file.
    Param : contents (string), contents of the file to be written.

    Return : 1 if successful, 0 if not.
    ------------------------------------------------------------------------------------------*/
    ofstream fout; 
    fout.open(file_name);
    if(fout){
        fout << contents << endl; 
        fout.close();
        cout<<"[INFO] Code "<<file_name<<" written successfully!"<<endl;
    }
    else{
        cout<<"[ERROR] Unable to write file!"<<endl;
        return 0;
    } 
    return 1;
}

void invalid_args(char* name){
    /*------------------------------------------------------------------------------------------
    Displays the error message and usage format.
    ------------------------------------------------------------------------------------------*/
    cout<<"Invalid arguments\n  Usage "<<name<<" [-n num_bits] [-k pipeline_steps] \n  Example: "<<name<<" -n 64 -k 4\n";
}

int main(int argc, char * argv[]){
    /*------------------------------------------------------------------------------------------
    Input Validation
    ------------------------------------------------------------------------------------------*/
    if(argc != 5){
        invalid_args(argv[0]);
        return 0;
    }

    if(strcmp(argv[1], "-n") || strcmp(argv[3], "-k")){
        invalid_args(argv[0]);
        return 0;
    }

    string n_str = argv[2];
    string k_str = argv[4];
    int n, k;
    try{
        n = stoi(n_str);
        k = stoi(k_str);
    }
    catch(exception e){
        invalid_args(argv[0]);
        return 0;
    }

    /*------------------------------------------------------------------------------------------
    Create instances of the chips.
    ------------------------------------------------------------------------------------------*/
    WALLACE_TREE_MULTIPLIER_PIPELINED wtm("Sample Adder",
                                         {"input_1", "input_2", "clk"},
                                         "outputs",
                                         n, k);


    /*------------------------------------------------------------------------------------------
    Generate Verilog and write it to file. File save location ./generated_codes/
    ------------------------------------------------------------------------------------------*/
    generate_file("generated_codes/wtm_" + to_string(n) + "_bits_k_" + to_string(k) + ".v", wtm.generate_verilog());
   
    return 1;
}
