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
[![Clarius Org](https://avatars.githubusercontent.com/u/71888636?v=4&s=39 "Clarius Org")](https://github.com/clarius)
[![MFB Technologies, Inc.](https://avatars.githubusercontent.com/u/87181630?v=4&s=39 "MFB Technologies, Inc.")](https://github.com/MFB-Technologies-Inc)
[![Khamza Davletov](https://avatars.githubusercontent.com/u/13615108?u=11b0038e255cdf9d1940fbb9ae9d1d57115697ab&v=4&s=39 "Khamza Davletov")](https://github.com/khamza85)
[![SandRock](https://avatars.githubusercontent.com/u/321868?u=99e50a714276c43ae820632f1da88cb71632ec97&v=4&s=39 "SandRock")](https://github.com/sandrock)
[![DRIVE.NET, Inc.](https://avatars.githubusercontent.com/u/15047123?v=4&s=39 "DRIVE.NET, Inc.")](https://github.com/drivenet)
[![Keith Pickford](https://avatars.githubusercontent.com/u/16598898?u=64416b80caf7092a885f60bb31612270bffc9598&v=4&s=39 "Keith Pickford")](https://github.com/Keflon)
[![Thomas Bolon](https://avatars.githubusercontent.com/u/127185?u=7f50babfc888675e37feb80851a4e9708f573386&v=4&s=39 "Thomas Bolon")](https://github.com/tbolon)
[![Kori Francis](https://avatars.githubusercontent.com/u/67574?u=3991fb983e1c399edf39aebc00a9f9cd425703bd&v=4&s=39 "Kori Francis")](https://github.com/kfrancis)
[![Reuben Swartz](https://avatars.githubusercontent.com/u/724704?u=2076fe336f9f6ad678009f1595cbea434b0c5a41&v=4&s=39 "Reuben Swartz")](https://github.com/rbnswartz)
[![Jacob Foshee](https://avatars.githubusercontent.com/u/480334?v=4&s=39 "Jacob Foshee")](https://github.com/jfoshee)
[![](https://avatars.githubusercontent.com/u/33566379?u=bf62e2b46435a267fa246a64537870fd2449410f&v=4&s=39 "")](https://github.com/Mrxx99)
[![Eric Johnson](https://avatars.githubusercontent.com/u/26369281?u=41b560c2bc493149b32d384b960e0948c78767ab&v=4&s=39 "Eric Johnson")](https://github.com/eajhnsn1)
[![Jonathan ](https://avatars.githubusercontent.com/u/5510103?u=98dcfbef3f32de629d30f1f418a095bf09e14891&v=4&s=39 "Jonathan ")](https://github.com/Jonathan-Hickey)
[![Ken Bonny](https://avatars.githubusercontent.com/u/6417376?u=569af445b6f387917029ffb5129e9cf9f6f68421&v=4&s=39 "Ken Bonny")](https://github.com/KenBonny)
[![Simon Cropp](https://avatars.githubusercontent.com/u/122666?v=4&s=39 "Simon Cropp")](https://github.com/SimonCropp)
[![agileworks-eu](https://avatars.githubusercontent.com/u/5989304?v=4&s=39 "agileworks-eu")](https://github.com/agileworks-eu)
[![Zheyu Shen](https://avatars.githubusercontent.com/u/4067473?v=4&s=39 "Zheyu Shen")](https://github.com/arsdragonfly)
[![Vezel](https://avatars.githubusercontent.com/u/87844133?v=4&s=39 "Vezel")](https://github.com/vezel-dev)
[![ChilliCream](https://avatars.githubusercontent.com/u/16239022?v=4&s=39 "ChilliCream")](https://github.com/ChilliCream)
[![4OTC](https://avatars.githubusercontent.com/u/68428092?v=4&s=39 "4OTC")](https://github.com/4OTC)
[![domischell](https://avatars.githubusercontent.com/u/66068846?u=0a5c5e2e7d90f15ea657bc660f175605935c5bea&v=4&s=39 "domischell")](https://github.com/DominicSchell)
[![Adrian Alonso](https://avatars.githubusercontent.com/u/2027083?u=129cf516d99f5cb2fd0f4a0787a069f3446b7522&v=4&s=39 "Adrian Alonso")](https://github.com/adalon)
[![torutek](https://avatars.githubusercontent.com/u/33917059?v=4&s=39 "torutek")](https://github.com/torutek)
[![mccaffers](https://avatars.githubusercontent.com/u/16667079?u=110034edf51097a5ee82cb6a94ae5483568e3469&v=4&s=39 "mccaffers")](https://github.com/mccaffers)
[![Seika Logiciel](https://avatars.githubusercontent.com/u/2564602?v=4&s=39 "Seika Logiciel")](https://github.com/SeikaLogiciel)
[![Andrew Grant](https://avatars.githubusercontent.com/devlooped-user?s=39 "Andrew Grant")](https://github.com/wizardness)
[![eska-gmbh](https://avatars.githubusercontent.com/devlooped-team?s=39 "eska-gmbh")](https://github.com/eska-gmbh)


<!-- sponsors.md -->
[![Sponsor this project](https://avatars.githubusercontent.com/devlooped-sponsor?s=118 "Sponsor this project")](https://github.com/sponsors/devlooped)

[Learn more about GitHub Sponsors](https://github.com/sponsors)

<!-- https://github.com/devlooped/sponsors/raw/main/footer.md -->
