// (c) Copyright Cory Plotts.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

#include "stdafx.h"

#include "Injector.h"
#include <vcclr.h>

using namespace Bootstrap;

static unsigned int WM_GOBABYGO = ::RegisterWindowMessage(L"Injector_GOBABYGO!");
static HHOOK _messageHookHandle;

//-----------------------------------------------------------------------------
//Spying Process functions follow
//-----------------------------------------------------------------------------
void Injector::Launch(System::IntPtr windowHandle, System::String^ assembly, System::String^ className, System::String^ methodName)
{
	System::String^ assemblyClassAndMethod = assembly + "$" + className + "$" + methodName;
	pin_ptr<const wchar_t> acmLocal = PtrToStringChars(assemblyClassAndMethod);

	HINSTANCE hinstDLL;	

	if (::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)&MessageHookProc, &hinstDLL))
	{
		// Don't append the first call, so after each injection attempt, we get a clean log to inspect.
		LogMessage("GetModuleHandleEx successful", false);
		DWORD processID = 0;
		DWORD threadID = ::GetWindowThreadProcessId((HWND)windowHandle.ToPointer(), &processID);

		if (processID)
		{
			LogMessage("Got process id", true);
			HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
			if (hProcess)
			{
				LogMessage("Got process handle", true);
				int buffLen = (assemblyClassAndMethod->Length + 1) * sizeof(wchar_t);
				void* acmRemote = ::VirtualAllocEx(hProcess, NULL, buffLen, MEM_COMMIT, PAGE_READWRITE);

				if (acmRemote)
				{
					LogMessage("VirtualAllocEx successful", true);
					::WriteProcessMemory(hProcess, acmRemote, acmLocal, buffLen, NULL);
				
					_messageHookHandle = ::SetWindowsHookEx(WH_CALLWNDPROC, &MessageHookProc, hinstDLL, threadID);

					if (_messageHookHandle)
					{
						LogMessage("SetWindowsHookEx successful", true);
						::SendMessage((HWND)windowHandle.ToPointer(), WM_GOBABYGO, (WPARAM)acmRemote, 0);
						::UnhookWindowsHookEx(_messageHookHandle);
					}

					::VirtualFreeEx(hProcess, acmRemote, 0, MEM_RELEASE);
				}

				::CloseHandle(hProcess);
			}
		}
		::FreeLibrary(hinstDLL);
	}
}

void Injector::LogMessage(System::String^ message, bool append)
{
	System::String ^ applicationDataPath = Environment::GetFolderPath(Environment::SpecialFolder::LocalApplicationData);
	applicationDataPath += "\\Windows.Injector";

	if (!System::IO::Directory::Exists(applicationDataPath))
	{
		System::IO::Directory::CreateDirectory(applicationDataPath);
	}

	System::String ^ pathname = applicationDataPath + "\\log.txt";

	if (!append)
	{
		System::IO::File::Delete(pathname);
	}

	System::IO::FileInfo ^ fi = gcnew System::IO::FileInfo(pathname);

	System::IO::StreamWriter ^ sw = fi->AppendText();
	sw->WriteLine(System::DateTime::Now.ToString("yyyy/MM/dd HH:mm:ss") + " : " + message);
	sw->Close();
}

__declspec(dllexport) 
LRESULT __stdcall MessageHookProc(int nCode, WPARAM wparam, LPARAM lparam)
{
	if (nCode == HC_ACTION)
	{
		CWPSTRUCT* msg = (CWPSTRUCT*)lparam;
		if (msg != NULL && msg->message == WM_GOBABYGO)
		{
			Injector::LogMessage("Got WM_GOBABYGO message", true);

			wchar_t* acmRemote = (wchar_t*)msg->wParam;

			String^ acmLocal = gcnew System::String(acmRemote);
			Injector::LogMessage(System::String::Format("acmLocal = {0}", acmLocal), true);
			cli::array<System::String^>^ acmSplit = acmLocal->Split('$');
			cli::array<System::String^>^ methodSplit = acmSplit[2]->Split(':');

			cli::array<System::Object^>^ methodParams = nullptr;
			cli::array<System::Type^>^ methodParamTypes = nullptr;
			if (methodSplit->Length > 1) {
				methodParams = gcnew cli::array<System::Object^>(methodSplit->Length - 1);
				methodParamTypes = gcnew cli::array<System::Type^>(methodSplit->Length - 1);
				for (int i = 1; i < methodSplit->Length; i++) {
					methodParams[i - 1] = methodSplit[i];
					methodParamTypes[i - 1] = methodSplit[i]->GetType();
				}
			}

			Injector::LogMessage(String::Format("About to load assembly {0}", acmSplit[0]), true);
			System::Reflection::Assembly^ assembly = System::Reflection::Assembly::LoadFrom(acmSplit[0]);
			if (assembly != nullptr)
			{
				Injector::LogMessage(String::Format("About to load type {0}", acmSplit[1]), true);
				System::Type^ type = assembly->GetType(acmSplit[1]);
				if (type != nullptr)
				{
					Injector::LogMessage(String::Format("Just loaded the type {0}", acmSplit[1]), true);
					Injector::LogMessage(String::Format("Looking for full method {0}", acmSplit[2]), true);
					Injector::LogMessage(String::Format("Looking for method {0}", methodSplit[0]), true);
					System::Reflection::MethodInfo^ methodInfo = type->GetMethod(
						methodSplit[0],
						System::Reflection::BindingFlags::Static | System::Reflection::BindingFlags::Public,
						nullptr,
						methodParamTypes,
						nullptr);
					if (methodInfo != nullptr)
					{
						Injector::LogMessage(System::String::Format("About to invoke {0} on type {1}", methodInfo->Name, acmSplit[1]), true);
						methodInfo->Invoke(nullptr, methodParams);
					}
				}
			}
		}
	}
	return CallNextHookEx(_messageHookHandle, nCode, wparam, lparam);
}
