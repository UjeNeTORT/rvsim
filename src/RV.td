include "RVInsn.td"
include "RVInsnTypes.td"
include "RVOpcodes.td"

defvar RExecutePrefix =
  "Sim::Register Rs2 = static_cast<Sim::Register>(getOperand(0));\n\t\t"
  "Sim::Register Rs1 = static_cast<Sim::Register>(getOperand(1));\n\t\t"
  "Sim::Register Rd  = static_cast<Sim::Register>(getOperand(2));\n\t\t"
  "uint32_t Op1 = Model.getReg(Rs1);\n\t\t"
  "uint32_t Op2 = Model.getReg(Rs2);\n\t\t";

//                                FUNCT7 RS2 RS1           FUNCT3  RD      OPCODE | asmstr
def ADD  : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b000), ?, OPC_OP.Val], "add",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 + Op2);\n")>;
def SUB  : RVInsnR<[!cast<int>(0b0100000), ?, ?, !cast<int>(0b000), ?, OPC_OP.Val], "sub",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 - Op2);\n\t\t")>;
def SLL  : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b001), ?, OPC_OP.Val], "sll",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 << (Op2 & 0x1f));\n\t\t")>;
def SLT  : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b010), ?, OPC_OP.Val], "slt",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 << (Op2 & 0x1f));\n\t\t")>;
def SLTU : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b011), ?, OPC_OP.Val], "sltu",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 < Op2);\n\t\t")>;
def XOR  : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b100), ?, OPC_OP.Val], "xor",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 ^ Op2);\n\t\t")>;
def SRL  : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b101), ?, OPC_OP.Val], "srl",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 >> (Op2 & 0x1f));\n\t\t")>;
def SRA  : RVInsnR<[!cast<int>(0b0100000), ?, ?, !cast<int>(0b101), ?, OPC_OP.Val], "sra",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, std::bit_cast<int32_t>(Op1) >> (Op2 & 0x1f));\n\t\t")>;
def OR   : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b110), ?, OPC_OP.Val], "or",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 | Op2);\n\t\t")>;
def AND  : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b111), ?, OPC_OP.Val], "and",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 & Op2);\n\t\t")>;

defvar IExecutePrefix =
  "uint32_t Imm = getOperand(0);\n\t\t"
  "int32_t SImm = sign_extend_12_to_32(getOperand(0));\n\t\t"
  "Sim::Register Rs1 = static_cast<Sim::Register>(getOperand(1));\n\t\t"
  "Sim::Register Rd  = static_cast<Sim::Register>(getOperand(2));\n\t\t"
  "uint32_t Op1 = Model.getReg(Rs1);\n\t\t";

//                                   FUNCT3             OPCODE
def ADDI : RVInsnI<[?, ?, !cast<int>(0b000), ?, OPC_OP_IMM.Val], "addi",
  !strconcat(IExecutePrefix, "Model.setReg(Rd, Op1 + SImm);\n\t\t")>;
def SLTI : RVInsnI<[?, ?, !cast<int>(0b010), ?, OPC_OP_IMM.Val], "slti",
  !strconcat(IExecutePrefix, "Model.setReg(Rd, Op1 < std::bit_cast<int32_t>(SImm));\n\t\t")>;
def SLTIU : RVInsnI<[?, ?, !cast<int>(0b011), ?, OPC_OP_IMM.Val], "sltiu",
  !strconcat(IExecutePrefix, "Model.setReg(Rd, Op1 < std::bit_cast<uint32_t>(SImm));\n\t\t")>;
def ANDI : RVInsnI<[?, ?, !cast<int>(0b111), ?, OPC_OP_IMM.Val], "andi",
  !strconcat(IExecutePrefix, "Model.setReg(Rd, Op1 & SImm);\n\t\t")>;
def ORI  : RVInsnI<[?, ?, !cast<int>(0b110), ?, OPC_OP_IMM.Val], "ori",
  !strconcat(IExecutePrefix, "Model.setReg(Rd, Op1 | SImm);\n\t\t")>;
def XORI : RVInsnI<[?, ?, !cast<int>(0b100), ?, OPC_OP_IMM.Val], "xori",
  !strconcat(IExecutePrefix, "Model.setReg(Rd, (SImm == -1) ? ~Op1 : Op1 ^ SImm);\n\t\t")>;

def SLLI : RVInsnISh<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b001), ?, OPC_OP_IMM.Val], "slli",
  !strconcat(IExecutePrefix, "Model.setReg(Rd, Op1 << (Imm & 0x1f));\n\t\t")>;
