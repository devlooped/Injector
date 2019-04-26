using System;
using System.Diagnostics;
using System.Linq;
using System.Reflection;

namespace Sample
{
    class Program
    {
        static void Main(string[] args)
        {
            var exe = args.FirstOrDefault() ?? @"C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\Common7\IDE\devenv.exe";

            var process = Process.Start(exe);

            process.WaitForInputIdle();

            // Inspect %LocalAppData%\Windows.Injector\log.txt to troubleshoot

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
}
