class RVEncodingField<string enc_name> {
  int First;  // least significant field bit position
  int Last;   // most significat field bit position
  int Size = !add(!sub(Last, First), 1); // field length (sz [bits] = last - first + 1)
  // int Val;    // encoded value (bits)
  bit Encoded; // encoding field which determines the insn or (arg / some property) 

  string Name = enc_name; // field name
}

class RVEncF<int first, int last, string enc_name>
  : RVEncodingField<enc_name> {

  let First = first;
  let Last  = last;
  let Encoded = 0; // arguments hold no value that determines the insn
                   //   just its args, that can vary
}

class RVOpcode<int val, string type> : RVEncF<0, 6, "opcode"> {
  int Val = val;
  string Type = type;
}

multiclass RVOpcodes {
  def OP     : RVOpcode<0b0110011, "OP">;
  def OP_IMM : RVOpcode<0b0010011, "OP-IMM">;
  def LUI    : RVOpcode<0b0110111, "LUI">;
  def AUIPC  : RVOpcode<0b0010111, "AUIPC">;
  def JAL    : RVOpcode<0b1101111, "JAL">;
  def JALR   : RVOpcode<0b1100111, "JALR">;
  def BRANCH : RVOpcode<0b1100011, "BRANCH">;
  def LOAD   : RVOpcode<0b0000011, "LOAD">;
  def STORE  : RVOpcode<0b0100011, "STORE">;
  def SYSTEM : RVOpcode<0b1110011, "SYSTEM">;
}

defm OPC_ : RVOpcodes;

 // instruction encoding fields
def ENC_OPC       : RVEncF< 0,  6, "opcode">;
def ENC_RD        : RVEncF< 7, 11, "rd">;
def ENC_RS1       : RVEncF<15, 19, "rs1">;
def ENC_RS2       : RVEncF<20, 24, "rs2">;
def ENC_FUNCT3    : RVEncF<12, 14, "funct3">;
def ENC_FUNCT7    : RVEncF<25, 31, "funct7">;
def ENC_FUNCT12   : RVEncF<20, 31, "funct12">;
def ENC_IMM_11_0  : RVEncF<20, 31, "imm[11:0]">;
def ENC_IMM_11_5  : RVEncF<25, 31, "imm[11:5]">;
def ENC_SHAMT_4_0 : RVEncF<20, 24, "shamt[4:0]">;
def ENC_IMM_10_1  : RVEncF<21, 30, "imm[10:1]">;
def ENC_IMM_19_12 : RVEncF<12, 19, "imm[12:19]">;
def ENC_IMM_11    : RVEncF<20, 20, "imm[11]">;
def ENC_IMM_20    : RVEncF<31, 31, "imm[20]">;
