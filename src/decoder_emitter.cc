#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/TableGen/TableGenBackend.h"
#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/Main.h"
#include "llvm/TableGen/Error.h"
#include "llvm/Support/raw_ostream.h"

#include <bit>
#include <bitset>
#include <set>
#include <ranges>

using namespace llvm;

namespace {
/**
 * Overengineering, since i am still uncertain what info should be stored in an
 * encoded value.
 * For now these two classes just store single uint32_t value.
 * */
class IEncodingFieldValue {
public:
  virtual uint32_t getEncodedNumber() const;
  virtual void setEncodedNumber(uint32_t Value);
  virtual void print(raw_ostream &Out) const;
  virtual ~IEncodingFieldValue();
};

class RVEncodingFieldValue final : IEncodingFieldValue {
  uint32_t Value_;
public:
  uint32_t getEncodedNumber() const override { return Value_; }
  void setEncodedNumber(uint32_t Value) override { Value_ = Value; }

  void print(raw_ostream& Out) const override {
    Out << Value_;
    return;
  }
};

class EncodingField final {
  uint32_t First_; // field's LSB bit position
  uint32_t Last_;  // field's MSB bit position
  int Size_;
  bool IsEncoded_ = false; // determines whether the field is a real encoding field
  std::optional<uint32_t> Value_ = std::nullopt;

  std::string Name_ = "???";
public:
  EncodingField(uint32_t First, uint32_t Last, std::string Name)
    : First_(First), Last_(Last), Size_(Last_ - First + 1),
      IsEncoded_(false), Value_(std::nullopt), Name_(Name) {}
  EncodingField(uint32_t First, uint32_t Last, uint32_t Value, std::string Name)
    : First_(First), Last_(Last), Size_(Last_ - First + 1),
      IsEncoded_(true), Value_(Value), Name_(Name) {}

  uint32_t getSize() const { return Size_; }
  std::optional<uint32_t> getValue() const { return Value_; }

  // is a necessary part of encoding, which determines the instruction
  bool isEncoded() const { return IsEncoded_; }
  bool isOperand() const { return !IsEncoded_; }

  uint32_t getLSBPos() const { return First_; }
  uint32_t getMSBPos() const { return Last_; }

  std::string getName() const { return Name_; }
};
}

namespace {
class InstructionInfo final {
  uint32_t RawEncoding_;
  uint32_t TypeMask_;

  std::vector<EncodingField> EncodingFields_;

  // Operand mask is has bits set from LSB to MSB, others are zeroed
  using OpndMaskTy = std::pair<uint32_t, uint32_t>;
  // operand masks for each operand from LSB to MSB
  std::vector<std::pair<OpndMaskTy, std::string>> OpndMasks_;

  std::string Name_;
  std::string Type_;
  std::string AsmStr_;
  std::string ExecuteCode_; // todo add to ctor

public:
  InstructionInfo(uint32_t RawEncoding, uint32_t TypeMask,
                  std::vector<EncodingField> EncodingFields,
                  std::string Name, std::string Type, std::string AsmStr)
    : RawEncoding_(RawEncoding), TypeMask_(TypeMask), EncodingFields_(EncodingFields),
      Name_(Name), Type_(Type), AsmStr_(AsmStr) {

    for (const auto &EF: EncodingFields_) {
      if (!EF.isOperand()) continue;

      OpndMasks_.push_back(std::pair<OpndMaskTy, std::string>(
          OpndMaskTy(EF.getLSBPos(), EF.getMSBPos()),
          EF.getName()
        )
      );
    }
  }

  uint32_t getTypeMask() const { return TypeMask_; }
  uint32_t getRawEncoding() const { return RawEncoding_; }
  std::string getName() const { return Name_; }
  std::string getType() const { return Type_; }

  uint32_t getOperandMaskLSB(uint32_t OpIdx) const noexcept {
    return OpndMasks_[OpIdx].first.first;
  }

  std::string getOperandName(uint32_t OpIdx) const noexcept {
    return OpndMasks_[OpIdx].second;
  }

  uint32_t getOperandMask(uint32_t OpIdx) const noexcept {
    uint32_t LSB = OpndMasks_[OpIdx].first.first;
    uint32_t MSB = OpndMasks_[OpIdx].first.second;
    uint32_t Mask = (1 << (MSB - LSB + 1)) - 1;
    return Mask << LSB;
  }

