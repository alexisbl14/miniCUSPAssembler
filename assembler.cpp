#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <algorithm>

using namespace std;

string base10_to_hexstring(int val){
    string result = "";
    while(val != 0){
        int rem = val%16;
        if(rem >= 0 && rem <= 9){
            result += (char)(rem + '0');
        }
        else{
            result += (char)(rem - 10 + 'A');
        }
        val = val/16;
    }
    while(result.length() < 3){
        result += '0';
    }
    reverse(result.begin(), result.end());
    return result;
}

int main(int argc, char *argv[]){
    ifstream input_file;
    input_file.open(argv[1]);

    string line;
    map<string, string> label_map;
    int location_counter = 0;
    string location, label;

    //pass one to make label map
    while(getline(input_file, line)){
        if(islower(line[0])){
            label = line;
            label.pop_back();
            if (label[label.length()-1] == ':'){
                label.pop_back();
            }
            location = base10_to_hexstring(location_counter);
            if (label_map.count(label) > 0){
                std::cerr << "ASSEMBLER ERROR: label defined multiple times." << endl;
                exit(1);
            }
            label_map.insert(make_pair(label, location));
        }
        else{
            location_counter += 1;
        }
    }
    input_file.close();

    //making maps for instructions, supported addressing modes, addressing modes representation

    map<string, string> instr_rep = {{"LDA", "00"}, {"LDX", "01"}, {"LDS", "02"}, {"LDF", "03"}, 
                                    {"STA", "04"}, {"STX", "05"}, {"STS", "06"}, {"STF", "07"}, 
                                    {"PSH", "08"},{"POP", "09"}, {"CLR", "0A"}, {"SET", "0B"}, 
                                    {"PSHA", "FFF010"}, {"PSHX", "FFF011"}, {"PSHF", "FFF012"}, 
                                    {"POPA", "FFF013"}, {"POPX", "FFF014"}, {"POPF", "FFF015"},
                                    {"ADA", "10"}, {"ADX", "11"}, {"ADS", "12"}, {"ADF", "13"},
                                    {"SBA", "14"}, {"SBX", "15"}, {"SBS", "16"}, {"SBF", "17"},
                                    {"MUL", "18"}, {"DIV", "19"}, {"MOD", "1A"}, {"CMA", "20"},
                                    {"CMX", "21"}, {"CMS", "22"}, {"CMF", "23"}, {"SHRA", "FFF022"},
                                    {"SHLA", "FFF023"}, {"JSR", "41"}, {"RTN", "FFF040"}, {"JEQ", "48"},
                                    {"JLT", "4A"}, {"JGE", "4B"}, {"NOP", "FFF038"}, {"HLT", "FFFFFF"}};
    
    map<string, string> instr_addressing = {{"LDA", "1"}, {"LDX", "1"}, {"LDS", "1"}, {"LDF", "1"}, 
                                            {"STA", "2"}, {"STX", "2"}, {"STS", "2"}, {"STF", "2"}, 
                                            {"PSH", "1"},{"POP", "2"}, {"CLR", "2"}, {"SET", "2"}, 
                                            {"PSHA", "0"}, {"PSHX", "0"}, {"PSHF", "0"}, 
                                            {"POPA", "0"}, {"POPX", "0"}, {"POPF", "0"},
                                            {"ADA", "1"}, {"ADX", "1"}, {"ADS", "1"}, {"ADF", "1"},
                                            {"SBA", "1"}, {"SBX", "1"}, {"SBS", "1"}, {"SBF", "1"},
                                            {"MUL", "1"}, {"DIV", "1"}, {"MOD", "1"}, {"CMA", "1"},
                                            {"CMX", "1"}, {"CMS", "1"}, {"CMF", "1"}, {"SHRA", "0"},
                                            {"SHLA", "0"}, {"JSR", "2"}, {"RTN", "0"}, {"JEQ", "2"},
                                            {"JLT", "2"}, {"JGE", "2"}, {"NOP", "0"}, {"HLT", "0"}};
    
    map<string, string> addressing_modes = {{"#", "0"}, {" ", "2"}, {"+", "4"}, {"*", "6"}, {"&", "8"},
                                            {"#!", "1"}, {"!", "3"}, {"+!", "5"}, {"*!", "7"}, {"&!", "9"}};
    
    //starting pass 2, opening file
    input_file.open(argv[1]);
    ofstream output_file;
    output_file.open(argv[2]);
    string type, value, second, third, fourth;
    string output;
    string op_code, operand, addr_mode;
    string label_op, addr_chars;
    string constant;
    string instr_addr;


    while(getline(input_file, line)){  // try out second nested for loop for iterating through the line
        stringstream word(line);
        word >> type;
        if(isupper(type[0])){
            //must be an instruction
            op_code = instr_rep.find(type)->second;
            word >> second;
            if(second == ""){                 //looks like HLT, is an operate
                output = instr_rep.find(type)->second;
                output_file << output << endl;
            }
            else if(islower(second[0])){     //looks like LDA label4
                label_op = second;              //change variable name to keep track
                addr_mode = addressing_modes.find(" ")->second;
                if(label_map.count(label_op) == 0){
                    cerr << "ASSEMBLER ERROR: undefined label." << endl;
                    exit(1);
                }
                operand = label_map.find(label_op)->second;
                output = op_code + addr_mode + operand;
                output_file << output << endl;
            }
            else if(isdigit(second[0])){   //looks like LDA 19, have to convert 19 into hex
                constant = second;
                constant = base10_to_hexstring(stoi(constant));
                addr_mode = addressing_modes.find(" ")->second;
                output = op_code + addr_mode + constant;
                output_file << output << endl;
            }
            else if(second[0] == '$'){    //looks like LDA $19
                constant = second;
                constant.erase(0,1); //remove the $ sign
                while(constant.length() < 3){
                    constant = '0' + constant;
                }
                addr_mode = addressing_modes.find(" ")->second;
                output = op_code + addr_mode + constant;
                output_file << output << endl;
            }
            else if(second[0] == '#' || second[0] == '+' || second[0] == '*' || second[0] == '&' || second[0] == '!'){
                addr_chars = second;
                addr_mode = addressing_modes.find(addr_chars)->second;
                instr_addr = instr_addressing.find(type)->second;
                if(instr_addr == "2" && (addr_mode == "0" || addr_mode == "1")){
                    cerr << "ASSEMBLER ERROR: instruction using unsupported addressing mode." << endl;
                    exit(1);
                }
                word >> third;
                if(islower(third[0])){  //looks like LDA # label
                    label_op = third;
                    if(label_map.count(label_op) == 0){
                        cerr << "ASSEMBLER ERROR: undefined label." << endl;
                        exit(1);
                    }
                    operand = label_map.find(label_op)->second;
                    output = op_code + addr_mode + operand;
                    output_file << output << endl;
                }
                else if(isdigit(third[0])){  //looks like LDA # 19
                    constant = third;
                    constant = base10_to_hexstring(stoi(constant));
                    output = op_code + addr_mode + constant;
                    output_file << output << endl;
                }
                else if(third[0] == '$'){   //looks like LDA # $19
                    constant = third; 
                    constant.erase(0,1); //removing the $
                    while(constant.length() < 3){
                        constant = '0' + constant;
                    }
                    output = op_code + addr_mode + constant;
                    output_file << output << endl;
                }
                else if (third[0] == '!'){  //next char is part of addressing mode
                    addr_chars += third;
                    addr_mode = addressing_modes.find(addr_chars)->second;
                    instr_addr = instr_addressing.find(type)->second;
                    if(instr_addr == "2" && (addr_mode == "0" || addr_mode == "1")){
                        cerr << "ASSEMBLER ERROR: instruction using unsupported addressing mode." << endl;
                        exit(1);
                    }
                    word >> fourth;
                    if (islower(fourth[0])){  //looks like LDA # ! label
                        label_op = fourth;
                        if(label_map.count(label_op) == 0){
                            cerr << "ASSEMBLER ERROR: undefined label." << endl;
                            exit(1);
                        }
                        operand = label_map.find(label_op)->second;
                        output = op_code + addr_mode + operand;
                        output_file << output << endl;
                    }
                    else if(isdigit(fourth[0])){  //looks like LDA # ! 19
                        constant = fourth;
                        constant = base10_to_hexstring(stoi(constant));
                        output = op_code + addr_mode + constant;
                        output_file << output << endl;
                    }
                    else if(fourth[0] == '$'){   //looks like LDA # ! $19
                        constant = fourth;
                        constant.erase(0,1); //removing the $
                        while(constant.length() < 3){
                            constant = '0' + constant;
                        }
                        output = op_code + addr_mode + constant;
                        output_file << output << endl;
                    }
                }

            }          
        }
        else if(type[0] == '.'){  //looks like .word 53, is a directive
            word >> value;       // gets the number, ie 53
            output = value;
            reverse(output.begin(), output.end());
            while(output.length() != 6){
                output += '0';
            }
            reverse(output.begin(), output.end());
            output_file << output << endl;        
        }
        second = "";   //reset value of second
    }

    input_file.close();
    output_file.close();
 
}