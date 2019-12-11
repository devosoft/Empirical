#include <iostream>
#include <vector>
#include <ostream>
#include <cstring>


class ContiguousStreamBuf : public std::streambuf {

private:

  std::vector<char> buffer;

public:

  ContiguousStreamBuf(const size_t init_size=4) {
    buffer.reserve(init_size);
    // DON'T initiailize streambuf pointers with setp
    // because it breaks overflow (why?)
  }

  void Reset() {
    this->setp(
      GetData(),
      GetData() + GetCapacity()
    );
  }

  inline char* GetData() { return buffer.data(); }

  inline const char* GetData() const { return buffer.data(); }

  inline size_t GetSize() const {
    return std::distance<const char*>(
      GetData(),
      this->pptr()
    );
  }

  inline size_t GetCapacity() const { return buffer.capacity(); }

  void Print(std::ostream & os=std::cout) const {
    for (size_t i = 0; i < GetSize(); ++i) {
      os << buffer.data()[i];
    }
  }

  // Called by sputc when we run out of space in buffer
  int_type overflow(int_type c) override {

    // double the buffer's capacity
    const size_t prev_capacity = GetCapacity();
    buffer.resize(prev_capacity * 2);

    // update the 3 streambuf pointers to point to the fresh write area
    this->setp(
      GetData() + prev_capacity, // pbase & pptr
      GetData() + GetCapacity() // epptr
    );

    // add the character that originally overflowed.
    *(this->pptr()) = c;
    pbump(1);

    return c;

  }

};


int main() {

  ContiguousStreamBuf cs;
  std::ostream out(&cs);

  // Send 12 letters: 11 + \n.
  out << "Hello_World";
  cs.Print();
  std::cout << std::endl;

  // Overflow the buffer
  out << "calloVerflow" << std::endl;
  cs.Print();
  std::cout << std::endl;

  // Overflow the buffer
  for (size_t i = 0; i < 5; ++i) {
    out << "jsad;kfjsa;lkdfja;ksdjfksajdkjfjjjjasdf Verflow" << std::endl;
  }
  cs.Print();
  std::cout << std::endl;

  std::cout << "Reset" << std::endl;
  cs.Reset();
  cs.Print();
  std::cout << std::endl;

  out << "Hello_World";
  cs.Print();
  std::cout << std::endl;

  return 0;

}
