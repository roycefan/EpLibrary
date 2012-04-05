/*! 
ConsoleHelper for the EpLibrary
Copyright (C) 2012  Woong Gyu La <juhgiyo@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "epConsoleHelper.h"

using namespace epl;

EpTString ConsoleHelper::ExecuteConsoleCommand(const TCHAR * command)
{
	EpTString csExecute;
	csExecute=command;

	SECURITY_ATTRIBUTES secattr;
	ZeroMemory(&secattr,sizeof(secattr));
	secattr.nLength=sizeof(secattr);
	secattr.bInheritHandle=TRUE;

	HANDLE rPipe, wPipe;

	CreatePipe(&rPipe,&wPipe,&secattr,0);

	STARTUPINFO sInfo;
	ZeroMemory(&sInfo, sizeof(sInfo));
	PROCESS_INFORMATION pInfo;
	ZeroMemory(&pInfo,sizeof(pInfo));
	sInfo.cb=sizeof(sInfo);
	sInfo.dwFlags=STARTF_USESTDHANDLES;
	sInfo.hStdInput=NULL;
	sInfo.hStdOutput=wPipe;
	sInfo.hStdError=wPipe;

	CreateProcess(0,reinterpret_cast<LPWSTR>(const_cast<wchar_t*>(csExecute.c_str())),0,0,TRUE,NORMAL_PRIORITY_CLASS|CREATE_NO_WINDOW,0,0,&sInfo,&pInfo);
	CloseHandle(wPipe);

	TCHAR buf[100];
	unsigned long reDword;
	EpTString m_csOutput,csTemp;
	long res;
	do
	{
		res=::ReadFile(rPipe,buf,100,&reDword,0);
		csTemp=buf;
		m_csOutput+=csTemp.substr(0,reDword);
	}while(res);
	return m_csOutput;

}

void ConsoleHelper::ExecuteProgram(const TCHAR * execFilePath)
{
	ShellExecute(0,0,execFilePath,0,0,SW_SHOWNORMAL);
}