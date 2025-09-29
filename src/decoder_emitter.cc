#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/TableGen/TableGenBackend.h"
#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/Main.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

class DecoderEmitter {
  const RecordKeeper &RK_;

public:
  DecoderEmitter(const RecordKeeper &RK) : RK_(RK) {}

  void run(raw_ostream &OS);
};
}

void DecoderEmitter::run(raw_ostream &OS) {
  emitSourceFileHeader("RV Decoder structures", OS);

  for (auto &R : RK_.getClasses()) {
    OS << "Found Class\n" << R.first << ' ' << *R.second << R.second->isClass() << '\n';
  }
  
  for (auto &R : RK_.getDefs()) {
    OS << "Found Definition\n" << R.first << ' ' << *R.second << R.second->isClass() << '\n';
  }

}

static TableGen::Emitter::OptClass<DecoderEmitter> X("gen-decoder-emitter-class", "Generate rv decoder emitter class");

int main(int argc, char *argv[]) {
  InitLLVM X(argc, argv);
  cl::ParseCommandLineOptions(argc, argv);

  return TableGenMain(argv[0]);
}
