# BasicBlockExtraction
# Basic Block Extraction from Execution Trace Using Pintool, which was part of my research project over the summer
● Parse the execution trace into instruction addresses,
opcodes and operands
● Mark the boundary of each block as begin and end which
satisfy following rules
● Instruction is end of block if it is ret (return from a function
call) or a jmp instruction.
● Instruction is start of a new block if it is the address of the
instruction following jmp, same with ret

Advantage: Simple, easy to implement
Disadvantage: O(L^2) with L is the execution trace length, can be optimized to O(L) with a hash table

