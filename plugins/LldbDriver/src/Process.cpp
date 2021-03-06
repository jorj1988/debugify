#include "Process.h"
#include "ProcessImpl.h"
#include "Error.h"
#include "ErrorImpl.h"
#include "lldb/API/SBEvent.h"
#include "lldb/API/SBStream.h"
#include "lldb/API/SBDebugger.h"
#include <array>

namespace LldbDriver
{
xCompileTimeAssert((int)ProcessState::Suspended == 11);

Process::Process()
  {
  }

Process::~Process()
  {
  kill();
  }

std::shared_ptr<Target> Process::target()
  {
  return _impl->target;
  }

size_t Process::processID() const
  {
  return _impl->process.GetProcessID();
  }

ProcessState Process::currentState() const
  {
  return (ProcessState)_impl->processState;
  }

Eks::String Process::getStateString(ProcessState s)
  {
  return lldb::SBDebugger::StateAsCString((lldb::StateType)s);
  }

NoArgNotifier *Process::outputAvailable()
  {
  return &_impl->outputAvailable;
  }

NoArgNotifier *Process::errorAvailable()
  {
  return &_impl->errorAvailable;
  }

void Process::getOutputs(Eks::String &out, Eks::String &err)
  {
  auto forwardOutput = [this](auto type, auto &send)
  {
    std::array<char, 256> output;
    while (size_t read = getOutput(type, output.data(), output.size()-1))
      {
      xAssert(read <= (output.size()-1));
      output[read] = '\0';
      send += output.data();
      }
    };

  forwardOutput(Process::OutputType::Output, out);
  forwardOutput(Process::OutputType::Error, err);
  }

size_t Process::getOutput(Process::OutputType type, char *data, size_t inputSize)
  {
  if (type == OutputType::Error)
    {
    return _impl->process.GetSTDERR(data, inputSize);
    }
  else if (type == OutputType::Output)
    {
    return _impl->process.GetSTDOUT(data, inputSize);
    }

  return 0;
  }

Error Process::kill()
  {
  auto err = _impl->process.Kill();
  return Error::Helper::makeError(err);
  }

Error Process::pauseExecution()
  {
  auto err = _impl->process.Stop();
  return Error::Helper::makeError(err);
  }

Error Process::continueExecution()
  {
  auto err = _impl->process.Continue();
  return Error::Helper::makeError(err);
  }

int Process::exitStatus() const
  {
  return _impl->process.GetExitStatus();
  }

Eks::String Process::exitDescription() const
  {
  auto desc = _impl->process.GetExitDescription();
  if (!desc)
    {
    return Eks::String();
    }

  return desc;
  }

void Process::processEvents()
  {
  lldb::SBEvent ev;
  while (_impl->listener.GetNextEvent(ev))
    {
    if (ev.GetType() == lldb::SBProcess::eBroadcastBitSTDOUT)
      {
      _impl->outputAvailable();
      }
      if (ev.GetType() == lldb::SBProcess::eBroadcastBitSTDERR)
        {
        _impl->errorAvailable();
        }

    if (lldb::SBProcess::EventIsProcessEvent(ev) &&
        lldb::SBProcess::GetStateFromEvent(ev) != _impl->processState)
      {
      _impl->processState = _impl->process.GetState();

      _impl->stateChanged((ProcessState)_impl->processState);
      if (currentState() == ProcessState::Invalid)
        {
        _impl->ended();
        }
      }
    }
  }

ProcessStateChangeNotifier *Process::stateChanged()
  {
  return &_impl->stateChanged;
  }

NoArgNotifier *Process::ended()
  {
  return &_impl->ended;
  }

size_t Process::threadCount()
  {
  return _impl->process.GetNumThreads();
  }

std::shared_ptr<Thread> Process::threadAt(size_t index)
  {
  auto thread = _impl->process.GetThreadAtIndex(index);

  return _impl->wrapThread(thread);
  }

void Process::selectThread(const std::shared_ptr<Thread> &t)
  {
  if (!t)
    {
    return;
    }

  _impl->process.SetSelectedThread(t->_impl->thread);
  }

std::shared_ptr<Thread> Process::selectedThread()
  {
  return _impl->wrapThread(_impl->process.GetSelectedThread());
  }
}
