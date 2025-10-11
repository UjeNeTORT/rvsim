#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/TableGen/TableGenBackend.h"
#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/Main.h"
#include "llvm/TableGen/Error.h"
#include "llvm/Support/raw_ostream.h"

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

  StringRef Name_ = "???";
public:
  EncodingField(uint32_t First, uint32_t Last, StringRef Name)
    : First_(First), Last_(Last), Size_(Last_ - First + 1),
      IsEncoded_(false), Value_(std::nullopt), Name_(Name) {}
  EncodingField(uint32_t First, uint32_t Last, uint32_t Value, StringRef Name)
    : First_(First), Last_(Last), Size_(Last_ - First + 1),
      IsEncoded_(true), Value_(Value), Name_(Name) {}

  uint32_t getSize() const { return Size_; }
  std::optional<uint32_t> getValue() const { return Value_; }
};
}

namespace {
class InstructionInfo final {
  uint32_t RawEncoding_;
  uint32_t TypeMask_;

  std::vector<EncodingField> EncodingFields_;

  std::string Name_; // todo add to ctor
  std::string AsmStr_; // todo add to ctor
  std::string ExecuteCode_; // todo add to ctor

public:
  InstructionInfo(uint32_t RawEncoding, uint32_t TypeMask,
                  std::vector<EncodingField> EncodingFields, std::string Name)
    : RawEncoding_(RawEncoding), TypeMask_(TypeMask), EncodingFields_(EncodingFields),
      Name_(Name) {}

  uint32_t getTypeMask() const { return TypeMask_; }

  void emitClass(raw_ostream &Out) const {
    Out << "class " << Name_ << " {\n";
    Out << "\t" << "uint32_t RawEncoding_ = " << RawEncoding_ << "; // "
                << "0b" << std::bitset<32>(RawEncoding_).to_string() << "\n";
    Out << "\t" << "uint32_t TypeMask_ = " << TypeMask_ << "; // "
                << "0b" << std::bitset<32>(TypeMask_).to_string() << "\n";
    Out << "\t" << "std::string AsmStr_   = " << AsmStr_ << ";\n";
    Out << "public:\n";

    Out << "\t" << "void execute(IRVModel &Model) const override {\n"
        << "\t" << ExecuteCode_ << '\n'
        << "\t}\n";

    Out << "};\n";
  }
};
}

namespace {

class DecoderEmitter final {
  const RecordKeeper &RK_;

  ListInit *getEncodingFields(const Record *InsnDef) const;
  ListInit *getEncodingValues(const Record *InsnDef) const;

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

void DecoderEmitter::run(raw_ostream &OS) {

  dump();

  emitSourceFileHeader("RV Decoder structures", OS);

  OS << "#include <iostream>\n"
     << "#include <cstdint>\n"
     << "\n"
     << "#include \"encoding,hpp\"\n"
     << "\n";

  std::vector<InstructionInfo> InsnInfos;

  // whenever a unique instruction type is encountered
  // its mask is stored here for its further use in decoder function
  std::set<std::pair<uint32_t, StringRef>> EncodingMasks;

  for (auto &D : RK_.getAllDerivedDefinitions("RVInsn")) {
    ListInit *TGEncodingFields = getEncodingFields(D);
    ListInit *TGEncodingValues = getEncodingValues(D);
    if (!TGEncodingFields) {
      PrintError(D->getLoc(), "EncFields is not a ListInit!");
      return;
    }

    if (!TGEncodingValues) {
      PrintError(D->getLoc(), "EncValues is not a ListInit!");
      return;
    }

    std::vector<EncodingField> EncodingFields;

    uint32_t RawEncoding = 0;
    uint32_t EncodingMask = 0;

    std::string InsnName = D->getNameInitAsString();

    std::optional<StringRef> TyName;
    try {
      TyName = D->getValueAsOptionalString("Type");
    } catch (...) {
      PrintFatalError(D->getLoc(), "Type field does not exist in RVEncodingField");
      return;
    }

    auto TGEncodingFieldInit = TGEncodingFields->begin();
    auto TGEncodingValueInit = TGEncodingValues->begin();

    while (TGEncodingFieldInit != TGEncodingFields->end() &&
           TGEncodingValueInit != TGEncodingValues->end()) {

      std::optional<StringRef> EncName;

      //? is it ok? can it be simplified? do we need catch block at all?
      try {
        EncName = D->getValueAsOptionalString("Name");
      } catch (...) {
        PrintFatalError(D->getLoc(), "Name field does not exist in RVEncodingField");
        return;
      }

      if (EncName == std::nullopt) {
        PrintError(D->getLoc(), "Name field is uninitialized in RVEncodingField");
        continue;
      }

      DefInit *TGEncodingField = dyn_cast<DefInit>(*TGEncodingFieldInit);
      if (!TGEncodingField || !TGEncodingField->getDef()->isSubClassOf("RVEncodingField")) {
        PrintError(D->getLoc(), "Encoding must be of type RVEncodingField");
        return;
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
        EncodingFields.push_back(EncodingField(LSBPos, MSBPos, EncName.value()));
        TGEncodingFieldInit++;
        TGEncodingValueInit++;
        continue;
      }

      IntInit *TGEncodingValue = dyn_cast<IntInit>(*TGEncodingValueInit);
      uint32_t EncValCode = TGEncodingValue->getValue();

      RawEncoding |= EncValCode << LSBPos;

      EncodingMask |= ((1 << (MSBPos - LSBPos + 1)) - 1) << LSBPos;
      EncodingFields.push_back(EncodingField(LSBPos, MSBPos, EncValCode, EncName.value()));

      TGEncodingFieldInit++;
      TGEncodingValueInit++;
    }

    EncodingMasks.insert(std::pair<uint32_t, StringRef>(EncodingMask, TyName.value()));
    InsnInfos.push_back(InstructionInfo(RawEncoding, EncodingMask, EncodingFields, InsnName));
  }

  for (const auto &EM : EncodingMasks)
    OS << "const uint32_t ENC_MASK_TYPE_" << EM.second << " = " << EM.first << "; "
       << "// 0b" << std::bitset<32>(EM.first).to_string() << '\n';
  OS << '\n';
  for (const auto &II : InsnInfos) II.emitClass(OS);

  OS << "std::unique_ptr<IInsn> decode(uint32_t opcode) {\n";

  OS << "} // decode()\n";
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
