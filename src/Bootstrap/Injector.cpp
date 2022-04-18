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
void Injector::Launch(System::IntPtr windowHandle, System::String^ assemblyFile, System::String^ typeFullName, System::String^ methodName)
{
	System::String^ assemblyClassAndMethod = assemblyFile + "$" + typeFullName + "$" + methodName;
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
        else
        {
            if (windowHandle == IntPtr::Zero)
                LogMessage("Invalid window handle received", true);
            else
                LogMessage("Could not get process from window handle " + windowHandle.ToString(), true);
        }
		::FreeLibrary(hinstDLL);
	}
}

void Injector::LogMessage(String^ message, bool append)
{
	String^ applicationDataPath = Environment::GetFolderPath(Environment::SpecialFolder::LocalApplicationData);
	applicationDataPath += "\\Devlooped\\Injector";

	if (!System::IO::Directory::Exists(applicationDataPath))
	{
		System::IO::Directory::CreateDirectory(applicationDataPath);
	}

	String ^ pathname = applicationDataPath + "\\log.txt";

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

			Injector::LogMessage(String::Format("About to load assembly {0}", acmSplit[0]), true);
			System::Reflection::Assembly^ assembly = System::Reflection::Assembly::LoadFrom(acmSplit[0]);
			if (assembly != nullptr)
			{
				Injector::LogMessage(String::Format("About to load type {0}", acmSplit[1]), true);
				System::Type^ type = assembly->GetType(acmSplit[1]);
				if (type != nullptr)
				{
					Injector::LogMessage(String::Format("Just loaded the type {0}", acmSplit[1]), true);
					// Injector::LogMessage(String::Format("Looking for full method and parameters {0}", acmSplit[2]), true);
					Injector::LogMessage(String::Format("Looking for method named {0}", methodSplit[0]), true);

					System::Reflection::MethodInfo^ methodInfo = type->GetMethod(
						methodSplit[0],
						System::Reflection::BindingFlags::Static | System::Reflection::BindingFlags::Public);

					if (methodInfo != nullptr)
					{
                        if (methodSplit->Length > 1)
                        {
                            cli::array<System::Reflection::ParameterInfo^>^ parameters = methodInfo->GetParameters();
                            if (methodSplit->Length - 1 != parameters->Length)
                            {
                                Injector::LogMessage(System::String::Format("Did not receive the expected {0} parameters to invoke {1}.{2}", parameters->Length, acmSplit[1], methodSplit[0]), true);
                            }
                            else
                            {
                                Injector::LogMessage(System::String::Format("Converting {0} received method arguments", methodSplit->Length - 1), true);
                                cli::array<System::Object^>^ methodParams = nullptr;

                                methodParams = gcnew cli::array<System::Object^>(parameters->Length);
                                for (int i = 0; i < parameters->Length; i++) {
                                    System::Type^ paramType = parameters[i]->ParameterType;
                                    System::ComponentModel::TypeConverter^ converter = System::ComponentModel::TypeDescriptor::GetConverter(paramType);
                                    // NOTE: take into account that the first part of the methodSplit is the method name.
                                    methodParams[i] = converter->ConvertFromString(methodSplit[i + 1]);
                                }

                                Injector::LogMessage(System::String::Format("Invoking {0}.{1}({2})",
                                    acmSplit[1],
                                    methodInfo->Name,
                                    System::String::Join(", ", methodSplit, 1, methodSplit->Length - 1)), true);
                                methodInfo->Invoke(nullptr, methodParams);
                            }
                        }
                        else
                        {
                            Injector::LogMessage(System::String::Format("Invoking {0}.{1}()", acmSplit[1], methodInfo->Name), true);
                            methodInfo->Invoke(nullptr, nullptr);
                        }
					}
                    else
                    {
                        Injector::LogMessage(System::String::Format("Did not find method named {0} on type {1}", methodSplit[0], acmSplit[1]), true);
                    }
				}
			}
		}
	}
	return CallNextHookEx(_messageHookHandle, nCode, wparam, lparam);
}
