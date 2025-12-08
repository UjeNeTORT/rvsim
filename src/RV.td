include "RVInsn.td"
include "RVInsnTypes.td"
include "RVOpcodes.td"

defvar IncPC = "Model.setPC(Model.getPC() + sizeof(uint32_t));\n\t\t";

defvar RExecutePrefix =
  "Sim::Register Rs2 = static_cast<Sim::Register>(getOperand(0));\n\t\t"
  "Sim::Register Rs1 = static_cast<Sim::Register>(getOperand(1));\n\t\t"
  "Sim::Register Rd  = static_cast<Sim::Register>(getOperand(2));\n\t\t"
  "uint32_t Op1 = Model.getReg(Rs1);\n\t\t"
  "uint32_t Op2 = Model.getReg(Rs2);\n\t\t"
  "(void) Op1; (void) Op2; (void) Rd;\n\t\t";

defvar IExecutePrefix =
  "uint32_t Imm = getOperand(0);\n\t\t"
  "int32_t SImm = sign_extend_12_to_32(Imm);\n\t\t"
  "Sim::Register Rs1 = static_cast<Sim::Register>(getOperand(1));\n\t\t"
  "Sim::Register Rd  = static_cast<Sim::Register>(getOperand(2));\n\t\t"
  "uint32_t Op1 = Model.getReg(Rs1);\n\t\t"
  "(void)SImm; (void)Rd; (void)Op1;\n\t\t";

defvar UExecutePrefix =
  "uint32_t Imm = getOperand(0);\n\t\t"
  "uint32_t UImm = Imm << 12;\n\t\t"
  "Sim::Register Rd = static_cast<Sim::Register>(getOperand(1));\n\t\t"
  "(void)Imm; (void)UImm; (void) Rd;\n\t\t";

defvar BExecutePrefix =
  "auto Args = getBArgs(Operands_);\n\t\t"
  "int32_t SImm = sign_extend_13_to_32(std::get<0>(Args));\n\t\t"
  "Sim::Register Rs2 = static_cast<Sim::Register>(std::get<1>(Args));\n\t\t"
  "Sim::Register Rs1 = static_cast<Sim::Register>(std::get<2>(Args));\n\t\t"
  "uint32_t Op2 = Model.getReg(Rs2);\n\t\t"
  "uint32_t Op1 = Model.getReg(Rs1);\n\t\t"
  "(void) SImm; (void)Op1; (void)Op2;\n\t\t";

defvar LOADExecutePrefix =
  "uint32_t Imm = getOperand(0);\n\t\t"
  "Sim::Register Rs1 = static_cast<Sim::Register>(getOperand(1));\n\t\t"
  "Sim::Register Rd  = static_cast<Sim::Register>(getOperand(2));\n\t\t"
  "int32_t SImm = sign_extend_12_to_32(Imm);\n\t\t"
  "uint32_t MemAddr = Model.getReg(Rs1) + SImm;\n\t\t"
  "(void) Rd; (void)MemAddr;\n\t\t";

defvar STORExecutePrefix =
  "uint32_t Imm = getSTOREImm(Operands_);\n\t\t"
  "Sim::Register Rs2 = static_cast<Sim::Register>(getOperand(1));\n\t\t"
  "Sim::Register Rs1 = static_cast<Sim::Register>(getOperand(2));\n\t\t"
  "uint32_t Op1 = Model.getReg(Rs1);\n\t\t"
  "uint32_t Op2 = Model.getReg(Rs2);\n\t\t"
  "int32_t SImm = sign_extend_12_to_32(Imm);\n\t\t"
  "uint32_t MemAddr = Op1 + SImm;\n\t\t"
  "(void) MemAddr; (void) Op2;\n\t\t";

//                                FUNCT7 RS2 RS1           FUNCT3  RD      OPCODE | asmstr
def ADD  : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b000), ?, OPC_OP.Val], "add",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 + Op2);\n\t\t", IncPC)>;
def SUB  : RVInsnR<[!cast<int>(0b0100000), ?, ?, !cast<int>(0b000), ?, OPC_OP.Val], "sub",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 - Op2);\n\t\t", IncPC)>;
def SLL  : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b001), ?, OPC_OP.Val], "sll",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 << (Op2 & 0x1f));\n\t\t", IncPC)>;
def SLT  : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b010), ?, OPC_OP.Val], "slt",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, "
  "std::bit_cast<int32_t>(Op1) < std::bit_cast<int32_t>(Op2));\n\t\t", IncPC)>;
def SLTU : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b011), ?, OPC_OP.Val], "sltu",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 < Op2);\n\t\t", IncPC)>;
def XOR  : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b100), ?, OPC_OP.Val], "xor",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 ^ Op2);\n\t\t", IncPC)>;
def SRL  : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b101), ?, OPC_OP.Val], "srl",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 >> (Op2 & 0x1f));\n\t\t", IncPC)>;
def SRA  : RVInsnR<[!cast<int>(0b0100000), ?, ?, !cast<int>(0b101), ?, OPC_OP.Val], "sra",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, std::bit_cast<int32_t>(Op1) >> (Op2 & 0x1f));\n\t\t", IncPC)>;
def OR   : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b110), ?, OPC_OP.Val], "or",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 | Op2);\n\t\t", IncPC)>;
def AND  : RVInsnR<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b111), ?, OPC_OP.Val], "and",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 & Op2);\n\t\t", IncPC)>;

