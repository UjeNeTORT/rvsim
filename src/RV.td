
include "RVOpcodes.td"
include "RVEncoding.td"

class Insn {
  string Name;
}

class RVInsn<list<RVEncodingField> encfields, string name> : Insn {
  bits<32> Encoding;
  list<RVEncodingField> EncFields = encfields;
  list<int> EncValues;
  let Name = name;
  string Namespace = "RV";
  string Type = "UNTYPED";
}

class RVInsnRBase<string name>
  : RVInsn<[ENC_FUNCT7, ENC_RS2, ENC_RS1, ENC_FUNCT3, ENC_RD, ENC_OPC], name> {
  let Type = "R";
}

class RVInsnR<list<int> encvalues, string name> : RVInsnRBase<name> {
    let EncValues = encvalues;
}

 //                               FUNCT7 RS2 RS1           FUNCT3  RD      OPCODE | asmstr
def ADD  : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b000), ?, OPC_OP.Val], "add">;
def SUB  : RVInsnR<[!cast<int>(0b0100000), ?, ?, !cast<int>(0b000), ?, OPC_OP.Val], "sub">;
def SLL  : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b001), ?, OPC_OP.Val], "sll">;
def SLT  : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b010), ?, OPC_OP.Val], "slt">;
def SLTU : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b011), ?, OPC_OP.Val], "sltu">;
def XOR  : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b100), ?, OPC_OP.Val], "xor">;
def SRL  : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b101), ?, OPC_OP.Val], "srl">;
def SRA  : RVInsnR<[!cast<int>(0b0100000), ?, ?, !cast<int>(0b101), ?, OPC_OP.Val], "sra">;
def OR   : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b110), ?, OPC_OP.Val], "or">;
def AND  : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b111), ?, OPC_OP.Val], "and">;
