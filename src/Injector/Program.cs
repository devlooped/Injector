using System;

namespace Injector
{
    /// <summary>
    /// Helper process to allow injection into processes that have different bitness 
    /// into WPF app processes.
    ///
    /// VS is 32-bit, so it cannot write to the memory of a 64-bit process, and delegates
    /// that to this program instead.
    /// </summary>
    public class Program
    {
        static int Main(string[] args)
        {
            if (args.Length != 4)
            {
                Console.WriteLine("Usage: Injector.exe <mainWindowHandle> <assemblyFile> <typeName> <methodName>");
                Console.WriteLine();
                Console.WriteLine("Arguments:");
                Console.WriteLine("  <processMainWindowHandle>   IntPtr of the main window handle of the process to inject, i.e. Process.MainWindowHandle.");
                Console.WriteLine("  <assemblyFile>              The full path to the .NET assembly to load in the remote process.");
                Console.WriteLine("  <typeName>                  Full type name of the public static class to invoke in the remote process.");
                Console.WriteLine("  <methodName>                Name of the static method in that class to invoke in the remote process. Must be a ");
                Console.WriteLine("                              static method, which can also receive arguments, such as 'Start:true:42'.");
                return -1;
            }

            var mainWindow = new IntPtr(int.Parse(args[0]));
            var assemblyFile = args[1].Trim('"');
            var typeName = args[2].Trim('"');
            var methodName = args[3].Trim('"');

            Bootstrap.Injector.Launch(mainWindow, assemblyFile, typeName, methodName);
            return 0;
        }
    }
}