def SRLI : RVInsnISh<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b101), ?, OPC_OP_IMM.Val], "srli",
  !strconcat(IExecutePrefix, "Model.setReg(Rd, Op1 >> (Imm & 0x1f));\n\t\t")>;
def SRAI : RVInsnISh<[!cast<int>(0b0100000), ?, ?, !cast<int>(0b101), ?, OPC_OP_IMM.Val], "srai",
  !strconcat(IExecutePrefix, "Model.setReg(Rd, std::bit_cast<int32_t>(Op1) >> (Imm & 0x1f));\n\t\t")>;

//                                             RD       OPCODE
def JAL  :    RVInsnJ<[?, ?, ?, ?,              ?, OPC_JAL.Val], "jal",
  "uint32_t CurrPC = Model.getPC();\n\t\t"
  "auto Args = getJalArgs(Operands_);\n\t\t"
  "uint32_t Offset = Args.first;\n\t\t"
  "uint32_t Rd  = Args.second;\n\t\t"
  "Model.setReg(static_cast<rv32i_sim::Register>(Rd), CurrPC);\n\t\t"
  "Model.setPC(CurrPC + sign_extend_21_to_32(Offset));\n\t\t">;

def JALR : RVInsnJALR<[?, ?, !cast<int>(0b000), ?, OPC_JALR.Val], "jalr",
  "int32_t SImm = sign_extend_12_to_32(getOperand(0));\n\t\t"
  "Sim::Register Rs1 = static_cast<Sim::Register>(getOperand(1));\n\t\t"
  "Sim::Register Rd  = static_cast<Sim::Register>(getOperand(2));\n\t\t"
  "uint32_t RetAddr = Model.getPC();\n\t\t"
  "Model.setReg(Rd, RetAddr); // + 4 happens on advance PC\n\t\t"
  "uint32_t JmpAddr = (Model.getReg(Rs1) + SImm) & ~1U; // clear LSB;\n\t\t"
  "Model.setPC(JmpAddr);\n\t\t">;

defvar BExecutePrefix =
  "auto Args = getBArgs(Operands_);\n\t\t"
  "int32_t SImm = sign_extend_13_to_32(std::get<0>(Args));\n\t\t"
  "Sim::Register Rs2 = static_cast<Sim::Register>(std::get<1>(Args));\n\t\t"
  "Sim::Register Rs1 = static_cast<Sim::Register>(std::get<2>(Args));\n\t\t"
  "uint32_t Op2 = Model.getReg(Rs2);\n\t\t"
  "uint32_t Op1 = Model.getReg(Rs1);\n\t\t";

def BEQ : RVInsnB<[?, ?, ?, ?, !cast<int>(0b000), ?, ?, OPC_BRANCH.Val], "beq",
  !strconcat(BExecutePrefix,
  "if (Op1 == Op2) Model.setPC(Model.getPC() + SImm - sizeof(uint32_t));\n\t\t")>;
def BNE : RVInsnB<[?, ?, ?, ?, !cast<int>(0b001), ?, ?, OPC_BRANCH.Val], "bne",
  !strconcat(BExecutePrefix,
  "if (Op1 != Op2) Model.setPC(Model.getPC() + SImm - sizeof(uint32_t));\n\t\t")>;
def BLT : RVInsnB<[?, ?, ?, ?, !cast<int>(0b100), ?, ?, OPC_BRANCH.Val], "blt",
  !strconcat(BExecutePrefix,
  "int32_t SOp1 = std::bit_cast<int32_t>(Op1);\n\t\t"
  "int32_t SOp2 = std::bit_cast<int32_t>(Op2);\n\t\t"
  "if (SOp1 < SOp2) Model.setPC(Model.getPC() + SImm - sizeof(uint32_t));\n\t\t")>;
def BGE : RVInsnB<[?, ?, ?, ?, !cast<int>(0b101), ?, ?, OPC_BRANCH.Val], "bge",
  !strconcat(BExecutePrefix,
  "int32_t SOp1 = std::bit_cast<int32_t>(Op1);\n\t\t"
  "int32_t SOp2 = std::bit_cast<int32_t>(Op2);\n\t\t"
  "if (SOp1 >= SOp2) Model.setPC(Model.getPC() + SImm - sizeof(uint32_t));\n\t\t")>;
def BLTU : RVInsnB<[?, ?, ?, ?, !cast<int>(0b110), ?, ?, OPC_BRANCH.Val], "bltu",
  !strconcat(BExecutePrefix,
  "if (Op1 < Op2) Model.setPC(Model.getPC() + SImm - sizeof(uint32_t));\n\t\t")>;
