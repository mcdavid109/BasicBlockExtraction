#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <map>
#include <stack>
#include <vector>
#include <set>
#include <stdlib.h>

using namespace std;

#include "core.h"

vector<Inst>inslist;

struct BasicBlock{
     vector<Inst>blockIns; //block instructions
};

string jmpInstrName[33] = {"jo","jno","js","jns","je","jz","jne",
                           "jnz","jb","jnae","jc","jnb","jae",
                           "jnc","jbe","jna","ja","jnbe","jl",
                           "jnge","jge","jnl","jle","jng","jg",
                           "jnle","jp","jpe","jnp","jpo","jcxz",
                           "jecxz", "jmp"};  //jmp instruction and ret

set<int> *jmpset;    // jmp instructions
map<string, int> *instenum; // instruction enumerations

//is the instruction a jump instruction
bool isjump(int i, set<int> *jumpset)
{
     set<int>::iterator it = jumpset->find(i);
     if (it == jumpset->end())
          return false;
     else
          return true;
}

//build a map of <opcode , id>
map<string, int> *buildOpcodeMap(vector<Inst> *L)
{
     map<string, int> *instenum = new map<string, int>;
     for(vector<Inst>::iterator it = L->begin() ; it != L->end() ; it++){
     	if(instenum->find(it->opcstr) == instenum->end()){
     	   instenum->insert(pair<string, int>(it->opcstr, instenum->size()+1));
        }
     }
     return instenum;
}
//return id of the opcode in the map
int getOpc(string s , map<string , int> *m)
{
     map<string , int>::iterator it = m->find(s);
     if (it != m->end()){
	     return it->second;
     }
     else {
	     return 0;
     }
}

void outputBlockTrace(vector<BasicBlock> *L)
{
      int n = 0;
      //iterate over basic block
      for(vector<BasicBlock>::iterator it = L->begin(); it != L->end(); ++it){
          vector<Inst>Instruction = it->blockIns;
          string blockfile = "block" + to_string(n++) + ".txt";
          FILE *fp = fopen(blockfile.c_str() , "w");
          //iterate over inst inside basic block
          for(vector<Inst>::iterator itr = Instruction.begin(); itr != Instruction.end(); ++itr){
              vector<string>operands = itr->oprs;
              //print out address of inst and opcode name
              fprintf(fp, "%s;%s;" , itr->addr.c_str() , itr->opcstr.c_str());
              //print out string of operands
              for(vector<string>::iterator ite = operands.begin() ; ite != operands.end(); ++ite){
                  fprintf(fp,"%s;", (*ite).c_str());
              }
              fprintf(fp, "\n");
          }
          fclose(fp);
      }
}


//parse the instruction trace
void parseTrace(ifstream *infile, vector<Inst> *L)
{
     string line;
     int num = 1;

     while (infile->good()) {
          getline(*infile, line);
          if (line.empty()) { continue; }

          istringstream strbuf(line);
          string temp, disasstr;

          Inst *ins = new Inst();
          ins->id = num++;

          // get the instruction address
          getline(strbuf, ins->addr, ';');
          ins->addrn = strtol(ins->addr.c_str(), NULL, 16);

          // get the disassemble string
          getline(strbuf, disasstr, ';');
          ins->assembly = disasstr;
          //get the opcode name
	       istringstream disasbuf(disasstr);
         getline(disasbuf, ins->opcstr, ' ');
	       //get operands
	       while(disasbuf.good()){
	          getline(disasbuf, temp, ',');
            if(temp.find_first_not_of(' ') != string::npos){
		            ins->oprs.push_back(temp);	
		          }
	       }

          ins->oprnum = ins->oprs.size();
	        L->push_back(*ins);
     }
}

void preprocess(vector<Inst> *L)
{
     // build global instruction enum based on the instlist
     instenum = buildOpcodeMap(L);

     // update opc field in L
     for (vector<Inst>::iterator it = L->begin(); it != L->end(); ++it) {
          it->opc = getOpc(it->opcstr, instenum);
     }

     // create a set containing the opcodes of all jump instructions
     jmpset = new set<int>;
     for (string &s : jmpInstrName) {
          int n;
          if ((n = getOpc(s, instenum)) != 0) {
               jmpset->insert(n);
          }
     }
}

void BBdetect(vector<Inst> *L)
{
      stack<vector<Inst>::iterator>stk;
      //vector of block end 
      vector<Inst>blockEnd;
      //vector of block body 
      vector<Inst>blockBody; 
      //basic block vector 
      vector<BasicBlock>BB;
      vector<Inst>::iterator inst;

      vector<Inst>::iterator it = L->begin();
      blockBody.push_back(*it);
      
      //scan through the execution trace
      for(it; it != L->end(); ++it){
        //if inst is jmp, it is end of block
        if(isjump(it->opc , jmpset)){
          blockEnd.push_back(*it);
          blockBody.push_back(*it);
          //find the inst with the jmp addr
          for(vector<Inst>::iterator itr = L->begin() ; itr != L->end() ; itr++){
              unsigned long targetaddr = strtol(it->oprs[0].c_str() , NULL , 16);
              if(itr->addrn == targetaddr){blockBody.push_back(*itr);}
            } 
        }
        //if inst is call, push into stack
        else if(it->opcstr == "call"){
            stk.push(it);
        }
        //if inst is ret, it is the end of a block
        else if(it->opcstr == "ret"){
            blockEnd.push_back(*it);
            blockBody.push_back(*it);
            //pop the call inst and mark it as new block
            if(!stk.empty()){
                inst = stk.top();
                inst = next(inst , 1);
            }
            blockBody.push_back(*inst);          
        }
        //if inst is normal, it will be bb body
        else{
            blockBody.push_back(*it);
        }
      
      }
      for(vector<Inst>::iterator tail = blockEnd.begin() ; tail != blockEnd.end() ; tail++){
          //new basic block
          BasicBlock *block = new BasicBlock();
          for(vector<Inst>::iterator body = blockBody.begin() ; body != blockBody.end() ; body++){
              vector<Inst>InsBlock = block->blockIns;
              InsBlock.push_back(*body);
              //if we reach block end, push the block
              //into block vector and break the loop
              if(compareInst(*body , *tail) == 1){
                  BB.push_back(*block);
                  break;  
              }
          }
      }

      outputBlockTrace(&BB);
}

int main(int argc , char **argv)
{
      if(argc != 2){
          fprintf(stderr , "usage: %s <tracefile>\n" , argv[0]);
          return 1;
      }

      ifstream infile(argv[1]);
      if(!infile.is_open()){
          fprintf(stderr , "Open file error!\n");
          return 1;
      }
      //parse execution trace
      parseTrace(&infile , &inslist);
      //close the file
      infile.close();
      //preprocess instruction list
      preprocess(&inslist);
      //generate basic block
      BBdetect(&inslist);

      return 0;
}

