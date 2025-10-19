class RVInsnRBase<string name>
  : RVInsn<[ENC_FUNCT7,
            ENC_RS2,
            ENC_RS1,
            ENC_FUNCT3,
            ENC_RD,
            ENC_OPC],
            name> {
  let Encoding{31...25} = EncValues[0];
  let Type = "R";
}

class RVInsnR<list<int> encvalues, string name> : RVInsnRBase<name> {
  let EncValues = encvalues;
}

class RVInsnIBase<string name>
  : RVInsn<[ENC_IMM_11_0,
            ENC_RS1,
            ENC_FUNCT3,
            ENC_RD,
            ENC_OPC],
            name> {

  let Type = "I";
}

class RVInsnI<list<int> encvalues, string name> : RVInsnIBase<name> {
  let EncValues = encvalues;
}

class RVInsnIShBase<string name>
  : RVInsn<[ENC_IMM_11_5,
            ENC_SHAMT_4_0,
            ENC_RS1,
            ENC_FUNCT3,
            ENC_RD,
            ENC_OPC],
            name> {

  let Type = "ISh";
}

class RVInsnISh<list<int> encvalues, string name> : RVInsnIShBase<name> {
  let EncValues = encvalues;
}

class RVInsnSystemBase<string name>
  : RVInsn<[ENC_FUNCT12,
            ENC_RS1,
            ENC_FUNCT3,
            ENC_RD,
            ENC_OPC],
            name> {

  let Type = "SYSTEM";
}

class RVInsnSystem<list<int> encvalues, string name> : RVInsnSystemBase<name> {
  let EncValues = encvalues;
}