def BGEU : RVInsnB<[?, ?, ?, ?, !cast<int>(0b111), ?, ?, OPC_BRANCH.Val], "bgeu",
  !strconcat(BExecutePrefix,
  "if (Op1 >= Op2) Model.setPC(Model.getPC() + SImm - sizeof(uint32_t));\n\t\t")>;

defvar UExecutePrefix =
  "uint32_t Imm = getOperand(0);\n\t\t"
  "Sim::Register Rd = static_cast<Sim::Register>(getOperand(1));\n\t\t";

def LUI : RVInsnU<[?, ?, OPC_LUI.Val], "lui",
  !strconcat(UExecutePrefix, "Model.setReg(Rd, Imm);\n\t\t")>;
def AUIPC : RVInsnU<[?, ?, OPC_AUIPC.Val], "auipc",
  !strconcat(UExecutePrefix, "Model.setReg(Rd, Imm + Model.getPC());\n\t\t")>;

defvar LOADExecutePrefix =
  "uint32_t Imm = getOperand(0);\n\t\t"
  "Sim::Register Rs1 = static_cast<Sim::Register>(getOperand(1));\n\t\t"
  "Sim::Register Rd  = static_cast<Sim::Register>(getOperand(2));\n\t\t"
  "int32_t SImm = sign_extend_12_to_32(Imm);\n\t\t"
  "uint32_t MemAddr = Model.getReg(Rs1) + SImm;\n\t\t";

def LB : RVInsnLOAD<[?, ?, !cast<int>(0b000), ?, OPC_LOAD.Val], "lb",
  !strconcat(LOADExecutePrefix,
  "Model.setReg(Rd, sign_extend_8_to_32(Model.readByte(MemAddr)));")>;
def LH : RVInsnLOAD<[?, ?, !cast<int>(0b001), ?, OPC_LOAD.Val], "lh",
  !strconcat(LOADExecutePrefix,
  "Model.setReg(Rd, sign_extend_16_to_32(Model.readHalf(MemAddr)));")>;
def LW : RVInsnLOAD<[?, ?, !cast<int>(0b010), ?, OPC_LOAD.Val], "lw",
  !strconcat(LOADExecutePrefix,
  "Model.setReg(Rd, Model.readWord(MemAddr));")>;
def LBU : RVInsnLOAD<[?, ?, !cast<int>(0b100), ?, OPC_LOAD.Val], "lbu",
  !strconcat(LOADExecutePrefix,
  "Model.setReg(Rd, static_cast<uint32_t>(Model.readByte(MemAddr)));")>;
def LHU : RVInsnLOAD<[?, ?, !cast<int>(0b101), ?, OPC_LOAD.Val], "lhu",
  !strconcat(LOADExecutePrefix,
  "Model.setReg(Rd, static_cast<uint32_t>(Model.readHalf(MemAddr)));")>;

defvar STORExecutePrefix =
  "uint32_t Imm = getSTOREImm(Operands_);\n\t\t"
  "Sim::Register Rs2 = static_cast<Sim::Register>(getOperand(1));\n\t\t"
  "Sim::Register Rs1 = static_cast<Sim::Register>(getOperand(2));\n\t\t"
  "uint32_t Op1 = Model.getReg(Rs1);\n\t\t"
  "uint32_t Op2 = Model.getReg(Rs2);\n\t\t"
  "int32_t SImm = sign_extend_12_to_32(Imm);\n\t\t"
  "uint32_t MemAddr = Op1 + SImm;\n\t\t";

def SB : RVInsnSTORE<[?, ?, ?, !cast<int>(0b000), ?, OPC_STORE.Val], "sb",
  !strconcat(STORExecutePrefix,
  "Model.writeByte(MemAddr, static_cast<uint8_t>(Op2 & 0xff));")>;
def SH : RVInsnSTORE<[?, ?, ?, !cast<int>(0b001), ?, OPC_STORE.Val], "sh",
  !strconcat(STORExecutePrefix,
  "Model.writeByte(MemAddr, static_cast<uint16_t>(Op2 & 0xffff));")>;
def SW : RVInsnSTORE<[?, ?, ?, !cast<int>(0b010), ?, OPC_STORE.Val], "sw",
  !strconcat(STORExecutePrefix,
  "Model.writeByte(MemAddr, Op2 & 0xffffffff);")>;

def EBREAK : RVInsnSystem<[!cast<int>(0b000000000001), // funct12
                           !cast<int>(0b00000),        // rs1
                           !cast<int>(0b000),          // funct3
                           !cast<int>(0b00000),        // rd
                           OPC_SYSTEM.Val], "ebreak", "Model.exit();">;
