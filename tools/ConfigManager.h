#ifndef EMP_CONFIG_MANAGER_H
#define EMP_CONFIG_MANAGER_H

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  The ConfigManager templated class handles the building and configuration of new objects
//  of the target type.
//
//  The manager is created with two keywords; one for the type of the managed class, and the
//  other for the keyword to trigger commands for it.
//
//  For example, if we're configuring an instruction set, the type might be 'inst_set' and the
//  keyword might be 'inst'.  Then the configuration file can have lines like:
//
//     new inst_lib 4stack
//     inst nopA
//     inst inc
//     ...
//
//  Note: The class being managed must also have a member function:
//    bool CommandCallback(std::string)
//  to handle configuation commands.

#include <map>
#include <string>
#include <sstring>

namespace emp {

  template <class MANAGED_TYPE> class ConfigManager {
  private:
    std::map<std::string, MANAGED_TYPE *> name_map;
    MANAGED_TYPE * cur_obj;
    const std::string type_keyword;
    const std::string command_keyword;

  public:
    ConfigManager(const std::string & _type, const std::string & _command)
      : cur_obj(NULL), type_keyword(_type), command_keyword(_command) { ; }
    ~ConfigManager() { ; }

    void NewObject(const std::string & obj_name) {
      if (name_map.find(obj_name) != name_map.end()) {
        std::stringstream ss;
        ss << "Build new object of type '" << type_keyword
           << "' named '" << obj_name < "' when one already exists. Replacing."
           << std::endl;
        NotifyError(ss.str());
        delete name_map[obj_name];
      }
      cur_obj = new MANAGED_TYPE;
      name_map[obj_name] = cur_obj;
    }

    void UseObject(const std::string & obj_name) {
      if (name_map.find(obj_name) == name_map.end()) {
        std::stringstream ss;
        ss << "Trying to use object of type '" << type_keyword
           << "' named '" << obj_name < "', but does not exist. Ignoring."
           << std::endl;
        NotifyError(ss.str());
        return;
      }
      cur_obj = name_map[obj_name];
    }

    bool CommandCallback(const std::string & command) {
      if (cur_obj == NULL) {
        std::stringstream ss;
        ss << "Must build new object of type '" << type_keyword
           << "' before using command '" << command_keyword < "'.  Ignoring."
           << std::endl;
        NotifyError(ss.str());
        return false;
      }

      return cur_obj->CommandCallback(command);
    }
  };

};

#endif
