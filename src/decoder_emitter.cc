#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/TableGen/TableGenBackend.h"
#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/Main.h"
#include "llvm/TableGen/Error.h"
#include "llvm/Support/raw_ostream.h"

#include <bitset>
#include <cstdint>
#include <set>

using namespace llvm;

namespace {

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
                  std::string Name, std::string Type, std::string AsmStr,
                  std::string ExecuteCode = "")
    : RawEncoding_(RawEncoding), TypeMask_(TypeMask), EncodingFields_(EncodingFields),
      Name_(Name), Type_(Type), AsmStr_(AsmStr), ExecuteCode_(ExecuteCode) {

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
    OS << "class " << Name_ << " : public IRVInsn {\n";
    OS << "\t" << "static constexpr uint32_t RawEncoding_ = " << RawEncoding_ << "; // "
                << "0b" << std::bitset<32>(RawEncoding_).to_string() << "\n";
    OS << "\t" << "uint32_t Opcode_ = RawEncoding_; // fully encoded instruction\n";
    OS << "\t" << "std::string AsmStr_ = \"" << AsmStr_ << "\";\n";

    OS << "\t" << "std::vector<uint32_t> Operands_;\n\n";

    OS << "public:\n";

    // constructors
    OS << "\t" << Name_ << "() : " << Name_ << "(RawEncoding_) {} // w/a to assign some operands even for default constructed insn;\n\n";
    OS << "\t" << Name_ << "(uint32_t Opcode) : Opcode_(Opcode) {\n";
    OS << "\t\t" << "Operands_.resize(" << nOperands() << ");\n";
    OS << "\t\t" << "encode_(Opcode);\n";
    OS << "\t}\n\n";

    // opcode
    OS << "\t" << "uint32_t getOpcode() const override { return Opcode_; }\n";

    // type
    OS << "\t" << "RVInsnTypes getType() const override {\n"
       << "\t\t" << "return RVInsnTypes::" << Type_ << "_TYPE_INSN;\n"
       << "\t" << "}\n\n";
    OS << "\t" << "static uint32_t getTypeMask() {\n"
       << "\t\t" << "return " << TypeMask_ << "; // "
                 << "0b" << std::bitset<32>(TypeMask_).to_string() << "\n"
       << "\t" << "}\n\n";

    // name
    OS << "\t" << "std::string getName() const override {\n"
       << "\t\t" << "return AsmStr_;\n"
       << "\t}\n\n";

    // operand functions
    OS << "\t" << "// sets the operand\n";
    OS << "\t" << "void setOperand(uint32_t OpIdx, uint32_t OpVal) override {\n"
       << "\t\t" << "assert(OpIdx < " << nOperands() << ");\n"
       << "\t\t" << "Operands_[OpIdx] = OpVal;\n"
       << "\t" << "}\n\n";
    OS << "\t" << "uint32_t getOperand(uint32_t OpIdx) const override {\n"
       << "\t\t" << "return Operands_[OpIdx];\n"
       << "\t" << "}\n\n";
    OS << "\t" << "uint32_t nOperands() const override {\n"
       << "\t\t" << "return " << nOperands() << ";\n"
       << "\t" << "}\n\n";

    // encode
    OS << "\t" << "// encode operands\n";
    for (uint32_t OpIdx = 0; OpIdx != nOperands(); ++OpIdx) {
      OS << "\t" << "// @param Operands[" << OpIdx << "] - "
                 << getOperandName(OpIdx) << "\n";
    }
	  OS << "\t" << "// @throws `std::out_of_range` exception if `Operands` vector is too small,\n"
	     << "\t" << "//          if it is too big, the extra elements are ignored\n"
	     << "\t" << "// @returns freshly encoded instruction\n"
       << "\t" << "uint32_t encode(std::vector<uint32_t> Operands) override {\n"
       << "\t\t" << "(void) Operands; // unused var warning for insns w/o operands\n";
    for (uint32_t OpIdx = 0; OpIdx != nOperands(); ++OpIdx) {
      OS << "\t\t" << "Opcode_     |= Operands.at(" << OpIdx << ") << "
                   << getOperandMaskLSB(OpIdx) << "; // "
                   << getOperandName(OpIdx) << "\n";
      OS << "\t\t" << "Operands_[" << OpIdx << "] = Operands.at(" << OpIdx << ");\n";
    }
    OS << "\t\t" << "return Opcode_;\n";
    OS << "\t" << "}\n\n";

    // encode (new Opcode)
    OS << "private:\n\t";
    OS << "// encode\n\t";
	  OS << "void encode_(uint32_t NewOpcode) {\n\t\t";
	  OS << "assert((Opcode_ & getTypeMask()) == (NewOpcode & getTypeMask()));\n\t\t";
    for (uint32_t OpIdx = 0, NOps = nOperands(); OpIdx != NOps; ++OpIdx)
      OS << "setOperand(" << OpIdx << ", (NewOpcode & " << getOperandMask(OpIdx)
         << ") >> " << getOperandMaskLSB(OpIdx)
         << ");\n\t\t";
    OS << "Opcode_ = NewOpcode;\n\t";
    OS << "}\n\n";

    // encode (new Opcode)
    OS << "public:\n";
    OS << "\t" << "// encode\n";
	  OS << "\t" << "void encode(uint32_t NewOpcode) override {\n\t\t";
	  OS << "\t" << "encode_(NewOpcode);\n";
    OS << "}\n\n";

    // execute
    OS << "\t" << "void execute(rv32i_sim::IRVModel &Model) const override {\n"
       << "\t\t" << ExecuteCode_ << '\n'
       << "\t}\n\n";

    // print
    OS << "\t" << "void print(std::ostream &Out) const override {\n"
       << "\t\t" << "Out << std::bitset<32>(Opcode_).to_string() << ' ' << AsmStr_ << \" ("
                 << Type_ << ")\";\n";
    OS << "\t""}\n\n";

    OS << "};\n";

    return;
  }
};
}