  uint32_t nOperands() const noexcept { return OpndMasks_.size(); }

  void emitClass(raw_ostream &OS) const {
    OS << "class " << Name_ << " : public IInsn {\n";
    OS << "\t" << "const uint32_t RawEncoding_ = " << RawEncoding_ << "; // "
                << "0b" << std::bitset<32>(RawEncoding_).to_string() << "\n";
    OS << "\t" << "const uint32_t TypeMask_ = " << TypeMask_ << "; // "
                << "0b" << std::bitset<32>(TypeMask_).to_string() << "\n";
    OS << "\t" << "uint32_t Opcode_ = 0; // fully encoded instruction\n";
    OS << "\t" << "const std::string AsmStr_ = \"" << AsmStr_ << "\";\n";

    OS << "\t" << "std::vector<std::pair<uint32_t, std::string>> Operands_;\n\n";

    OS << "public:\n";

    // constructors
    OS << "\t" << Name_ << "() = delete;\n";
    OS << "\t" << Name_ << "(uint32_t Opcode) : Opcode_(Opcode) {}\n\n";

    // opcode
    OS << "\t" << "uint32_t getOpcode() const override { return Opcode_; }\n";

    // type
    OS << "\t" << "RVInsnTypes getType() const override {\n"
       << "\t\t" << "return RVInsnTypes::" << Type_ << "_TYPE_INSN; }\n\n";

    // operand functions
    OS << "\t" << "// returns index of the pushed operand\n";
    OS << "\t" << "uint32_t addOperand(uint32_t OpVal, std::string Name) override {\n"
       << "\t\t" << "Operands_.push_back(std::pair<uint32_t, std::string>(OpVal, Name));\n"
       << "\t\t" << "return Operands_.size() - 1;\n"
       << "\t" << "}\n\n";
    OS << "\t" << "uint32_t getOperand(uint32_t OpIdx) const override {\n"
       << "\t\t" << "return Operands_[OpIdx].first;\n"
       << "\t" << "}\n\n";
    OS << "\t" << "uint32_t nOperands() const override {\n"
       << "\t\t" << "return Operands_.size();\n"
       << "\t" << "}\n\n";

    // execute
    OS << "\t" << "void execute(IRVModel &Model) const override {\n"
       << "\t" << ExecuteCode_ << '\n'
       << "\t}\n\n";

    // print
    OS << "\t" << "void print(std::ostream &Out) const {\n"
       << "\t\t" << "Out << std::bitset<32>(Opcode_).to_string() << AsmStr_ << \"("
                 << Type_ << ")\";\n";
    OS << "\t""}\n\n";

    // destructor
    OS << "\t" << "~" << Name_ << "() = default;\n";
    OS << "};\n";


    return;
  }
};
}

namespace {

class DecoderEmitter final {
  const RecordKeeper &RK_;

  ListInit *getEncodingFields(const Record *InsnDef) const;
  ListInit *getEncodingValues(const Record *InsnDef) const;
  uint32_t  formEncodingFields(const Record * const Def,
                               std::vector<EncodingField> &EncFields,
                               uint32_t &RawEncoding) const;

  static void emitDecoderFunc(raw_ostream &OS,
                              const std::vector<InstructionInfo> &InsnInfos);
  static void emitTypesEnum(raw_ostream &OS,
                            const std::vector<InstructionInfo> &InsnInfos);
public:
  DecoderEmitter(const RecordKeeper &RK) : RK_(RK) {}

  void run(raw_ostream &OS);
  void dump() const;
};
}

ListInit *DecoderEmitter::getEncodingFields(const Record *InsnDef) const {
  if (!InsnDef) return nullptr;

  const RecordVal *EncF = InsnDef->getValue("EncFields");
  if (!EncF) {
    PrintFatalError(InsnDef->getLoc(),
                    Twine("list<RVEncodingField> EncFields must be present in ")
                    .concat(InsnDef->getName())
                    .concat(" instruction def!"));
    return nullptr;
  }

  const ListRecTy *ListTy = dyn_cast<ListRecTy>(EncF->getType());
  if (!ListTy) {
    PrintError(InsnDef->getLoc(),
              "EncFields in RiscV instruction must be of type list<RVEncodingField>!");
    return nullptr;
  }

  const RecTy *EncFTy = ListTy->getElementType();
  if (EncFTy->getAsString() != "RVEncodingField") {
    PrintError(InsnDef->getLoc(),
              "EncFields in RiscV instruction must be of type list<RVEncodingField> "
              "(received: " + EncFTy->getAsString() + ")");
    return nullptr;
  }

  Init *EncFInit = EncF->getValue();

  return dyn_cast<ListInit>(EncFInit);
}

