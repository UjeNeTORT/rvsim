class RVInsnRBase<string name, code execcode>
  : RVInsn<[ENC_FUNCT7,
            ENC_RS2,
            ENC_RS1,
            ENC_FUNCT3,
            ENC_RD,
            ENC_OPC],
            name, execcode> {
  let Encoding{31...25} = EncValues[0];
  let Type = "R";
}

class RVInsnR<list<int> encvalues, string name, code execcode>
  : RVInsnRBase<name, execcode> {
  let EncValues = encvalues;
}

class RVInsnIBase<string name, code execcode>
  : RVInsn<[ENC_IMM_11_0,
            ENC_RS1,
            ENC_FUNCT3,
            ENC_RD,
            ENC_OPC],
            name, execcode> {

  let Type = "I";
}

class RVInsnI<list<int> encvalues, string name, code execcode>
  : RVInsnIBase<name, execcode> {
  let EncValues = encvalues;
}

class RVInsnIShBase<string name, code execcode>
  : RVInsn<[ENC_IMM_11_5,
            ENC_SHAMT_4_0,
            ENC_RS1,
            ENC_FUNCT3,
            ENC_RD,
            ENC_OPC],
            name, execcode> {

  let Type = "ISh";
}

class RVInsnISh<list<int> encvalues, string name, code execcode>
  : RVInsnIShBase<name, execcode> {
  let EncValues = encvalues;
}

class RVInsnJBase<string name, code execcode>
  : RVInsn<[ENC_IMM_20,
            ENC_IMM_10_1,
            ENC_IMM_11,
            ENC_IMM_19_12,
            ENC_RD,
            ENC_OPC],
            name, execcode> {
  let Type = "J";
}

class RVInsnJ<list<int> encvalues, string name, code execcode>
  : RVInsnJBase<name, execcode> {
  let EncValues = encvalues;
}

class RVInsnJALRBase<string name, code execcode>
  : RVInsn<[ENC_IMM_11_0,
            ENC_RS1,
            ENC_FUNCT3,
            ENC_RD,
            ENC_OPC],
            name, execcode> {
  let Type = "JALR";
}

class RVInsnJALR<list<int> encvalues, string name, code execcode>
  : RVInsnJALRBase<name, execcode> {
  let EncValues = encvalues;
}

class RVInsnSystemBase<string name, code execcode>
  : RVInsn<[ENC_FUNCT12,
            ENC_RS1,
            ENC_FUNCT3,
            ENC_RD,
            ENC_OPC],
            name, execcode> {

  let Type = "SYSTEM";
}

class RVInsnSystem<list<int> encvalues, string name, code execcode>
  : RVInsnSystemBase<name, execcode> {
  let EncValues = encvalues;
}