def MUL  : RVInsnR<[!cast<int>(0b0000001), ?, ?, !cast<int>(0b000), ?, OPC_OP.Val], "mul",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, (Op1 * Op2) & 0xffff'ffff);\n\t\t", IncPC)>;
def MULH : RVInsnR<[!cast<int>(0b0000001), ?, ?, !cast<int>(0b001), ?, OPC_OP.Val], "mulh",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, static_cast<uint32_t>("
                             "(static_cast<int64_t>(static_cast<int32_t>(Op1)) * "
                             "static_cast<int64_t>(static_cast<int32_t>(Op2))) >> 32) & 0xffff'ffff);\n\t\t", IncPC)>;
def MULHU: RVInsnR<[!cast<int>(0b0000001), ?, ?, !cast<int>(0b011), ?, OPC_OP.Val], "mulhu",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, ((static_cast<uint64_t>(Op1) * "
                             "static_cast<uint64_t>(Op2)) >> 32) & 0xffff'ffff);\n\t\t", IncPC)>;
def MULHSU: RVInsnR<[!cast<int>(0b0000001), ?, ?, !cast<int>(0b010), ?, OPC_OP.Val], "mulhsu",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, static_cast<uint32_t>("
                             "(static_cast<int64_t>(static_cast<int32_t>(Op1)) * "
                             "static_cast<uint64_t>(Op2)) >> 32) & 0xffff'ffff);\n\t\t", IncPC)>;
def DIV  : RVInsnR<[!cast<int>(0b0000001), ?, ?, !cast<int>(0b100), ?, OPC_OP.Val], "div",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op2 == 0 ? 0xFFFFFFFFu"
                             ": (static_cast<int32_t>(Op1) == INT32_MIN "
                             "&& static_cast<int32_t>(Op2) == -1)"
                             "? static_cast<uint32_t>(INT32_MIN)"
                             ": static_cast<uint32_t>(static_cast<int32_t>(Op1) "
                             "/ static_cast<int32_t>(Op2)));\n\t\t", IncPC)>;
def DIVU : RVInsnR<[!cast<int>(0b0000001), ?, ?, !cast<int>(0b101), ?, OPC_OP.Val], "divu",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op2 == 0 ? 0xffff'ffffu"
                             " : static_cast<uint32_t>(Op1) / static_cast<uint32_t>(Op2));\n\t\t", IncPC)>;
def REM  : RVInsnR<[!cast<int>(0b0000001), ?, ?, !cast<int>(0b110), ?, OPC_OP.Val], "rem",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op2 == 0 ? Op1 "
                             ": (static_cast<int32_t>(Op1) == INT32_MIN "
                             "&& static_cast<int32_t>(Op2) == -1) ? 0u "
                             ": static_cast<uint32_t>(static_cast<int32_t>(Op1) "
                             "% static_cast<int32_t>(Op2)));\n\t\t", IncPC)>;
def REMU : RVInsnR<[!cast<int>(0b0000001), ?, ?, !cast<int>(0b111), ?, OPC_OP.Val], "remu",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op2 == 0 ? Op1 : static_cast<uint32_t>(Op1 % Op2));\n\t\t", IncPC)>;

def ANDN : RVInsnR<[!cast<int>(0b0100000), ?, ?, !cast<int>(0b111), ?, OPC_OP.Val], "andn",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 & ~Op2);\n\t\t")>;
def ORN  : RVInsnR<[!cast<int>(0b0100000), ?, ?, !cast<int>(0b110), ?, OPC_OP.Val], "orn",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 | ~Op2);\n\t\t")>;
def XNOR : RVInsnR<[!cast<int>(0b0100000), ?, ?, !cast<int>(0b100), ?, OPC_OP.Val], "xnor",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, ~(Op1 ^ Op2));\n\t\t", IncPC)>;

def CLZ  : RVInsnR<[!cast<int>(0b0110000), !cast<int>(0b00000), ?, !cast<int>(0b001), ?, OPC_OP_IMM.Val], "clz",
  !strconcat(RExecutePrefix, "uint32_t Res = -1u;\n\t\t"
                             "for (int32_t Bit = 31; Bit >= 0; --Bit) {\n\t\t"
                             "  if (Op1 >> Bit & 1u) {\n\t\t"
                             "    Res = Bit;\n\t\t"
                             "    break;\n\t\t"
                             "  }\n\t\t"
                             "}\n\t\t"
                             "Model.setReg(Rd, 31 - Res); // 31 = xlen - 1\n\t\t", IncPC)>;

