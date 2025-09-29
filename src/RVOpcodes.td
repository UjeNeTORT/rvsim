class RVOpcode<bits<7> val, string name> {
  bits<7> Val = val;
  string Name = name;
}

def OPC_OP     : RVOpcode<0b0110011, "OP">;
def OPC_OP_IMM : RVOpcode<0b0010011, "OP-IMM">;
def OPC_LUI    : RVOpcode<0b0110111, "LUI">;
def OPC_AUIPC  : RVOpcode<0b0010111, "AUIPC">;
def OPC_JAL    : RVOpcode<0b1101111, "JAL">;
def OPC_JALR   : RVOpcode<0b1100111, "JALR">;
def OPC_BRANCH : RVOpcode<0b1100011, "BRANCH">;
def OPC_LOAD   : RVOpcode<0b0000011, "LOAD">;
def OPC_STORE  : RVOpcode<0b0100011, "STORE">;
def OPC_SYSTEM : RVOpcode<0b1110011, "SYSTEM">;

// def OPC_MISC_MEM : RVOpcode<0b0001111, "MISC-MEM">;