ListInit *DecoderEmitter::getEncodingValues(const Record *InsnDef) const {
  if (!InsnDef) return nullptr;

  const RecordVal *EncV = InsnDef->getValue("EncValues");
  if (!EncV) {
    PrintFatalError(InsnDef->getLoc(),
                    "list<int> EncValues must be present in "
                    + InsnDef->getName()
                    + " instruction def!");
    return nullptr;
  }

  const ListRecTy *ListTy = dyn_cast<ListRecTy>(EncV->getType());
  if (!ListTy) {
    PrintError(InsnDef->getLoc(),
              "EncValues in RiscV instruction must be of type list<int>!");
    return nullptr;
  }

  const RecTy *EncVTy = ListTy->getElementType();
  if (EncVTy->getAsString() != "int") {
    PrintError(InsnDef->getLoc(),
              "EncValues in RiscV instruction must be of type list<int> "
              "(received: " + EncVTy->getAsString() + ")");
    return nullptr;
  }

  Init *EncVInit = EncV->getValue();

  return dyn_cast<ListInit>(EncVInit);
}

uint32_t DecoderEmitter::formEncodingFields(const Record * const Def,
                                            std::vector<EncodingField> &EncFields,
                                            uint32_t &RawEncoding) const {
  assert(EncFields.empty());

  ListInit *TGEncodingFields = getEncodingFields(Def);
  ListInit *TGEncodingValues = getEncodingValues(Def);
  if (!TGEncodingFields) {
    PrintError(Def->getLoc(), "EncFields is not a ListInit!");
    return 0;
  }

  if (!TGEncodingValues) {
    PrintError(Def->getLoc(), "EncValues is not a ListInit!");
    return 0;
  }

  auto TGEncodingFieldInit = TGEncodingFields->begin();
  auto TGEncodingValueInit = TGEncodingValues->begin();

  uint32_t EncodingMask = 0;

  while (TGEncodingFieldInit != TGEncodingFields->end() &&
         TGEncodingValueInit != TGEncodingValues->end()) {

    std::string EncName = (*TGEncodingFieldInit)->getAsString();

    DefInit *TGEncodingField = dyn_cast<DefInit>(*TGEncodingFieldInit);
    if (!TGEncodingField || !TGEncodingField->getDef()->isSubClassOf("RVEncodingField")) {
      PrintError(Def->getLoc(), "Encoding must be of type RVEncodingField");
      return 0;
    }

    // todo unsafe
    uint32_t MSBPos = dyn_cast<IntInit>(
      TGEncodingField->getDef()->getValue("Last")->getValue()
    )->getValue();

    uint32_t LSBPos = dyn_cast<IntInit>(
      TGEncodingField->getDef()->getValue("First")->getValue()
    )->getValue();

    // todo add more rules to skip encoding part
    if (!(*TGEncodingValueInit)->isComplete()) {
      EncFields.push_back(EncodingField(LSBPos, MSBPos, EncName));
      TGEncodingFieldInit++;
      TGEncodingValueInit++;
      continue;
    }

    IntInit *TGEncodingValue = dyn_cast<IntInit>(*TGEncodingValueInit);
    uint32_t EncValCode = TGEncodingValue->getValue();

    RawEncoding |= EncValCode << LSBPos;

    EncodingMask |= ((1 << (MSBPos - LSBPos + 1)) - 1) << LSBPos;
    EncFields.push_back(EncodingField(LSBPos, MSBPos, EncValCode, EncName));

    TGEncodingFieldInit++;
    TGEncodingValueInit++;
  }

  return EncodingMask;
}

