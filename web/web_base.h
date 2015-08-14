#ifndef EMP_WEB_BASE_H
#define EMP_WEB_BASE_H

namespace emp {
namespace web {
  
  class Close {
  private:
    std::string close_id;
  public:
    Close(const std::string & id) : close_id(id) { ; }

    const std::string & GetID() const { return close_id; }
  };

}
}

#endif
