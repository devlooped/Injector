# Windows.Injector

Allows injecting .NET code into a remote process on Windows

Heavily based on [Cory Plott](http://www.cplotts.com)'s [Snoop](https://github.com/cplotts/snoopwpf).

Usage:

* Install:
```
install-package Windows.Injector
```

* Launch:

```csharp
var targetProcess = System.Diagnostics.Process.GetProcessesByName("devenv.exe")[0];
Injector.Launch(
    // IntPtr of the main window handle of the project to inject
    targetProcess.MainWindowHandle,
    // The full path to the .NET assembly to load in the remote process
    Assembly.GetExecutingAssembly().Location,
    // Full type name of the public static class to invoke in the remote process
    "MyApp.Startup",
    // Name of the static method in that class to invoke in the remote process
    "Start");
```