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
