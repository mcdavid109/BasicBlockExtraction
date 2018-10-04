#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <map>
#include <stack>
#include <vector>
#include <set>
#include <string.h>

using namespace std;

struct Inst{
    int id;   //id of instruction
    string addr;  //instruction address
    unsigned long addrn;  //unsigned int of instruction address
    string assembly;//name of the instruction
    int opc; 
    string opcstr; //name of the opcode 
    vector<string>oprs; //vector of operands
    int oprnum; //number of operands
};

//compare two struct Inst
int compareInst(const struct Inst a , const struct Inst b){
	if(memcmp(&a , &b , sizeof(struct Inst)) == 0){
		return 1;
	}
	else{
		return 0;
	}
}


