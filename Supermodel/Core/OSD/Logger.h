/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/

/*
 * Logger.h
 *
 * Header file for message logging. The OSD code is expected to set up a
 * default logger (CFileLogger). 
 */
 
#ifndef INCLUDED_LOGGER_H
#define INCLUDED_LOGGER_H

#include "Types.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>


/******************************************************************************
 Class Definitions
******************************************************************************/

/*
 * CLogger
 * 
 * Abstract class that receives log messages from Supermodel's log functions.
 * The logger object handles actual output of messages. Use the function-based
 * message logging interface to generate messages.
 */
class CLogger
{
public:
	/*
	 * DebugLog(fmt, ...):
	 * DebugLog(fmt, vl):
	 *
	 * Prints to debug log. If DEBUG is not defined, will end up doing nothing. 
	 *
	 * Parameters:
	 *		fmt		printf()-style format string.
	 *		...		Variable number of parameters, corresponding to format 
	 *				string.
	 *		vl		Variable arguments already stored in a list.
	 */
	void DebugLog(const char *fmt, ...)
	{
		va_list vl;
		va_start(vl, fmt);
		DebugLog(fmt, vl);
		va_end(vl);
	}
	
	virtual void DebugLog(const char *fmt, va_list vl) = 0;

	/*
	 * InfoLog(fmt, ...):
	 * InfoLog(fmt, vl):
	 *
	 * Prints to error log but does not output an error to stderr. This is
	 * useful for printing session information to the error log.
	 *
	 * Parameters:
	 *		fmt		printf()-style format string.
	 *		...		Variable number of parameters, corresponding to format 
	 *				string.
	 *		vl		Variable arguments already stored in a list.
	 */
	void InfoLog(const char *fmt, ...)
	{
		va_list vl;
		va_start(vl, fmt);
		InfoLog(fmt, vl);
		va_end(vl);
	}
	
	virtual void InfoLog(const char *fmt, va_list vl) = 0;

	/*
	 * ErrorLog(fmt, ...):
	 * ErrorLog(fmt, vl):
	 *
	 * Prints to error log and outputs an error message to stderr.
	 *
	 * Parameters:
	 *		fmt		printf()-style format string.
	 *		...		Variable number of parameters, corresponding to format 
	 *				string.
	 *		vl		Variable arguments already stored in a list.
	 */
	void ErrorLog(const char *fmt, ...)
	{
		va_list vl;
		va_start(vl, fmt);
		ErrorLog(fmt, vl);
		va_end(vl);
	}
	
	virtual void ErrorLog(const char *fmt, va_list vl) = 0;
};

/*
 * CFileLogger:
 *  
 * Default logger that logs to debug and error log files. Files are opened and
 * closed for each message in order to preserve contents in case of program
 * crash.
 */ 
class CFileLogger : public CLogger
{
public:
	
	void DebugLog(const char *fmt, va_list vl)
	{	
#ifdef DEBUG
		char	string[1024];
		FILE 	*fp;

		fp = fopen(m_debugLogFile, "a");
		if (NULL != fp)
		{
			vsprintf(string, fmt, vl);
			fprintf(fp, string);
			fclose(fp);
		}
#endif // DEBUG
	}

	void InfoLog(const char *fmt, va_list vl)
	{
		char	string[4096];
		FILE 	*fp;

		vsprintf(string, fmt, vl);
		
		fp = fopen(m_errorLogFile, "a");
		if (NULL != fp)
		{
			fprintf(fp, "%s\n", string);
			fclose(fp);
		}
	
		CLogger::DebugLog("Info: ");
		CLogger::DebugLog(string);
		CLogger::DebugLog("\n");
	}

	void ErrorLog(const char *fmt, va_list vl)
	{
		char	string[4096];
		FILE 	*fp;

		vsprintf(string, fmt, vl);
		fprintf(stderr, "Error: %s\n", string);
		
		fp = fopen(m_errorLogFile, "a");
		if (NULL != fp)
		{
			fprintf(fp, "%s\n", string);
			fclose(fp);
		}
		
		CLogger::DebugLog("Error: ");
		CLogger::DebugLog(string);
		CLogger::DebugLog("\n");
	}

	/*
	 * ClearLogs():
	 *
	 * Clears all log files.
	 */
	void ClearLogs(void)
	{
#ifdef DEBUG
		ClearLog(m_debugLogFile, "Supermodel v"SUPERMODEL_VERSION" Debug Log");
#endif // DEBUG
		ClearLog(m_errorLogFile, "Supermodel v"SUPERMODEL_VERSION" Error Log");	
	}
	
	/*
	 * ClearLog(file, title):
	 *
	 * Clears a log file.
	 *
	 * Parameters:
	 *		file	File name.
	 *		title	A string that is written to the file after it is cleared.
	 */
	void ClearLog(const char *file, const char *title)
	{
		FILE *fp = fopen(file, "w");
		if (NULL != fp)
		{
			unsigned	i;
			fprintf(fp, "%s\n", title);
			for (i = 0; i < strlen(title); i++)
				fputc('-', fp);
			fprintf(fp, "\n\n");
			fclose(fp);
		}
	}
	
	/*
	 * CFileLogger(debugLogFile, errorLogFile):
	 *
	 * Constructor. Specifies debug and error log files to use.
	 */
	CFileLogger(const char *debugLogFile, const char *errorLogFile) : 
		m_debugLogFile(debugLogFile), m_errorLogFile(errorLogFile)
	{
	}

private:
	const char *m_debugLogFile;
	const char *m_errorLogFile;
};


/******************************************************************************
 Log Functions
 
 Message logging interface. All messages are passed by the OSD layer to the
 currently active logger object.
******************************************************************************/

/*
 * DebugLog(fmt, ...):
 *
 * Prints debugging information. The OSD layer may choose to print this to a
 * log file, the screen, neither, or both. Newlines and other formatting codes
 * must be explicitly included.
 *
 * Parameters:
 *		fmt		A format string (the same as printf()).
 *		...		Variable number of arguments, as required by format string.
 */
extern void	DebugLog(const char *fmt, ...);

/*
 * ErrorLog(fmt, ...):
 *
 * Prints error information. Errors need not require program termination and
 * may simply be informative warnings to the user. Newlines should not be 
 * included in the format string -- they are automatically added at the end of
 * a line.
 *
 * Parameters:
 *		fmt		A format string (the same as printf()).
 *		...		Variable number of arguments, as required by format string.
 *
 * Returns:
 *		Must always return FAIL.
 */
extern bool	ErrorLog(const char *fmt, ...);

/*
 * InfoLog(fmt, ...);
 *
 * Prints information to the error log file but does not print to stderr. This
 * is useful for logging non-error information. Newlines are automatically
 * appended.
 *
 * Parameters:
 *		fmt		Format string (same as printf()).
 *		...		Variable number of arguments as required by format string.
 */
extern void InfoLog(const char *fmt, ...);

/*
 * SetLogger(Logger):
 *
 * Sets the logger object to use.
 *
 * Parameters:
 *		Logger	New logger object. If NULL, log messages will not be output.
 */
extern void SetLogger(CLogger *Logger);

/*
 * GetLogger(void):
 *
 * Returns:
 *		Current logger object (NULL if none has been set).
 */
extern CLogger *GetLogger(void);


#endif	// INCLUDED_LOGGER_H
