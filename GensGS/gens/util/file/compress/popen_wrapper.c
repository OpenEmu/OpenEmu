/*
 * popen_wrapper.c: Provides a popen()-like function for Win32.
 * Adapted from http://lists.trolltech.com/qt-interest/1999-09/thread00282-0.html
 */


/*------------------------------------------------------------------------------
  Globals fuer die Routinen pt_popen() / pt_pclose()
------------------------------------------------------------------------------*/
#include <windows.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>

HANDLE my_pipein[2], my_pipeout[2], my_pipeerr[2];
char   my_popenmode = ' ';
PROCESS_INFORMATION piProcInfo;

static int my_pipe(HANDLE *readwrite)
{
	SECURITY_ATTRIBUTES sa;
	
	sa.nLength = sizeof(sa);          /* Laenge in Byte */
	sa.bInheritHandle = 1;            /* Descriptoren sollen vererbbar sein */
	sa.lpSecurityDescriptor = NULL;
	
	if (!CreatePipe(&readwrite[0], &readwrite[1], &sa, 1 << 13))
	{
		errno = EMFILE;
		return -1;
	}
	
	return 0;
}


/*------------------------------------------------------------------------------
  Ersatz fuer die Routine 'popen()' unter WIN32.
  ACHTUNG: Wenn 'cmd' den String '2>&1' enthaelt, wird der Standarderror File-
  Handle auf den Standardoutputfilehandle umgebogen.
------------------------------------------------------------------------------*/
FILE* gens_popen(const char *cmd, const char *mode)
{
	FILE *fptr = (FILE *)0;
	STARTUPINFO siStartInfo;
	int success, umlenkung;

	my_pipein[0]   = INVALID_HANDLE_VALUE;
	my_pipein[1]   = INVALID_HANDLE_VALUE;
	my_pipeout[0]  = INVALID_HANDLE_VALUE;
	my_pipeout[1]  = INVALID_HANDLE_VALUE;
	my_pipeerr[0]  = INVALID_HANDLE_VALUE;
	my_pipeerr[1]  = INVALID_HANDLE_VALUE;
	
	if (!mode || !*mode)
		goto finito;
	
	my_popenmode = *mode;
	if (my_popenmode != 'r' && my_popenmode != 'w')
		goto finito;
	
	/* Soll der stderr auf stdin umgelenkt werden ? */
	umlenkung = (strstr((char*)cmd, "2>&1") != 0);
	
	/* Erzeuge die Pipes... */
	if (my_pipe(my_pipein)  == -1 ||
		   my_pipe(my_pipeout) == -1)
		goto finito;
	if (!umlenkung && my_pipe(my_pipeerr) == -1)
		goto finito;
	
	/* Erzeuge jetzt den Sohnprozess */
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb           = sizeof(STARTUPINFO);
	siStartInfo.hStdInput    = my_pipein[0];
	siStartInfo.hStdOutput   = my_pipeout[1];
	if (umlenkung)
		siStartInfo.hStdError  = my_pipeout[1];
	else
		siStartInfo.hStdError  = my_pipeerr[1];
	siStartInfo.dwFlags    = STARTF_USESTDHANDLES;
	
	success = CreateProcess(NULL,
				(LPTSTR)cmd,       // command line 
				NULL,              // process security attributes 
				NULL,              // primary thread security attributes 
				TRUE,              // handles are inherited 
				DETACHED_PROCESS,  // creation flags: Ohne Fenster (?)
				NULL,              // use parent's environment 
				NULL,              // use parent's current directory 
				&siStartInfo,      // STARTUPINFO pointer 
				&piProcInfo);      // receives PROCESS_INFORMATION 
	
	if (!success)
		goto finito;
	
	/* Diese Handles gehoeren dem Sohnprozess */
	CloseHandle(my_pipein[0]);  my_pipein[0]  = INVALID_HANDLE_VALUE;
	CloseHandle(my_pipeout[1]); my_pipeout[1] = INVALID_HANDLE_VALUE;
	CloseHandle(my_pipeerr[1]); my_pipeerr[1] = INVALID_HANDLE_VALUE;
	
	if (my_popenmode == 'r')
		fptr = _fdopen(_open_osfhandle((long)my_pipeout[0],_O_BINARY),"r");
	else
		fptr = _fdopen(_open_osfhandle((long)my_pipein[1],_O_BINARY),"w");
	
finito:
	if (!fptr)
	{
		if (my_pipein[0]  != INVALID_HANDLE_VALUE)
			CloseHandle(my_pipein[0]);
		if (my_pipein[1]  != INVALID_HANDLE_VALUE)
			CloseHandle(my_pipein[1]);
		if (my_pipeout[0] != INVALID_HANDLE_VALUE)
			CloseHandle(my_pipeout[0]);
		if (my_pipeout[1] != INVALID_HANDLE_VALUE)
			CloseHandle(my_pipeout[1]);
		if (my_pipeerr[0] != INVALID_HANDLE_VALUE)
			CloseHandle(my_pipeerr[0]);
		if (my_pipeerr[1] != INVALID_HANDLE_VALUE)
			CloseHandle(my_pipeerr[1]);
	}
	return fptr;
}

/*------------------------------------------------------------------------------
  Ersatz fuer die Routine 'pclose()' unter WIN32
------------------------------------------------------------------------------*/
int gens_pclose(FILE *fle)
{
	if (fle)
	{
		(void)fclose(fle);
		
		CloseHandle(my_pipeerr[0]);
		if (my_popenmode == 'r')
			CloseHandle(my_pipein[1]);
		else
			CloseHandle(my_pipeout[0]);
		
		// Terminate the child process.
		if (piProcInfo.hProcess)
		{
			TerminateProcess(piProcInfo.hProcess, 0);
			piProcInfo.hProcess = NULL;
		}
		
		return 0;
	}
	
	// Pipe isn't open.
	return -1;
}
