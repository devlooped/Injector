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

![Screenshot](https://raw.githubusercontent.com/devlooped/Injector/main/assets/img/content-files.png)

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
of `devenv.exe`, the Visual Studio main process, as an example), and invoke the right executable:

```csharp
var targetProcess = System.Diagnostics.Process.GetProcessesByName("devenv.exe")[0];

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
var targetProcess = System.Diagnostics.Process.GetProcessesByName("devenv.exe")[0];

// NOTE: target process bitness must match our own assembly architecture for 
// this to succeed.
Devlooped.Injector.Launch(
    // IntPtr of the main window handle of the process to inject
    targetProcess.MainWindowHandle,
    // The full path to the .NET assembly to load in the remote process
    Assembly.GetExecutingAssembly().Location,
    // Full type name of the public static class to invoke in the remote process
    typeof(Startup).FullName,
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
[![Kirill Osenkov](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/KirillOsenkov.png "Kirill Osenkov")](https://github.com/KirillOsenkov)
[![MFB Technologies, Inc.](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/MFB-Technologies-Inc.png "MFB Technologies, Inc.")](https://github.com/MFB-Technologies-Inc)
[![Stephen Shaw](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/decriptor.png "Stephen Shaw")](https://github.com/decriptor)
[![Torutek](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/torutek-gh.png "Torutek")](https://github.com/torutek-gh)
[![DRIVE.NET, Inc.](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/drivenet.png "DRIVE.NET, Inc.")](https://github.com/drivenet)
[![Ashley Medway](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/AshleyMedway.png "Ashley Medway")](https://github.com/AshleyMedway)
[![Keith Pickford](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/Keflon.png "Keith Pickford")](https://github.com/Keflon)
[![Thomas Bolon](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/tbolon.png "Thomas Bolon")](https://github.com/tbolon)
[![Kori Francis](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/kfrancis.png "Kori Francis")](https://github.com/kfrancis)
[![Toni Wenzel](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/twenzel.png "Toni Wenzel")](https://github.com/twenzel)
[![Giorgi Dalakishvili](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/Giorgi.png "Giorgi Dalakishvili")](https://github.com/Giorgi)
[![Mike James](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/MikeCodesDotNET.png "Mike James")](https://github.com/MikeCodesDotNET)
[![Uno Platform](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/unoplatform.png "Uno Platform")](https://github.com/unoplatform)
[![Dan Siegel](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/dansiegel.png "Dan Siegel")](https://github.com/dansiegel)
[![Reuben Swartz](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/rbnswartz.png "Reuben Swartz")](https://github.com/rbnswartz)
[![Jacob Foshee](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/jfoshee.png "Jacob Foshee")](https://github.com/jfoshee)
[![](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/Mrxx99.png "")](https://github.com/Mrxx99)
[![Eric Johnson](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/eajhnsn1.png "Eric Johnson")](https://github.com/eajhnsn1)
[![Ix Technologies B.V.](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/IxTechnologies.png "Ix Technologies B.V.")](https://github.com/IxTechnologies)
[![David JENNI](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/davidjenni.png "David JENNI")](https://github.com/davidjenni)
[![Jonathan ](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/Jonathan-Hickey.png "Jonathan ")](https://github.com/Jonathan-Hickey)
[![Oleg Kyrylchuk](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/okyrylchuk.png "Oleg Kyrylchuk")](https://github.com/okyrylchuk)
[![Charley Wu](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/akunzai.png "Charley Wu")](https://github.com/akunzai)
[![Jakob Tikjøb Andersen](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/jakobt.png "Jakob Tikjøb Andersen")](https://github.com/jakobt)
[![Seann Alexander](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/seanalexander.png "Seann Alexander")](https://github.com/seanalexander)
[![Tino Hager](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/tinohager.png "Tino Hager")](https://github.com/tinohager)
[![Mark Seemann](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/ploeh.png "Mark Seemann")](https://github.com/ploeh)
[![Angelo Belchior](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/angelobelchior.png "Angelo Belchior")](https://github.com/angelobelchior)
[![Ken Bonny](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/KenBonny.png "Ken Bonny")](https://github.com/KenBonny)
[![Simon Cropp](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/SimonCropp.png "Simon Cropp")](https://github.com/SimonCropp)
[![agileworks-eu](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/agileworks-eu.png "agileworks-eu")](https://github.com/agileworks-eu)
[![sorahex](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/sorahex.png "sorahex")](https://github.com/sorahex)
[![Zheyu Shen](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/arsdragonfly.png "Zheyu Shen")](https://github.com/arsdragonfly)
[![Vezel](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/vezel-dev.png "Vezel")](https://github.com/vezel-dev)
[![Michael Staib](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/michaelstaib.png "Michael Staib")](https://github.com/michaelstaib)
[![ChilliCream](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/ChilliCream.png "ChilliCream")](https://github.com/ChilliCream)
[![4OTC](https://raw.githubusercontent.com/devlooped/sponsors/main/.github/avatars/4OTC.png "4OTC")](https://github.com/4OTC)


<!-- sponsors.md -->

[![Sponsor this project](https://raw.githubusercontent.com/devlooped/sponsors/main/sponsor.png "Sponsor this project")](https://github.com/sponsors/devlooped)
&nbsp;

[Learn more about GitHub Sponsors](https://github.com/sponsors)

<!-- https://github.com/devlooped/sponsors/raw/main/footer.md -->
