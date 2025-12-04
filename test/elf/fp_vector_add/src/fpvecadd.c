#include "api.h"
static const unsigned N_INPUTS = 7;
static const volatile float INPUTS[][7] = {
  { .0, 1.1,  2.2, 1e-7, 1e3, 1e-3, 0},
  {-.0, 1.1, -2.2,  1e7,  10, 1e-3, 1e10}
};

static float RunTest(float a, float b) {
  return a * b + a + b - a / b;
}

int main() {
  float Results[N_INPUTS];

  for (int i = 0; i != N_INPUTS; ++i) {
    Results[i] = RunTest(INPUTS[0][i], INPUTS[1][i]);
  }

  write(1, (const char *) Results, sizeof(Results));
  return 0;
}