def CTZ  : RVInsnR<[!cast<int>(0b0110000), !cast<int>(0b00001), ?, !cast<int>(0b001), ?, OPC_OP_IMM.Val], "ctz",
  !strconcat(RExecutePrefix, "uint32_t Res = 32; // xlen\n\t\t"
                             "for (uint32_t Bit = 0; Bit < 32; --Bit) {\n\t\t"
                             "  if (Op1 >> Bit & 1u) {\n\t\t"
                             "    Res = Bit;\n\t\t"
                             "    break;\n\t\t"
                             "  }\n\t\t"
                             "}\n\t\t"
                             "Model.setReg(Rd, Res);\n\t\t", IncPC)>;

def CPOP : RVInsnR<[!cast<int>(0b0110000), !cast<int>(0b00010), ?, !cast<int>(0b001), ?, OPC_OP_IMM.Val], "cpop",
  !strconcat(RExecutePrefix, "uint32_t BitCount = 0;\n\t\t"
                             "for (uint32_t Bit = 0; Bit < 32; --Bit) {\n\t\t"
                             "  if (Op1 >> Bit & 1u)\n\t\t"
                             "    BitCount++;\n\t\t"
                             "}\n\t\t"
                             "Model.setReg(Rd, BitCount);\n\t\t", IncPC)>;

def MAX  : RVInsnR<[!cast<int>(0b0000101), ?, ?, !cast<int>(0b110), ?, OPC_OP.Val], "max",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, std::bit_cast<int32_t>(Op1) >= std::bit_cast<int32_t>(Op2) ? Op1 : Op2);\n\t\t", IncPC)>;
def MAXU : RVInsnR<[!cast<int>(0b0000101), ?, ?, !cast<int>(0b111), ?, OPC_OP.Val], "maxu",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 >= Op2 ? Op1 : Op2);\n\t\t", IncPC)>;
def MIN  : RVInsnR<[!cast<int>(0b0000101), ?, ?, !cast<int>(0b100), ?, OPC_OP.Val], "min",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, std::bit_cast<int32_t>(Op1) < std::bit_cast<int32_t>(Op2) ? Op1 : Op2);\n\t\t", IncPC)>;
def MINU : RVInsnR<[!cast<int>(0b0000101), ?, ?, !cast<int>(0b101), ?, OPC_OP.Val], "minu",
  !strconcat(RExecutePrefix, "Model.setReg(Rd, Op1 < Op2 ? Op1 : Op2);\n\t\t", IncPC)>;

def SEXT_B : RVInsnR<[!cast<int>(0b0110000), !cast<int>(0b00100), ?, !cast<int>(0b001), ?, OPC_OP_IMM.Val], "sext.b",
  !strconcat("Sim::Register Rs = static_cast<Sim::Register>(getOperand(0));\n\t\t"
             "Sim::Register Rd = static_cast<Sim::Register>(getOperand(1));\n\t\t"
             "uint32_t Op = Model.getReg(Rs);\n\t\t"
             "Model.setReg(Rd, sign_extend_8_to_32(Op));\n\t\t", IncPC)>;
def SEXT_H : RVInsnR<[!cast<int>(0b0110000), !cast<int>(0b00101), ?, !cast<int>(0b001), ?, OPC_OP_IMM.Val], "sext.h",
  !strconcat("Sim::Register Rs = static_cast<Sim::Register>(getOperand(0));\n\t\t"
             "Sim::Register Rd = static_cast<Sim::Register>(getOperand(1));\n\t\t"
             "uint32_t Op = Model.getReg(Rs);\n\t\t"
             "Model.setReg(Rd, sign_extend_16_to_32(Op));\n\t\t", IncPC)>;
def ZEXT_H : RVInsnR<[!cast<int>(0b0000100), !cast<int>(0b00000), ?, !cast<int>(0b100), ?, OPC_OP.Val], "zext.h",
  !strconcat("Sim::Register Rs = static_cast<Sim::Register>(getOperand(0));\n\t\t"
             "Sim::Register Rd = static_cast<Sim::Register>(getOperand(1));\n\t\t"
             "uint32_t Op = Model.getReg(Rs);\n\t\t"
             "Model.setReg(Rd, zero_extend_16_to_32(Op));\n\t\t", IncPC)>;

def ROL  : RVInsnR<[!cast<int>(0b0110000), ?, ?, !cast<int>(0b001), ?, OPC_OP.Val], "rol",
  !strconcat(RExecutePrefix, "uint32_t Shamt = Op2 & 0x1f;\n\t\t"
                             "Model.setReg(Rd, (Op1 << Shamt) | (Op1 >> (32 - Shamt)));\n\t\t", IncPC)>;
def ROR  : RVInsnR<[!cast<int>(0b0110000), ?, ?, !cast<int>(0b101), ?, OPC_OP.Val], "ror",
  !strconcat(RExecutePrefix, "uint32_t Shamt = Op2 & 0x1f;\n\t\t"
                             "Model.setReg(Rd, (Op1 >> Shamt) | (Op1 << (32 - Shamt)));\n\t\t", IncPC)>;
