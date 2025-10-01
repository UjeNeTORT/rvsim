#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/TableGen/TableGenBackend.h"
#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/Main.h"
#include "llvm/TableGen/Error.h"
#include "llvm/Support/raw_ostream.h"

#include <bitset>
#include <ranges>

using namespace llvm;

namespace {

class DecoderEmitter {
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

  for (auto &D : RK_.getAllDerivedDefinitions("RVInsn")) {
    OS << D->getNameInitAsString() << '\n';

    ListInit *EncodingFields = getEncodingFields(D);
    ListInit *EncodingValues = getEncodingValues(D);
    if (!EncodingFields) {
      PrintError(D->getLoc(), "EncFields is not a ListInit!");
      return;
    }

    if (!EncodingValues) {
      PrintError(D->getLoc(), "EncValues is not a ListInit!");
      return;
    }

    uint32_t RawOpcode = 0;

    auto EncodingFieldInit = EncodingFields->begin();
    auto EncodingValueInit = EncodingValues->begin();

    while (EncodingFieldInit != EncodingFields->end() &&
           EncodingValueInit != EncodingValues->end()) {

      // todo add more rules to skip encoding part
      if (!(*EncodingValueInit)->isComplete()) {
        EncodingFieldInit++;
        EncodingValueInit++;
        continue;
      }

      IntInit *EncodingValue = dyn_cast<IntInit>(*EncodingValueInit);
      uint32_t EncValCode = EncodingValue->getValue();

      DefInit *EncodingField = dyn_cast<DefInit>(*EncodingFieldInit);
      if (!EncodingField || !EncodingField->getDef()->isSubClassOf("RVEncodingField")) {
        PrintError(D->getLoc(), "Encoding must be of type RVEncodingField");
        return;
      }

      // todo unsafe
      uint32_t MSBPos = dyn_cast<IntInit>(
        EncodingField->getDef()->getValue("Last")->getValue()
      )->getValue();

      uint32_t LSBPos = dyn_cast<IntInit>(
        EncodingField->getDef()->getValue("First")->getValue()
      )->getValue();

      RawOpcode |= EncValCode << LSBPos;

      EncodingFieldInit++;
      EncodingValueInit++;
    }

    std::bitset<32> RawOpcodeBin(RawOpcode);
    OS << RawOpcodeBin.to_string() << '\n';
  }

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