namespace {

class DecoderEmitter final {
  const RecordKeeper &RK_;

  const ListInit *getEncodingFields(const Record *InsnDef) const;
  const ListInit *getEncodingValues(const Record *InsnDef) const;
  uint32_t  formEncodingFields(const Record * const Def,
                               std::vector<EncodingField> &EncFields,
                               uint32_t &RawEncoding) const;

  static void emitDecoder(raw_ostream &OS,
                              const std::vector<InstructionInfo> &InsnInfos);
  static void emitTypesEnum(raw_ostream &OS,
                            const std::vector<InstructionInfo> &InsnInfos);
public:
  DecoderEmitter(const RecordKeeper &RK) : RK_(RK) {}

  void run(raw_ostream &OS);
  void dump() const;
};
}

const ListInit *DecoderEmitter::getEncodingFields(const Record *InsnDef) const {
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

  const Init *EncFInit = EncF->getValue();

  return dyn_cast<ListInit>(EncFInit);
}

const ListInit *DecoderEmitter::getEncodingValues(const Record *InsnDef) const {
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

  const Init *EncVInit = EncV->getValue();

  return dyn_cast<ListInit>(EncVInit);
}

uint32_t DecoderEmitter::formEncodingFields(const Record * const Def,
                                            std::vector<EncodingField> &EncFields,
                                            uint32_t &RawEncoding) const {
  assert(EncFields.empty());

  const ListInit *TGEncodingFields = getEncodingFields(Def);
  const ListInit *TGEncodingValues = getEncodingValues(Def);
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

    const DefInit *TGEncodingField = dyn_cast<DefInit>(*TGEncodingFieldInit);
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

    const IntInit *TGEncodingValue = dyn_cast<IntInit>(*TGEncodingValueInit);
    uint32_t EncValCode = TGEncodingValue->getValue();

    RawEncoding |= EncValCode << LSBPos;

    EncodingMask |= ((1 << (MSBPos - LSBPos + 1)) - 1) << LSBPos;
    EncFields.push_back(EncodingField(LSBPos, MSBPos, EncValCode, EncName));

    TGEncodingFieldInit++;
    TGEncodingValueInit++;
  }

  return EncodingMask;
}

void DecoderEmitter::emitDecoder(raw_ostream &OS,
                                 const std::vector<InstructionInfo> &InsnInfos) {
  OS << "class Decoder : public IDecoder {\n\t";
  OS << "std::unordered_map<uint64_t, std::shared_ptr<IRVInsn>> Insns_;\n";
  OS << "public:\n\t";

  OS << "Decoder() {\n\t\t";
  for (auto &II: InsnInfos) {
    OS << "Insns_[((uint64_t)" << II.getTypeMask() << " << 32) | "
                    << II.getRawEncoding() << "] = std::make_shared<"
                    << II.getName() << ">();\n\t\t";
  }

  OS << "}\n\t";

  OS << "std::shared_ptr<IRVInsn> decode(uint32_t Opcode) override {\n\t\t";

  std::set<uint32_t> UniqueMasks;
  for (auto &II: InsnInfos) UniqueMasks.insert(II.getTypeMask());

  OS << "auto ";
  for (auto &M : UniqueMasks) {
    OS << "It = Insns_.find(((uint64_t)" << M << " << 32) | " << "(Opcode & " << M << "));\n\t\t";
    OS << "if (It != Insns_.end()) { It->second->encode(Opcode); return It->second; }\n\n\t\t";
  }

  OS << "return nullptr;\n\t";

  OS << "} // decode()\n";

  OS << "};\n";
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

void DecoderEmitter::run(raw_ostream &OS) {
  emitSourceFileHeader("RV Decoder structures", OS);

  OS << "#include <iostream>\n"
     << "#include <bit>\n"
     << "#include <bitset>\n"
     << "#include <cstdint>\n"
     << "#include <cmath>\n"
     << "#include <vector>\n"
     << "\n"
     << "#include \"idecoder.hpp\"\n"
     << "#include \"decoder_helpers.hpp\"\n\n"
     << "namespace Sim = rv32i_sim;\n\n"
     << "using namespace RVDecoder;\n\n";

  std::vector<InstructionInfo> InsnInfos;

  for (const auto *D : RK_.getAllDerivedDefinitions("RVInsn")) {
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

    std::optional<StringRef> ExecCode = std::nullopt;
    try {
      ExecCode = D->getValueAsOptionalString("Code");
    } catch (...) {
      PrintFatalError(D->getLoc(), "Code field does not exist in RVEncodingField");
      return;
    }

    uint32_t EncodingMask = formEncodingFields(D, EncodingFields, RawEncoding);
    assert(EncodingMask && "EncMask can't be zero");

    InsnInfos.push_back(
      InstructionInfo(
        RawEncoding, EncodingMask, EncodingFields,
        InsnName, TyName.value().str(), AsmStr.value().str(),
        ExecCode.value_or("(void)Model; (void)TypeMask_; // no code provided in .td\n").str()
      )
    );
  }

  OS << "namespace RVISA {\n\n";

  emitTypesEnum(OS, InsnInfos);

  for (const auto &II : InsnInfos) {
    II.emitClass(OS);
    OS << '\n';
  }

  emitDecoder(OS, InsnInfos);

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
