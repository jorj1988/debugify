#pragma once
#include "Global.h"
#include "Containers/XStringSimple.h"

namespace LldbDriver
{

class Target;

/// \expose
class BreakpointLocation
  {
public:
  /// \noexpose
  BreakpointLocation();
  /// \noexpose
  BreakpointLocation(int id, const char *file, size_t line, bool resolved);

  int id() const { return _id; }
  Eks::String file() const { return _file; }
  size_t line() const { return _line; }
  bool resolved() const { return _resolved; }

private:
  int _id;
  Eks::String _file;
  size_t _line;
  bool _resolved;
  };

/// \expose
class Breakpoint
  {
  PIMPL_CLASS(Breakpoint, 64);

public:
  Breakpoint();
  Breakpoint(const Breakpoint &);
  ~Breakpoint();

  Breakpoint &operator=(const Breakpoint &);

  const std::shared_ptr<Target> &target();

  size_t id() const;

  bool enabled() const;
  void setEnabled(bool e);

  size_t locationCount() const;
  BreakpointLocation locationAt(size_t i) const;

  /// \param[out] outLoc the found breakpoint location
  bool findLocation(const Eks::String &file, size_t line, BreakpointLocation *outLoc);

  friend class Target;
  };

}