def RORI : RVInsnR<[!cast<int>(0b0110000), ?, ?, !cast<int>(0b101), ?, OPC_OP_IMM.Val], "rori",
  !strconcat(IExecutePrefix, "uint32_t Shamt = Imm & 0x1f;\n\t\t"
                             "Model.setReg(Rd, (Op1 >> Shamt) | (Op1 << (32 - Shamt)));\n\t\t", IncPC)>;

def ORC_B : RVInsnI<[!cast<int>(0b001010000111), ?, !cast<int>(0b101), ?, OPC_OP_IMM.Val], "orc.b",
  !strconcat(IExecutePrefix, "uint32_t Output = 0;\n\t\t"
                             "for (uint32_t i = 0; i < 32; i += 8) {\n\t\t"
                             "  Output |= ((Op1 >> i) & 0xff) ? (0xff << i)\n\t\t"
                             "                                : 0;\n\t\t"
                             "}\n\t\t"
                             "Model.setReg(Rd, Output);", IncPC)>;
def REV8 : RVInsnI<[!cast<int>(0b011010011000), ?, !cast<int>(0b101), ?, OPC_OP_IMM.Val], "rev8",
  !strconcat(IExecutePrefix, "uint32_t Output = 0;\n\t\t"
                             "Output |= (Op1 & 0xff << 0)  << 24;\n\t\t"
                             "Output |= (Op1 & 0xff << 8)  << 8;\n\t\t"
                             "Output |= (Op1 & 0xff << 16) >> 8;\n\t\t"
                             "Output |= (Op1 & 0xff << 24) >> 24;\n\t\t"
                             "Model.setReg(Rd, Output);", IncPC)>;

//                                   FUNCT3             OPCODE
def ADDI : RVInsnI<[?, ?, !cast<int>(0b000), ?, OPC_OP_IMM.Val], "addi",
  !strconcat(IExecutePrefix, "Model.setReg(Rd, Op1 + SImm);\n\t\t", IncPC)>;
def SLTI : RVInsnI<[?, ?, !cast<int>(0b010), ?, OPC_OP_IMM.Val], "slti",
  !strconcat(IExecutePrefix, "Model.setReg(Rd, std::bit_cast<int32_t>(Op1) < std::bit_cast<int32_t>(SImm));\n\t\t", IncPC)>;
def SLTIU : RVInsnI<[?, ?, !cast<int>(0b011), ?, OPC_OP_IMM.Val], "sltiu",
  !strconcat(IExecutePrefix, "Model.setReg(Rd, std::bit_cast<uint32_t>(Op1) < std::bit_cast<uint32_t>(SImm));\n\t\t", IncPC)>;
def ANDI : RVInsnI<[?, ?, !cast<int>(0b111), ?, OPC_OP_IMM.Val], "andi",
  !strconcat(IExecutePrefix, "Model.setReg(Rd, Op1 & SImm);\n\t\t", IncPC)>;
def ORI  : RVInsnI<[?, ?, !cast<int>(0b110), ?, OPC_OP_IMM.Val], "ori",
  !strconcat(IExecutePrefix, "Model.setReg(Rd, Op1 | SImm);\n\t\t", IncPC)>;
def XORI : RVInsnI<[?, ?, !cast<int>(0b100), ?, OPC_OP_IMM.Val], "xori",
  !strconcat(IExecutePrefix, "Model.setReg(Rd, (SImm == -1) ? ~Op1 : Op1 ^ SImm);\n\t\t", IncPC)>;

def SLLI : RVInsnISh<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b001), ?, OPC_OP_IMM.Val], "slli",
  !strconcat(IExecutePrefix, "Model.setReg(Rd, Op1 << (Imm & 0x1f));\n\t\t", IncPC)>;
def SRLI : RVInsnISh<[!cast<int>(0b0000000), ?, ?, !cast<int>(0b101), ?, OPC_OP_IMM.Val], "srli",
  !strconcat(IExecutePrefix, "Model.setReg(Rd, Op1 >> (Imm & 0x1f));\n\t\t", IncPC)>;
def SRAI : RVInsnISh<[!cast<int>(0b0100000), ?, ?, !cast<int>(0b101), ?, OPC_OP_IMM.Val], "srai",
  !strconcat(IExecutePrefix, "Model.setReg(Rd, std::bit_cast<int32_t>(Op1) >> (Imm & 0x1f));\n\t\t", IncPC)>;

//                                             RD       OPCODE
def JAL  :    RVInsnJ<[?, ?, ?, ?,              ?, OPC_JAL.Val], "jal",
  "uint32_t CurrPC = Model.getPC();\n\t\t"
  "auto Args = getJalArgs(Operands_);\n\t\t"
  "uint32_t Offset = Args.first;\n\t\t"
  "uint32_t Rd = Args.second;\n\t\t"
  "Model.setReg(static_cast<rv32i_sim::Register>(Rd), CurrPC + sizeof(uint32_t));\n\t\t"
  "Model.setPC(CurrPC + sign_extend_21_to_32(Offset));\n\t\t">;

