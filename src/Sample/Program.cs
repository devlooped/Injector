using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading;

namespace Sample
{
    class Program
    {
        static void Main(string[] args)
        {
            var exe = args.FirstOrDefault() ?? @"C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\IDE\devenv.exe";

            var process = Process.Start(exe);
            process.WaitForInputIdle();
            // NOTE: it's important to wait until the window handle is actually created. 
            // this may take a while if the startup of the target app is lenghty
            while (process.MainWindowHandle == IntPtr.Zero)
            {
                Thread.Sleep(200);
            }

            // Inspect %LocalAppData%\Windows.Injector\log.txt to troubleshoot

            // Dynamic injection based on the target process bitness
            // using the external helper process.
            NativeMethods.IsWow64Process(process.Handle, out var isWow);
            var platform = isWow ? "x86" : "x64";

            Process.Start(Path.Combine("Injector", platform, "Injector.exe"), 
                process.MainWindowHandle + " " +
                Assembly.GetExecutingAssembly().Location + " " +
                typeof(Startup).FullName + " " +
                $"{nameof(Startup.Start)}:hello:42:true");

            // API-based injection if target process has same bitness as ours
            Bootstrap.Injector.Launch(
                process.MainWindowHandle,
                Assembly.GetExecutingAssembly().Location,
                typeof(Startup).FullName,
                $"{nameof(Startup.Start)}:hello:42:true");

            Console.ReadLine();
            
            if (!process.HasExited)
                process.Kill();
        }
    }

    public static class Startup
    {
        // NOTE: parameter type conversion from the {method}:arg1:argN format is done automatically
        public static void Start(string arg1, int arg2, bool debug)
        {
            if (debug)
                Debugger.Launch();
        }
    }

    internal static class NativeMethods
    {
        [DllImport("kernel32.dll", SetLastError = true, CallingConvention = CallingConvention.Winapi)]
        [return: MarshalAs(UnmanagedType.Bool)]
        internal static extern bool IsWow64Process([In] IntPtr process, [Out] out bool wow64Process);
    }
}
