#pragma once
#include "Global.h"
#include "Containers/XStringSimple.h"
#include "Utilities/XNotifier.h"

namespace LldbDriver
{

class Error;
class Target;
class Thread;

/// \expose
enum class ProcessState
  {
  Invalid = 0,
  Unloaded,
  Connected,    ///< Process is connected to remote debug services, but not launched or attached to anything yet
  Attaching,    ///< Process is currently trying to attach
  Launching,    ///< Process is in the process of launching
  Stopped,      ///< Process or thread is stopped and can be examined.
  Running,      ///< Process or thread is running and can't be examined.
  Stepping,     ///< Process or thread is in the process of stepping and can not be examined.
  Crashed,      ///< Process or thread has crashed and can be examined.
  Detached,     ///< Process has been detached and can't be examined.
  Exited,       ///< Process has exited and can't be examined.
  Suspended     ///< Process or thread is in a suspended state as far
                ///< as the debugger is concerned while other processes
                ///< or threads get the chance to run.
  };

/// \expose unmanaged
X_DECLARE_NOTIFIER(ProcessStateChangeNotifier, std::function<void (ProcessState)>);

/// \expose unmanaged
X_DECLARE_NOTIFIER(NoArgNotifier, std::function<void ()>);

/// \expose sharedpointer
class Process
  {
  SHARED_CLASS(Process);
  PIMPL_CLASS(Process, sizeof(void*) * 36);

public:
  /// \noexpose
  Process();
  ~Process();


  std::shared_ptr<Target> target();

  size_t processID() const;
  ProcessState currentState() const;

  static Eks::String getStateString(ProcessState s);

  NoArgNotifier *outputAvailable();
  NoArgNotifier *errorAvailable();

  enum class OutputType
    {
    Output,
    Error
    };

  size_t getOutput(OutputType type, char *data, size_t inputSize);
  /// Get the output from the process
  /// \param[out] out
  /// \param[out] err
  void getOutputs(Eks::String &out, Eks::String &err);

  Error kill();
  Error pauseExecution();
  Error continueExecution();

  int exitStatus() const;
  Eks::String exitDescription() const;

  size_t threadCount();
  std::shared_ptr<Thread> threadAt(size_t index);

  void selectThread(const std::shared_ptr<Thread> &);
  std::shared_ptr<Thread> selectedThread();

  void processEvents();

  ProcessStateChangeNotifier *stateChanged();
  NoArgNotifier *ended();


  friend class Target;
  };

}