void DecoderEmitter::emitDecoderFunc(raw_ostream &OS,
                                     const std::vector<InstructionInfo> &InsnInfos) {
  OS << "std::unique_ptr<IInsn> decode(uint32_t Opcode) {\n";
  for (auto &II : InsnInfos) {
    OS << "\t""if (uint32_t RawOpcode = "
                        "Opcode & 0b" << std::bitset<32>(II.getTypeMask()).to_string() << ") {\n"
       << "\t\t""if (RawOpcode == 0b" << std::bitset<32>(II.getRawEncoding()).to_string() << ") {\n"
       << "\t\t\t""std::unique_ptr<IInsn>Insn(new " << II.getName() << "(Opcode));\n";
      for (uint32_t OpIdx = 0; OpIdx != II.nOperands(); ++OpIdx)
        OS << "\t\t\t""Insn->addOperand(Opcode & " << II.getOperandMask(OpIdx)
                                        << " >> " << II.getOperandMaskLSB(OpIdx) << ", "
                                        << "\"" << II.getOperandName(OpIdx) << "\""
           << ");\n";
    OS << "\t\t\t""return Insn;\n"
      << "\t\t""}\n"
      << "\t""}\n";
  }

  OS << "\t""std::cerr << \"Fatal - failed to decode [\" << Opcode << \"]\";\n";
  OS << "\t""return nullptr;\n";
  OS << "} // decode()\n";
  return;
}

void DecoderEmitter::emitTypesEnum(raw_ostream &OS,
                                   const std::vector<InstructionInfo> &InsnInfos) {
  std::set<std::string> InsnTypes;
  OS << "enum class RVInsnTypes : uint32_t {\n";
  OS << "\t""UNDEF_TYPE_INSN = 0,\n";
  for (auto &II: InsnInfos) {
    std::string ITy = II.getType();
    if (InsnTypes.emplace(ITy).second) OS << "\t" << ITy << "_TYPE_INSN" << ",\n";
  }
  OS << "};\n\n";
  return;
}
// todo надо заамендить коммит в котором додебаживается decoder.inc (undefined ref to vtable/typeinfo)
void DecoderEmitter::run(raw_ostream &OS) {
  dump();

  emitSourceFileHeader("RV Decoder structures", OS);

  OS << "#include <iostream>\n"
     << "#include <bitset>\n"
     << "#include <memory>\n"
     << "#include <cstdint>\n"
     << "#include <vector>\n"
     << "\n"
     << "#include \"encoding.hpp\"\n"
     << "\n";

  std::vector<InstructionInfo> InsnInfos;

  for (auto D : RK_.getAllDerivedDefinitions("RVInsn")) {
    std::vector<EncodingField> EncodingFields;

    uint32_t RawEncoding = 0;

    std::string InsnName = D->getNameInitAsString();
    std::optional<StringRef> AsmStr = D->getValueAsOptionalString("Name");
    if (AsmStr == std::nullopt) {
      PrintFatalError(D->getLoc(), "Name field uninitialized in RVInsn");
      return;
    }

    std::optional<StringRef> TyName = std::nullopt;
    try {
      TyName = D->getValueAsOptionalString("Type");
    } catch (...) {
      PrintFatalError(D->getLoc(), "Type field does not exist in RVEncodingField");
      return;
    }

    uint32_t EncodingMask = formEncodingFields(D, EncodingFields, RawEncoding);
    assert(EncodingMask && "EncMask can't be zero");

    InsnInfos.push_back(
      InstructionInfo(
        RawEncoding, EncodingMask, EncodingFields,
        InsnName, TyName.value().str(), AsmStr.value().str()
      )
    );
  }

  OS << "namespace RVISA {\n\n";

  emitTypesEnum(OS, InsnInfos);

  for (const auto &II : InsnInfos) {
    II.emitClass(OS);
    OS << '\n';
  }

  emitDecoderFunc(OS, InsnInfos);

  OS << "} // RVISA\n";
}

void DecoderEmitter::dump() const {
  RK_.dump();
}

static TableGen::Emitter::OptClass<DecoderEmitter> X("gen-decoder", "Generate rv decoder");

int main(int argc, char *argv[]) {
  InitLLVM X(argc, argv);
  cl::ParseCommandLineOptions(argc, argv);

  return TableGenMain(argv[0]);
}
