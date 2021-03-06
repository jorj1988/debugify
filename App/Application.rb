require_relative 'Debugger'
require_relative 'Console'
require_relative 'CallStack'
require_relative 'Threads'
require_relative 'Breakpoints'
require_relative 'Project'
require_relative 'Values'
require_relative 'Editors'
require_relative 'ProcessSelector'
require_relative 'ModuleExplorer'
require_relative 'Utils'

module App

  class Application
    RecentTargetCount = 10

    def initialize(debug)
      @project = DebuggerProject.new(nil)

      @log = Logger.new

      @application = UI::Application.new(File.dirname(__FILE__))
      @mainwindow = UI::MainWindow.new()
      @mainwindow.aboutToClose.listen do
        @project.set_value(:application_geometry, @mainwindow.geometry(), :user)
        @project.close()
        @project = nil
      end

      if (debug)
        require_relative 'DebuggerTerminal'
        @debugTerminal = App::DebuggerTerminal.new(self)
      end

      @debugger = App::Debugger.new(@mainwindow, @log, @project)
      @editors = Editors.new(@mainwindow, @debugger, @project)
      @moduleExplorer = App::ModuleExplorer.new(@mainwindow, @debugger)
      @callStack = App::CallStack.new(@mainwindow, @debugger)
      @threads = App::Threads.new(@mainwindow, @debugger)
      @values = App::Values.new(@mainwindow, @debugger)
      @breakpoints = App::Breakpoints.new(@mainwindow, @debugger, @project)
      @console = App::Console.new(@mainwindow, @debugger)
      @log.console = @console
      @processWindows = [ @threads, @callStack, @values ]

      buildToolbars()
      buildMenus()

      @debugger.processBegin.listen { |p| onProcessChanged(p) }
      @debugger.processEnd.listen { |p| onProcessChanged(nil) }
      @debugger.targetChanged.listen { |t| onTargetChanged(t) }
      @debugger.running.listen { |p| onRunning(p) }
      @debugger.ready.listen { |p| onReady(p) }
      @debugger.exited.listen { |p| onExited(p) }
    end

    attr_reader :mainwindow, :project, :editors

    def execute()
      val = @project.value(:application_geometry, "")
      @mainwindow.setGeometry(val)
      onTargetChanged(nil)
      onProcessChanged(nil)

      val = @project.value(:disable_most_recent, false)
      if (!val)
        recents = @project.value(:recents, [])
        if (recents.length > 0)
          recent = recents[-1]
          @log.log "Loading previous project #{App.shortPath(recent)}"
          loadTarget(recent)
        end
      end

      @mainwindow.show()
      
      @log.log "Debugger is up."
      @application.execute()
    end

    def loadTarget(target)
      recents = @project.value(:recents, [])
      recents.delete(target)
      if (recents.length > RecentTargetCount)
        recents.shift
      end
      recents << target

      @project.set_value(:recents, recents, :user)

      @debugger.load(target)
    end

  private
    def buildToolbars()
      @targetToolbar = @mainwindow.addToolBar("Target")
      @targetToolbar.addAction("Run", Proc.new {
        @debugger.launch()
      })
      @targetToolbar.addAction("Attach", Proc.new {
        pid = ProcessSelector.show(File.basename(@debugger.target.path))
        @debugger.attach(pid) unless pid == nil
      })
      @targetToolbar.addAction("Remote Attach", Proc.new {
        dlg = UI::Dialog.new(UI::UIC_PATH + "remoteconnect.ui")
        dlg.exec()
        if (UI::Dialog::Result[dlg.result()] == :Accepted)
          @debugger.connect("connect://" + dlg.value("url"))
        end
      })

      @processToolbar = @mainwindow.addToolBar("Process")
      @processToolbar.addAction("Kill", Proc.new {
        @mainwindow.process.kill()
      })
      
      @actions = { }
      @actions[:pause_continue] = @processToolbar.addAction("", Proc.new {
        if (@mainwindow.process.currentState == LldbDriver::ProcessState[:Stopped])
          @mainwindow.process.continueExecution()
        else
          @mainwindow.process.pauseExecution()
        end
      })

      @currentThreadToolbar = @mainwindow.addToolBar("Current Thread")
      @currentThreadToolbar.addAction("Step Into", Proc.new {
        @mainwindow.process.selectedThread.stepInto()
      })
      @currentThreadToolbar.addAction("Step Over", Proc.new {
        @mainwindow.process.selectedThread.stepOver()
      })
      @currentThreadToolbar.addAction("Step Out", Proc.new {
        @mainwindow.process.selectedThread.stepOut()
      })
      @currentThreadToolbar.hide()
    end

    def buildMenus()
      menu = @mainwindow.addMenu("File")

      menu.addAction("Load", Proc.new {
        target = @mainwindow.getOpenFilename("Load Target")
        if (target.length > 0)
          loadTarget(target)
        end
      })

      targets = menu.addMenu("Recent Targets")
      targets.aboutToShow.listen do
        targets.clear()
        recents = @project.value(:recents, [])
        recents.reverse_each do |r|
          path = App.shortPath(r)

          basename = File.basename(r)
          text = "#{basename} (#{path})"

          targets.addAction(text, Proc.new {
            loadTarget(r)
          })
        end
      end
    end

    def onTargetChanged(t)
      if (t != nil)
        t.breakpointsChanged.listen { |_| @editors.syncEditorBreakpoints() }
        @targetToolbar.show()
      else
        @targetToolbar.hide()
      end
    end

    def onProcessChanged(p)
      @processWindows.each do |w|
        if (p != nil)
          @mainwindow.showDock(w.widget)
        else
          @mainwindow.hideDock(w.widget)
        end
      end

      if (p != nil)
        @processToolbar.show()
      else
        @processToolbar.hide()
        @currentThreadToolbar.hide()
      end
    end

    def onRunning(p)
      @actions[:pause_continue].setText("Pause")
      @editors.clearSourceHighlight()
      @currentThreadToolbar.hide()
      @log.log "Process running"
    end

    def onReady(p)
      @actions[:pause_continue].setText("Continue")
      @editors.loadCurrentSource(@debugger.process)
      @editors.highlightSources()
      @currentThreadToolbar.show()
      @log.log "Process ready to debug"
    end

    def onExited(p)
      @log.log "Process exited with code #{p.exitStatus}"
      desc = p.exitDescription
      if (desc)
        @log.log "  " + desc
      end
    end
  end
end