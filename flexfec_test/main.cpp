#include <iostream>
#include "mytest/test.h"

int main() {
  webrtc::test t;
  t.WorkTest();

  std::cout << "Hello, World!" << std::endl;
  return 0;
}