def JALR : RVInsnJALR<[?, ?, !cast<int>(0b000), ?, OPC_JALR.Val], "jalr",
  "int32_t SImm = sign_extend_12_to_32(getOperand(0));\n\t\t"
  "Sim::Register Rs1 = static_cast<Sim::Register>(getOperand(1));\n\t\t"
  "Sim::Register Rd  = static_cast<Sim::Register>(getOperand(2));\n\t\t"
  "uint32_t t = Model.getPC() + sizeof(uint32_t);\n\t\t"
  "uint32_t JmpAddr = (Model.getReg(Rs1) + SImm) & ~1U; // clear LSB;\n\t\t"
  "Model.setPC(JmpAddr);\n\t\t"
  "Model.setReg(Rd, t);\n\t\t">;

def BEQ : RVInsnB<[?, ?, ?, ?, !cast<int>(0b000), ?, ?, OPC_BRANCH.Val], "beq",
  !strconcat(BExecutePrefix,
  "if (Op1 == Op2) Model.setPC(Model.getPC() + SImm);\n\t\t"
  "else ", IncPC)>;
def BNE : RVInsnB<[?, ?, ?, ?, !cast<int>(0b001), ?, ?, OPC_BRANCH.Val], "bne",
  !strconcat(BExecutePrefix,
  "if (Op1 != Op2) Model.setPC(Model.getPC() + SImm);\n\t\t"
  "else ", IncPC)>;
def BLT : RVInsnB<[?, ?, ?, ?, !cast<int>(0b100), ?, ?, OPC_BRANCH.Val], "blt",
  !strconcat(BExecutePrefix,
  "int32_t SOp1 = std::bit_cast<int32_t>(Op1);\n\t\t"
  "int32_t SOp2 = std::bit_cast<int32_t>(Op2);\n\t\t"
  "if (SOp1 < SOp2) Model.setPC(Model.getPC() + SImm);\n\t\t"
  "else ", IncPC)>;
def BGE : RVInsnB<[?, ?, ?, ?, !cast<int>(0b101), ?, ?, OPC_BRANCH.Val], "bge",
  !strconcat(BExecutePrefix,
  "int32_t SOp1 = std::bit_cast<int32_t>(Op1);\n\t\t"
  "int32_t SOp2 = std::bit_cast<int32_t>(Op2);\n\t\t"
  "if (SOp1 >= SOp2) Model.setPC(Model.getPC() + SImm);\n\t\t"
  "else ", IncPC)>;
def BLTU : RVInsnB<[?, ?, ?, ?, !cast<int>(0b110), ?, ?, OPC_BRANCH.Val], "bltu",
  !strconcat(BExecutePrefix,
  "if (Op1 < Op2) Model.setPC(Model.getPC() + SImm);\n\t\t"
  "else ", IncPC)>;
def BGEU : RVInsnB<[?, ?, ?, ?, !cast<int>(0b111), ?, ?, OPC_BRANCH.Val], "bgeu",
  !strconcat(BExecutePrefix,
  "if (Op1 >= Op2) Model.setPC(Model.getPC() + SImm);\n\t\t"
  "else ", IncPC)>;

def LUI : RVInsnU<[?, ?, OPC_LUI.Val], "lui",
  !strconcat(UExecutePrefix, "Model.setReg(Rd, UImm);\n\t\t", IncPC)>;
def AUIPC : RVInsnU<[?, ?, OPC_AUIPC.Val], "auipc",
  !strconcat(UExecutePrefix, "Model.setReg(Rd, UImm + Model.getPC());\n\t\t", IncPC)>;

def LB : RVInsnLOAD<[?, ?, !cast<int>(0b000), ?, OPC_LOAD.Val], "lb",
  !strconcat(LOADExecutePrefix,
  "Model.setReg(Rd, sign_extend_8_to_32(Model.readByte(MemAddr)));\n\t\t", IncPC)>;
def LH : RVInsnLOAD<[?, ?, !cast<int>(0b001), ?, OPC_LOAD.Val], "lh",
  !strconcat(LOADExecutePrefix,
  "Model.setReg(Rd, sign_extend_16_to_32(Model.readHalf(MemAddr)));\n\t\t", IncPC)>;
def LW : RVInsnLOAD<[?, ?, !cast<int>(0b010), ?, OPC_LOAD.Val], "lw",
  !strconcat(LOADExecutePrefix,
  "Model.setReg(Rd, Model.readWord(MemAddr));\n\t\t", IncPC)>;
def LBU : RVInsnLOAD<[?, ?, !cast<int>(0b100), ?, OPC_LOAD.Val], "lbu",
  !strconcat(LOADExecutePrefix,
  "Model.setReg(Rd, static_cast<uint32_t>(Model.readByte(MemAddr)));\n\t\t", IncPC)>;
