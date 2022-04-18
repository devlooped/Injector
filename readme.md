# Devlooped.Injector

[![Version](https://img.shields.io/nuget/vpre/Devlooped.Injector.svg)](https://www.nuget.org/packages/Devlooped.Injector)
[![Downloads](https://img.shields.io/nuget/dt/Devlooped.Injector.svg)](https://www.nuget.org/packages/Devlooped.Injector)
[![License](https://img.shields.io/github/license/devlooped/Injector.svg?color=blue)](https://github.com//devlooped/Injector/blob/main/license.txt)
[![Build](https://github.com/devlooped/Injector/workflows/build/badge.svg?branch=main)](https://github.com/devlooped/Injector/actions)

Allows injecting .NET code into a remote process on Windows

Heavily based on [Cory Plott](http://www.cplotts.com)'s [Snoop](https://github.com/cplotts/snoopwpf).

Usage:

* Install:

```
install-package Devlooped.Injector
```

The following properties are used to determine which version (`x86` or `x64`) of the `boostrap.dll` 
assembly will be referenced: `PlatformTarget`, `Platform` and `RuntimeIdentifier`. If any of those 
specify either `x86` or `x64`, the assembly will be automatically referenced. Note that you cannot 
invoke the injector via its API from an `AnyCPU` library, since it has to be of a specific bitness.

The targets automatically include as content both the assembly as well as a helper `Injector.exe` 
executable which you can use to inject into processes that have a different bitness than the calling one.

* Launch:

```
var targetProcess = System.Diagnostics.Process.GetProcessesByName("devenv.exe")[0];

Injector.Launch(
    // IntPtr of the main window handle of the process to inject
    targetProcess.MainWindowHandle,
    // The full path to the .NET assembly to load in the remote process
    Assembly.GetExecutingAssembly().Location,
    // Full type name of the public static class to invoke in the remote process
    "MyApp.Startup",
    // Name of the static method in that class to invoke in the remote process
    "Start");
```

Optionally, the injected method call can also receive parameters, in a `{method}:arg1:arg2:argN` format:

```
var targetProcess = System.Diagnostics.Process.GetProcessesByName("devenv.exe")[0];

Injector.Launch(
    // IntPtr of the main window handle of the process to inject
    targetProcess.MainWindowHandle,
    // The full path to the .NET assembly to load in the remote process
    Assembly.GetExecutingAssembly().Location,
    // Full type name of the public static class to invoke in the remote process
    "MyApp.Startup",
    // Name of the static method in that class to invoke in the remote process
    "Start:hello:42:true");
```

See [Program.cs](src/Sample/Program.cs) for complete example.

When referencing the package from an `AnyCPU` project, the `x86` and `x64` folders will be included as 
content and copied to the project output path. This allows you to run the relevant `Injector.exe` for 
that matches the target process bitness. This executable receives the same parameters as the `Launch` 
method shown above.

> NOTE: parameter type conversion is supported and happens via the `TypeConverter` associated with the 
parameter type.
