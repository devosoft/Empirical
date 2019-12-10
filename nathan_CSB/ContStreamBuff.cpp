#include <iostream>
#include <vector>
#include <ostream>
#include <cstring>


class ContiguousStreamBuf : public std::streambuf{ 
public:
  ContiguousStreamBuf(size_t s = 16){
    size = s;
    buffer = new char[size];
    //Initiailize the 3 streambuf pointers to point at the beg/end
    //the cur pointer points at beg too.
    this->setp(buffer, buffer+size);
  }

  virtual ~ContiguousStreamBuf(){
    delete[] buffer;
  }

  char* c_str(){
    return buffer;
  }

private:
  char* buffer;
  size_t size; //capacity of the array. doubles when overflow.


  // Called by sputc when we run out of space in buffer
  virtual int_type overflow (int_type c){
    //allocate a c str twice the size
    char* new_buffer = new char[size*2];
    // copy everything over. (I've been ignoring null terminators)
    // May need to check that everything I've done is safe.
    strcpy(new_buffer, buffer);

    //delete original buffer
    delete[] buffer;

    //update the new capacity
    size*=2; 

    //Update the 3 streambuf pointers to point to the new array.
    this->setp(buffer, buffer+size);
    this->pubseekoff(std::strlen(buffer), std::ios_base::beg);


     //add the character that originally overflowed.
    *(this->pptr()) = c;
    pbump(1);

   return c;
  }

};


int main(){
  ContiguousStreamBuf cs;
  std::ostream out(&cs);
  //Send 12 letters: 11 + \n.
  out << "Hello_World";
  std::cout << cs.c_str() << std::endl;
  std::cout << std::endl;
  //Overflow the buffer
  out << "calloVerflow"<<std::endl;
  std::cout << cs.c_str() << std::endl;

  std::cout<< "Unfortunately there's something wrong with the output. I may have misunderstood pubseekoff?";
  return 0;
}