def LHU : RVInsnLOAD<[?, ?, !cast<int>(0b101), ?, OPC_LOAD.Val], "lhu",
  !strconcat(LOADExecutePrefix,
  "Model.setReg(Rd, static_cast<uint32_t>(Model.readHalf(MemAddr)));\n\t\t", IncPC)>;

def SB : RVInsnSTORE<[?, ?, ?, !cast<int>(0b000), ?, OPC_STORE.Val], "sb",
  !strconcat(STORExecutePrefix,
  "Model.writeByte(MemAddr, static_cast<uint8_t>(Op2 & 0xff));\n\t\t", IncPC)>;
def SH : RVInsnSTORE<[?, ?, ?, !cast<int>(0b001), ?, OPC_STORE.Val], "sh",
  !strconcat(STORExecutePrefix,
  "Model.writeHalf(MemAddr, static_cast<uint16_t>(Op2 & 0xffff));\n\t\t", IncPC)>;
def SW : RVInsnSTORE<[?, ?, ?, !cast<int>(0b010), ?, OPC_STORE.Val], "sw",
  !strconcat(STORExecutePrefix,
  "Model.writeWord(MemAddr, Op2 & 0xffffffff);\n\t\t", IncPC)>;

def FLW : RVInsnLOAD<[?, ?, !cast<int>(0b010), ?, OPC_LD_FP.Val], "flw",
  !strconcat(LOADExecutePrefix, "Model.setReg(static_cast<Sim::FPRegister>(getOperand(2)), "
                                "std::bit_cast<float>(Model.readWord(MemAddr)));\n\t\t", IncPC)>;
def FSW : RVInsnSTORE<[?, ?, ?, !cast<int>(0b010), ?, OPC_ST_FP.Val], "fsw",
  !strconcat(STORExecutePrefix, "float FOp2 = Model.getReg(static_cast<Sim::FPRegister>(getOperand(2)));\n\t\t"
                                "Model.writeWord(MemAddr, std::bit_cast<uint32_t>(FOp2));\n\t\t", IncPC)>;

defvar FExecutePrefix = "auto Rs2 = static_cast<Sim::FPRegister>(getOperand(0));\n\t\t"
                        "auto Rs1 = static_cast<Sim::FPRegister>(getOperand(1));\n\t\t"
                        "auto Rd  = static_cast<Sim::FPRegister>(getOperand(2));\n\t\t"
                        "auto xRd = static_cast<Sim::Register>(getOperand(2));\n\t\t"
                        "float Op1 = Model.getReg(Rs1);\n\t\t"
                        "float Op2 = Model.getReg(Rs2);\n\t\t"
                        "(void) Rd; (void) xRd; (void) Op1; (void) Op2;\n\t\t";

def FADD : RVInsnF<[!cast<int>(0b00000), !cast<int>(0b00), ?, ?, !cast<int>(0b111), ?, OPC_FP.Val], "fadd.s",
  !strconcat(FExecutePrefix, "Model.setReg(Rd, Op1 + Op2);\n\t\t", IncPC)>;
def FSUB : RVInsnF<[!cast<int>(0b00001), !cast<int>(0b00), ?, ?, !cast<int>(0b111), ?, OPC_FP.Val], "fsub.s",
  !strconcat(FExecutePrefix, "Model.setReg(Rd, Op1 - Op2);\n\t\t", IncPC)>;
def FMUL : RVInsnF<[!cast<int>(0b00010), !cast<int>(0b00), ?, ?, !cast<int>(0b111), ?, OPC_FP.Val], "fmul.s",
  !strconcat(FExecutePrefix, "Model.setReg(Rd, Op1 * Op2);\n\t\t", IncPC)>;
def FDIV : RVInsnF<[!cast<int>(0b00011), !cast<int>(0b00), ?, ?, !cast<int>(0b111), ?, OPC_FP.Val], "fdiv.s",
  !strconcat(FExecutePrefix, "Model.setReg(Rd, Op1 / Op2);\n\t\t", IncPC)>;
def FMIN : RVInsnF<[!cast<int>(0b00101), !cast<int>(0b00), ?, ?, !cast<int>(0b000), ?, OPC_FP.Val], "fmin.s",
  !strconcat(FExecutePrefix, "Model.setReg(Rd, Op1 < Op2 ? Op1 : Op2);\n\t\t", IncPC)>;
def FMAX : RVInsnF<[!cast<int>(0b00101), !cast<int>(0b00), ?, ?, !cast<int>(0b001), ?, OPC_FP.Val], "fmax.s",
  !strconcat(FExecutePrefix, "Model.setReg(Rd, Op1 > Op2 ? Op1 : Op2);\n\t\t", IncPC)>;

def FSQRT : RVInsnF<[!cast<int>(0b01011), !cast<int>(0b00), !cast<int>(0b00000), ?, !cast<int>(0b111), ?, OPC_FP.Val], "fsqrt.s",
  !strconcat("float Op = Model.getReg(static_cast<Sim::FPRegister>(getOperand(0)));\n\t\t"
             "Sim::FPRegister FRd = static_cast<Sim::FPRegister>(getOperand(1));\n\t\t"
             "Model.setReg(FRd, std::sqrt(Op));\n\t\t", IncPC)>;
