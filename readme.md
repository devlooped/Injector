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

The `$(PlatformTarget)` or `$(Platform)` properties of the referencing project are used to determine 
which version (`x86` or `x64`) of the `boostrap.dll` assembly will be referenced. If any of those 
specify either `x86` or `x64`, the assembly will be automatically referenced. 

The targets automatically include as content both the assembly as well as a helper `Injector.exe` 
executable which you can use to inject into processes that have a different bitness than the calling one.

* Launch:

```csharp
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

When referencing the package from an `AnyCPU` project, a helper `Injector.exe` 
is provided in both `x86` and `x64` bitness versions and included in the referencing project 
as content, copied automatically to the output under `Injector\[x86|x64]\Injector.exe`. 
This allows you to run the relevant executable that matches the target process bitness. 
This executable receives the same parameters as the `Launch` method shown above. 

For example, to detect a target process bitness, you could use the following interop code:

```csharp
static class NativeMethods
{
    [DllImport("kernel32.dll", SetLastError = true, CallingConvention = CallingConvention.Winapi)]
    [return: MarshalAs(UnmanagedType.Bool)]
    internal static extern bool IsWow64Process([In] IntPtr process, [Out] out bool wow64Process);
}
```

And then use the following code (note it's similar to the API-based one above) to inject 
into it:

```csharp
var targetProcess = System.Diagnostics.Process.GetProcessesByName("devenv.exe")[0];

NativeMethods.IsWow64Process(targetProcess.Handle, out var isWow);
var platform = isWow ? "x86" : "x64";

Process.Start(Path.Combine("Injector", platform, "Injector.exe"),
    targetProcess.MainWindowHandle + " " +
    Assembly.GetExecutingAssembly().Location + " " +
    typeof(Startup).FullName + " " +
    $"{nameof(Startup.Start)}:hello:42:true");
```



Optionally, the injected method call can also receive parameters, in a `{method}:arg1:arg2:argN` format:

```csharp
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

> NOTE: parameter type conversion is supported and happens via the `TypeConverter` associated with the 
parameter type.


<!-- include docs/footer.md -->
# Sponsors 

<!-- sponsors.md -->
<!-- sponsors -->

<a href='https://github.com/KirillOsenkov'>
  <img src='https://github.com/devlooped/sponsors/raw/main/.github/avatars/KirillOsenkov.svg' alt='Kirill Osenkov' title='Kirill Osenkov'>
</a>
<a href='https://github.com/augustoproiete'>
  <img src='https://github.com/devlooped/sponsors/raw/main/.github/avatars/augustoproiete.svg' alt='C. Augusto Proiete' title='C. Augusto Proiete'>
</a>
<a href='https://github.com/sandrock'>
  <img src='https://github.com/devlooped/sponsors/raw/main/.github/avatars/sandrock.svg' alt='SandRock' title='SandRock'>
</a>
<a href='https://github.com/aws'>
  <img src='https://github.com/devlooped/sponsors/raw/main/.github/avatars/aws.svg' alt='Amazon Web Services' title='Amazon Web Services'>
</a>
<a href='https://github.com/MelbourneDeveloper'>
  <img src='https://github.com/devlooped/sponsors/raw/main/.github/avatars/MelbourneDeveloper.svg' alt='Christian Findlay' title='Christian Findlay'>
</a>
<a href='https://github.com/clarius'>
  <img src='https://github.com/devlooped/sponsors/raw/main/.github/avatars/clarius.svg' alt='Clarius Org' title='Clarius Org'>
</a>
<a href='https://github.com/MFB-Technologies-Inc'>
  <img src='https://github.com/devlooped/sponsors/raw/main/.github/avatars/MFB-Technologies-Inc.svg' alt='MFB Technologies, Inc.' title='MFB Technologies, Inc.'>
</a>

<!-- sponsors -->

<!-- sponsors.md -->

<br>&nbsp;
<a href="https://github.com/sponsors/devlooped" title="Sponsor this project">
  <img src="https://github.com/devlooped/sponsors/blob/main/sponsor.png" />
</a>
<br>

[Learn more about GitHub Sponsors](https://github.com/sponsors)

<!-- docs/footer.md -->
