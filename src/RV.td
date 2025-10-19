include "RVInsn.td"
include "RVInsnTypes.td"
include "RVOpcodes.td"

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

 //                                  FUNCT3             OPCODE
def ADDI : RVInsnI<[?, ?, !cast<int>(0b000), ?, OPC_OP_IMM.Val], "addi">;
def SLTI : RVInsnI<[?, ?, !cast<int>(0b010), ?, OPC_OP_IMM.Val], "slti">;
def SLTIU : RVInsnI<[?, ?, !cast<int>(0b011), ?, OPC_OP_IMM.Val], "sltiu">;
def ANDI : RVInsnI<[?, ?, !cast<int>(0b111), ?, OPC_OP_IMM.Val], "andi">;
def ORI  : RVInsnI<[?, ?, !cast<int>(0b110), ?, OPC_OP_IMM.Val], "ori">;
def XORI : RVInsnI<[?, ?, !cast<int>(0b100), ?, OPC_OP_IMM.Val], "xori">;

def SLLI : RVInsnISh<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b001), ?, OPC_OP_IMM.Val], "slli">;
def SRLI : RVInsnISh<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b101), ?, OPC_OP_IMM.Val], "srli">;
def SRAI : RVInsnISh<[!cast<int>(0b0100000), ?, ?, !cast<int>(0b101), ?, OPC_OP_IMM.Val], "srai">;

def EBREAK : RVInsnSystem<[!cast<int>(0b000000000001), // funct12
                           !cast<int>(0b00000),        // rs1
                           !cast<int>(0b000),          // funct3
                           !cast<int>(0b00000),        // rd
                           OPC_SYSTEM.Val], "ebreak">;
