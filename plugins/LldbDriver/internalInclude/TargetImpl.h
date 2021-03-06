#include "Target.h"
#include "ModuleImpl.h"
#include "BreakpointImpl.h"
#include "lldb/API/SBTarget.h"
#include <vector>

class LldbDriver::Target::Impl
  {
public:
  lldb::SBTarget target;
  std::weak_ptr<LldbDriver::Target> myself;

  bool modulesCached = false;
  std::vector<std::shared_ptr<LldbDriver::Module>> modules;

  void cacheModules()
    {
    if (modulesCached)
      {
      return;
      }

    modules.clear();
    for (size_t i = 0; i < target.GetNumModules(); ++i)
      {
      auto module = std::make_shared<Module>();
      module->_impl->myself = module;
      module->_impl->target = myself.lock();
      module->_impl->module = target.GetModuleAtIndex(i);

      modules.push_back(module);
      }

    modulesCached = true;
    }

  LldbDriver::Breakpoint make(const lldb::SBBreakpoint &br)
    {
    LldbDriver::Breakpoint breakpoint;
    breakpoint._impl->target = myself.lock();
    breakpoint._impl->breakpoint = br;

    return breakpoint;
    }
  };