def FCLASS : RVInsnF<[!cast<int>(0b11100), !cast<int>(0b00), !cast<int>(0b00000), ?, !cast<int>(0b001), ?, OPC_FP.Val], "fclass.s",
  !strconcat("float Op = Model.getReg(static_cast<Sim::FPRegister>(getOperand(0)));\n\t\t"
             "Sim::Register Rd = static_cast<Sim::Register>(getOperand(1));\n\t\t"
             "Model.setReg(Rd, classifyS(Op));\n\t\t", IncPC)>;

def FMV_X_W : RVInsnF<[!cast<int>(0b11100), !cast<int>(0b00), !cast<int>(0b00000), ?, !cast<int>(0b000), ?, OPC_FP.Val], "fmv.x.w",
  !strconcat("float Op = Model.getReg(static_cast<Sim::FPRegister>(getOperand(0)));\n\t\t"
             "Sim::Register Rd = static_cast<Sim::Register>(getOperand(1));\n\t\t"
             "Model.setReg(Rd, std::bit_cast<uint32_t>(Op));\n\t\t", IncPC)>;
def FMV_W_X : RVInsnF<[!cast<int>(0b11110), !cast<int>(0b00), !cast<int>(0b00000), ?, !cast<int>(0b000), ?, OPC_FP.Val], "fmv.w.x",
  !strconcat("uint32_t Op = Model.getReg(static_cast<Sim::Register>(getOperand(0)));\n\t\t"
             "Sim::FPRegister Rd = static_cast<Sim::FPRegister>(getOperand(1));\n\t\t"
             "Model.setReg(Rd, std::bit_cast<float>(Op));\n\t\t", IncPC)>;

def FCMP_LE : RVInsnF<[!cast<int>(0b10100), !cast<int>(0b00), ?, ?, !cast<int>(0b000), ?, OPC_FP.Val], "fle.s",
  !strconcat(FExecutePrefix, "Model.setReg(xRd, Op1 <= Op2);\n\t\t", IncPC)>;
def FCMP_LT : RVInsnF<[!cast<int>(0b10100), !cast<int>(0b00), ?, ?, !cast<int>(0b001), ?, OPC_FP.Val], "flt.s",
  !strconcat(FExecutePrefix, "Model.setReg(xRd, Op1 < Op2);\n\t\t", IncPC)>;
def FCMP_EQ : RVInsnF<[!cast<int>(0b10100), !cast<int>(0b00), ?, ?, !cast<int>(0b010), ?, OPC_FP.Val], "feq.s",
  !strconcat(FExecutePrefix, "Model.setReg(xRd, Op1 == Op2);\n\t\t", IncPC)>;

def FSGNJ  : RVInsnF<[!cast<int>(0b0010), !cast<int>(0b00), ?, ?, !cast<int>(0b000), ?, OPC_FP.Val], "fsgnj.s",
  !strconcat(FExecutePrefix, "uint32_t SOp2 = std::bit_cast<uint32_t>(Op2) & (1 << 31);\n\t\t"
                             "uint32_t EMOp1 = std::bit_cast<uint32_t>(Op1) & ~(1 << 31);\n\t\t"
                             "Model.setReg(Rd, std::bit_cast<float>(SOp2 | EMOp1));\n\t\t", IncPC)>;
def FSGNJN : RVInsnF<[!cast<int>(0b0010), !cast<int>(0b00), ?, ?, !cast<int>(0b001), ?, OPC_FP.Val], "fsgnjn.s",
  !strconcat(FExecutePrefix, "uint32_t NotSOp2 = (std::bit_cast<uint32_t>(Op2) & (1 << 31)) ^ (1 << 31);\n\t\t"
                             "uint32_t EMOp1 = std::bit_cast<uint32_t>(Op1) & ~(1 << 31);\n\t\t"
                             "Model.setReg(Rd, std::bit_cast<float>(NotSOp2 | EMOp1));\n\t\t", IncPC)>;
def FSGNJX : RVInsnF<[!cast<int>(0b0010), !cast<int>(0b00), ?, ?, !cast<int>(0b010), ?, OPC_FP.Val], "fsgnjx.s",
  !strconcat(FExecutePrefix, "uint32_t SOp2 = std::bit_cast<uint32_t>(Op2) & (1 << 31);\n\t\t"
                             "uint32_t SOp1 = std::bit_cast<uint32_t>(Op1) & (1 << 31);\n\t\t"
                             "uint32_t EMOp1 = std::bit_cast<uint32_t>(Op1) & ~(1 << 31);\n\t\t"
                             "Model.setReg(Rd, std::bit_cast<float>((SOp2 ^ SOp1) | EMOp1));\n\t\t", IncPC)>;

