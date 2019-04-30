using System;
using System.Diagnostics;

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
		static void Main(string[] args)
		{
			var handle = new IntPtr(int.Parse(args[0]));
			var supportDllPath = args[1].Trim('"');
			var className = args[2].Trim('"');
			var methodName = args[3].Trim('"');

			Bootstrap.Injector.Launch(handle, supportDllPath, className, methodName);
		}
	}
}
