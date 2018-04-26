#include "tools/resources.h"
#include <cstdlib>

class RandomPool {
  public:
  virtual int next() = 0;
};

template <size_t N>
class ExpensiveRandomPool {
  size_t index;
  int data[N];

  public:
  ExpensiveRandomPool() {
    for (int i = 0; i < N; ++i) {
      data[i] = std::rand();
    }
  }

  int next() {
    auto i = index;
    ++index;
    if (index >= N) index = 0;
    return data[i];
  }
};

int main() {
  emp::Resources<ExpensiveRandomPool<1024>>::Add(
    "", [] { return ExpensiveRandomPool<1024>{}; });
}