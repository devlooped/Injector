![Icon](https://raw.githubusercontent.com/devlooped/Injector/main/assets/img/icon-32.png) .NET Injector
============

[![Version](https://img.shields.io/nuget/vpre/Devlooped.Injector.svg)](https://www.nuget.org/packages/Devlooped.Injector)
[![Downloads](https://img.shields.io/nuget/dt/Devlooped.Injector.svg)](https://www.nuget.org/packages/Devlooped.Injector)
[![License](https://img.shields.io/github/license/devlooped/Injector.svg?color=blue)](https://github.com//devlooped/Injector/blob/main/license.txt)
[![Build](https://github.com/devlooped/Injector/workflows/build/badge.svg?branch=main)](https://github.com/devlooped/Injector/actions)

<!-- #content -->
Allows injecting .NET code into any Windows process.

Heavily based on [Cory Plott](http://www.cplotts.com)'s [Snoop](https://github.com/cplotts/snoopwpf).

The only requirement is that the injected code must be a public static method on a public static 
class, such as:

```csharp
namespace Sample;

public static class Startup
{
    public static void Start(string arg1, int arg2, bool debug)
    {
        if (debug)
            Debugger.Launch();

        // do stuff with arg1, arg2, etc.
        // note args are typed :)
    }
}
```

> NOTE: parameter type conversion is supported and happens via the `TypeConverter` associated with the 
parameter type.


## Usage

There are two main usages for this package:

* From AnyCPU code: your code is bitness-agnostic and can be injected into 
  the target process whether it's x86 or x64.
* From x86/x64 code: you are injecting into a target process that has the same 
  bitness as the calling code. 

### AnyCPU code

This is likely the more common scenario. You have .NET code that is AnyCPU and can 
therefore be injected regardless of the target process bitness. When referencing this 
package, you will get two (content) folders containing a helper `Injector.exe` for each architecture:

![Screenshot](https://github.com/devlooped/Injector/raw/main/assets/img/content-files.png)

These files are automatically copied to the output directory under `Injector\[x86|x64]\Injector.exe` 
(and are also included when doing `dotnet publish`). This allows you to run the relevant executable 
that matches the target process bitness. 

`Injector.exe` usage:

```
> Injector.exe -?
Usage: Injector.exe <mainWindowHandle> <assemblyFile> <typeName> <methodName>

Arguments:
  <processMainWindowHandle>   IntPtr of the main window handle of the process to inject, i.e. Process.MainWindowHandle.
  <assemblyFile>              The full path to the .NET assembly to load in the remote process.
  <typeName>                  Full type name of the public static class to invoke in the remote process.
  <methodName>                Name of the static method in that class to invoke in the remote process. Must be a
                              static method, which can also receive arguments, such as 'Start:true:42'.
```

To detect the target process bitness, you can use the following bit of interop:

```csharp
static class NativeMethods
{
    [DllImport("kernel32.dll", SetLastError = true, CallingConvention = CallingConvention.Winapi)]
    [return: MarshalAs(UnmanagedType.Bool)]
    internal static extern bool IsWow64Process([In] IntPtr process, [Out] out bool wow64Process);
}
```

And the following code would lookup the target process (in this case, we just get the first instance 
of `notepad.exe`, as an example), and invoke the right executable:

```csharp
var targetProcess = System.Diagnostics.Process.GetProcessesByName("notepad.exe")[0];

NativeMethods.IsWow64Process(targetProcess.Handle, out var isWow);
var platform = isWow ? "x86" : "x64";

Process.Start(Path.Combine("Injector", platform, "Injector.exe"),
    // IntPtr of the main window handle of the process to inject
    targetProcess.MainWindowHandle + " " +
    // The full path to the .NET assembly to load in the remote process
    Assembly.GetExecutingAssembly().Location + " " +
    // Full type name of the public static class to invoke in the remote process
    typeof(Startup).FullName + " " +
    // Name of the static method in that class to invoke in the remote process, 
    // and any parameters.
    $"{nameof(Startup.Start)}:hello:42:true");
```

> NOTE: we can pass typed arguments to the `Startup.Start` method (shown as an example 
> at the beginning) and type conversion will be applied automatically.


### Platform-specific code

When building platform-specific code, the project would typically have (for a console app, for example):

```xml
<Project Sdk="Microsoft.NET.Sdk">
	<PropertyGroup>
		<OutputType>Exe</OutputType>
		<TargetFramework>net6.0</TargetFramework>
		<Platforms>x64;x86</Platforms>
	</PropertyGroup>
</Project>
```

You would then build for either platform via: `dotnet build --arch x64` or `dotnet build --arch x86`.  

In this case, the bitness of the calling code (that intends to inject itself into a remote process) 
must match the target process bitness too. Since the bitness of both is the same, you can use the 
automatically referenced assembly from your code, rather than invoking the helper `Injector.exe` 
as shown in the first case.

The code will otherwise look similar to the previous case:


```csharp
var targetProcess = System.Diagnostics.Process.GetProcessesByName("notepad.exe")[0];

// NOTE: target process bitness must match our own assembly architecture for 
// this to succeed.
Devlooped.Injector.Launch(
    // IntPtr of the main window handle of the process to inject
    targetProcess.MainWindowHandle,
    // The full path to the .NET assembly to load in the remote process
    Assembly.GetExecutingAssembly().Location,
    // Full type name of the public static class to invoke in the remote process
    typeof(Startup).FullName + " " +
    // Name of the static method in that class to invoke in the remote process, 
    // and any parameters.
    $"{nameof(Startup.Start)}:hello:42:true");
```

> NOTE: the `Devlooped.Injector` type will NOT be available on AnyCPU projects


See [Program.cs](src/Sample/Program.cs) for a complete example.


<!-- include https://github.com/devlooped/sponsors/raw/main/footer.md -->
# Sponsors 

<!-- sponsors.md -->
[![Clarius Org](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/clarius.png "Clarius Org")](https://github.com/clarius)
[![Christian Findlay](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/MelbourneDeveloper.png "Christian Findlay")](https://github.com/MelbourneDeveloper)
[![C. Augusto Proiete](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/augustoproiete.png "C. Augusto Proiete")](https://github.com/augustoproiete)
[![Kirill Osenkov](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/KirillOsenkov.png "Kirill Osenkov")](https://github.com/KirillOsenkov)
[![MFB Technologies, Inc.](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/MFB-Technologies-Inc.png "MFB Technologies, Inc.")](https://github.com/MFB-Technologies-Inc)
[![Amazon Web Services](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/aws.png "Amazon Web Services")](https://github.com/aws)
[![SandRock](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/sandrock.png "SandRock")](https://github.com/sandrock)
[![David Pallmann](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/davidpallmann.png "David Pallmann")](https://github.com/davidpallmann)


<!-- sponsors.md -->

[![Sponsor this project](https://raw.githubusercontent.com/devlooped/sponsors/main/sponsor.png "Sponsor this project")](https://github.com/sponsors/devlooped)
&nbsp;

[Learn more about GitHub Sponsors](https://github.com/sponsors)

<!-- https://github.com/devlooped/sponsors/raw/main/footer.md -->
