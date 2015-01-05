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
//     inst divide cycle_cost=10
//     ...
//

#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <sstream>

namespace emp {

  class ConfigManager_Base {
  protected:
    const std::string type_keyword;
    const std::string command_keyword;

  public:
    ConfigManager_Base(const std::string & _type, const std::string & _command)
      : type_keyword(_type), command_keyword(_command) { ; }
    virtual ~ConfigManager_Base() { ; }

    const std::string & GetTypeKeyword() const { return type_keyword; }
    const std::string & GetCommandKeyword() const { return command_keyword; }

    virtual void NewObject(const std::string & obj_name) = 0;
    virtual void UseObject(const std::string & obj_name) = 0;
    virtual bool CommandCallback(const std::string & command) = 0;
  };

  template <class MANAGED_TYPE> class ConfigManager : public ConfigManager_Base {
  private:
    std::map<std::string, MANAGED_TYPE *> name_map;
    MANAGED_TYPE * cur_obj;
    std::function<bool(MANAGED_TYPE &, std::string)> callback_fun;

  public:
    ConfigManager(const std::string & _type, const std::string & _command,
                  std::function<bool(MANAGED_TYPE &, std::string)> _fun)
      : ConfigManager_Base(_type, _command)
      , cur_obj(NULL)
      , callback_fun(_fun)
    {
    }
    ~ConfigManager() { ; }

    void NewObject(const std::string & obj_name) {
      if (name_map.find(obj_name) != name_map.end()) {
        std::stringstream ss;
        ss << "Building new object of type '" << type_keyword
           << "' named '" << obj_name < "' when one already exists. Replacing.";
        ss << std::endl;
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
           << "' named '" << obj_name < "', but does not exist. Ignoring.";
        ss << std::endl;
        NotifyError(ss.str());
        return;
      }
      cur_obj = name_map[obj_name];
    }

    bool CommandCallback(const std::string & command) {
      if (cur_obj == NULL) {
        std::stringstream ss;
        ss << "Must build new object of type '" << type_keyword
           << "' before using command '" << command_keyword < "'.  Ignoring.";
        ss << std::endl;
        NotifyError(ss.str());
        return false;
      }

      return callback_fun(*cur_obj, command);
    }
  };

};

#endif
