
include "RVOpcodes.td"

class Insn {
  string name;
}

class RVInsn<string namestr> : Insn {
  bits<32> encoding;
  let name = namestr;
}

class RVInsnRBase<RVOpcode Opcode, string name> : RVInsn<name> {
  bits<7> funct7;
  bits<5> rs2;
  bits<5> rs1;
  bits<3> funct3;
  bits<5> rd;
  bits<7> opcode = Opcode.Val;

  let encoding{31-25} = funct7;
  let encoding{24-20} = rs2;
  let encoding{19-15} = rs1;
  let encoding{14-12} = funct3;
  let encoding{11-7}  = rd;
  let encoding{6-0}   = opcode;
}

class RVInsnR<bits<7> funct7_, bits<3> funct3_, RVOpcode Opcode, string name> 
  : RVInsnRBase<Opcode, name> {
  let funct7 = funct7_; 
  let funct3 = funct3_; 
}


def ADD : RVInsnR<0b0000000, 0b000, OPC_OP, "add">;
def SUB : RVInsnR<0b0100000, 0b000, OPC_OP, "sub">;