defvar F4ExecutePrefix = "auto Rs3 = static_cast<Sim::FPRegister>(getOperand(0));\n\t\t"
                         "auto Rs2 = static_cast<Sim::FPRegister>(getOperand(1));\n\t\t"
                         "auto Rs1 = static_cast<Sim::FPRegister>(getOperand(2));\n\t\t"
                         "auto Rd  = static_cast<Sim::FPRegister>(getOperand(3));\n\t\t"
                         "auto xRd = static_cast<Sim::Register>(getOperand(3));\n\t\t"
                         "float Op1 = Model.getReg(Rs1);\n\t\t"
                         "float Op2 = Model.getReg(Rs2);\n\t\t"
                         "float Op3 = Model.getReg(Rs3);\n\t\t"
                         "(void) Rd; (void) xRd; (void) Op1; (void) Op2; (void) Op3;\n\t\t";

def FMADD : RVInsnF<[?, !cast<int>(0b00), ?, ?, !cast<int>(0b111), ?, OPC_FMADD.Val], "fmadd.s",
  !strconcat(F4ExecutePrefix, "Model.setReg(Rd, Op1 * Op2 + Op3);\n\t\t", IncPC)>;
def FMSUB : RVInsnF<[?, !cast<int>(0b00), ?, ?, !cast<int>(0b111), ?, OPC_FMSUB.Val], "fmsub.s",
  !strconcat(F4ExecutePrefix, "Model.setReg(Rd, Op1 * Op2 - Op3);\n\t\t", IncPC)>;
def FNMADD : RVInsnF<[?, !cast<int>(0b00), ?, ?, !cast<int>(0b111), ?, OPC_FNMADD.Val], "fnmadd.s",
  !strconcat(F4ExecutePrefix, "Model.setReg(Rd, -Op1 * Op2 - Op3);\n\t\t", IncPC)>;
def FNMSUB : RVInsnF<[?, !cast<int>(0b00), ?, ?, !cast<int>(0b111), ?, OPC_FNMSUB.Val], "fnmsub.s",
  !strconcat(F4ExecutePrefix, "Model.setReg(Rd, -Op1 * Op2 + Op3);\n\t\t", IncPC)>;

def FCVT_W_S  : RVInsnF<[!cast<int>(0b11000), !cast<int>(0b00), !cast<int>(0b00000), ?, !cast<int>(0b111), ?, OPC_FP.Val], "fcvt.w.s",
  !strconcat("float Op = Model.getReg(static_cast<Sim::FPRegister>(getOperand(0)));\n\t\t"
             "Sim::Register Rd = static_cast<Sim::Register>(getOperand(1));\n\t\t"
             "Model.setReg(Rd, static_cast<int32_t>(Op));\n\t\t", IncPC)>;
def FCVT_WU_S : RVInsnF<[!cast<int>(0b11000), !cast<int>(0b00), !cast<int>(0b00001), ?, !cast<int>(0b111), ?, OPC_FP.Val], "fcvt.wu.s",
  !strconcat("float Op = Model.getReg(static_cast<Sim::FPRegister>(getOperand(0)));\n\t\t"
             "Sim::Register Rd = static_cast<Sim::Register>(getOperand(1));\n\t\t"
             "Model.setReg(Rd, static_cast<uint32_t>(Op));\n\t\t", IncPC)>;
def FCVT_S_W  : RVInsnF<[!cast<int>(0b11010), !cast<int>(0b00), !cast<int>(0b00000), ?, !cast<int>(0b111), ?, OPC_FP.Val], "fcvt.s.w",
  !strconcat("int32_t Op = Model.getReg(static_cast<Sim::Register>(getOperand(0)));\n\t\t"
             "Sim::Register Rd = static_cast<Sim::Register>(getOperand(1));\n\t\t"
             "Model.setReg(Rd, static_cast<float>(Op));\n\t\t", IncPC)>;
def FCVT_S_WU : RVInsnF<[!cast<int>(0b11010), !cast<int>(0b00), !cast<int>(0b00001), ?, !cast<int>(0b111), ?, OPC_FP.Val], "fcvt.s.wu",
  !strconcat("uint32_t Op = Model.getReg(static_cast<Sim::Register>(getOperand(0)));\n\t\t"
             "Sim::Register Rd = static_cast<Sim::Register>(getOperand(1));\n\t\t"
             "Model.setReg(Rd, static_cast<float>(Op));\n\t\t", IncPC)>;

def EBREAK : RVInsnI<[!cast<int>(0b000000000001), // funct12
                      !cast<int>(0b00000),        // rs1
                      !cast<int>(0b000),          // funct3
                      !cast<int>(0b00000),        // rd
                       OPC_SYSTEM.Val], "ebreak", "Model.exit();">;

def ECALL : RVInsnI<[!cast<int>(0b000000000000), // funct12
                     !cast<int>(0b00000),        // rs1
                     !cast<int>(0b000),          // funct3
                     !cast<int>(0b00000),        // rd
                     OPC_SYSTEM.Val], "ecall", "Model.envCall();">;
