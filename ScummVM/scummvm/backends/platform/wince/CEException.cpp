/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "CEException.h"

void CEException::writeString(HANDLE file, char *data) {
	DWORD dummy;
	WriteFile(file, data, strlen(data), &dummy, NULL);
	WriteFile(file, "\r\n", 2, &dummy, NULL);
}

void CEException::writeBreak(HANDLE file) {
	char tempo[100];
	int i;

	memset(tempo, 0, sizeof(tempo));
	for (i = 0; i < 40; i++)
		tempo[i] = '-';
	writeString(file, tempo);
}

void CEException::dumpContext(HANDLE file, HANDLE hProcess, CONTEXT *context) {
	char tempo[200];
	unsigned char memoryDump[100];
	DWORD size;
	unsigned int i;

#ifdef ARM
	writeBreak(file);
	writeString(file, "Context dump");
	sprintf(tempo, "R0=%.8x R1=%.8x R2=%.8x R3=%.8x R4=%.8x", context->R0, context->R1,
	        context->R2, context->R3, context->R4);
	writeString(file, tempo);
	sprintf(tempo, "R5=%.8x R6=%.8x R7=%.8x R8=%.8x R9=%.8x", context->R5, context->R6,
	        context->R7, context->R8, context->R9);
	writeString(file, tempo);
	sprintf(tempo, "R10=%.8x R11=%.8x R12=%.8x", context->R10, context->R11,
	        context->R12);
	writeString(file, tempo);
	sprintf(tempo, "Sp=%.8x Lr=%.8x Pc=%.8x Psr=%.8x", context->Sp, context->Lr,
	        context->Pc, context->Psr);
	writeString(file, tempo);
	writeBreak(file);

	sprintf(tempo, "Memory dump at %.8x", context->Pc - (sizeof(memoryDump) / 2));
	writeString(file, tempo);
	if (ReadProcessMemory(hProcess, (LPCVOID)(context->Pc - (sizeof(memoryDump) / 2)), memoryDump, sizeof(memoryDump), &size)) {
		for (i = 0; i < size; i += 8) {
			int j;
			char digit[4];
			int max;
			max = size - i;
			if (max > 8)
				max = 8;
			tempo[0] = '\0';
			for (j = 0; j < max; j++) {
				sprintf(digit, "%.2x ", memoryDump[i + j]);
				strcat(tempo, digit);
			}
			writeString(file, tempo);
		}
	}
#else
	writeBreak(file);
	writeString(file, "Context dump only available on ARM devices");
#endif
}

void CEException::dumpException(HANDLE file, EXCEPTION_RECORD *exceptionRecord) {
	char tempo[200];
	char exceptionName[50];
	unsigned int i;
#if (_WIN32_WCE >= 300)
	writeBreak(file);
	switch (exceptionRecord->ExceptionCode) {
	case EXCEPTION_ACCESS_VIOLATION :
		strcpy(exceptionName, "Access Violation");
		break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED :
		strcpy(exceptionName, "Array Bounds Exceeded");
		break;
	case EXCEPTION_DATATYPE_MISALIGNMENT :
		strcpy(exceptionName, "Datatype Misalignment");
		break;
	case EXCEPTION_IN_PAGE_ERROR :
		strcpy(exceptionName, "In Page Error");
		break;
	case EXCEPTION_INT_DIVIDE_BY_ZERO :
		strcpy(exceptionName, "Int Divide By Zero");
		break;
	case EXCEPTION_INT_OVERFLOW :
		strcpy(exceptionName, "Int Overflow");
		break;
	case EXCEPTION_STACK_OVERFLOW :
		strcpy(exceptionName, "Stack Overflow");
		break;
	default:
		sprintf(exceptionName, "%.8x", exceptionRecord->ExceptionCode);
		break;
	}
	sprintf(tempo, "Exception %s Flags %.8x Address %.8x", exceptionName, exceptionRecord->ExceptionFlags,
	        exceptionRecord->ExceptionAddress);
	writeString(file, tempo);
	if (exceptionRecord->NumberParameters) {
		for (i = 0; i < exceptionRecord->NumberParameters; i++) {
			sprintf(tempo, "Parameter %d %.8x", i, exceptionRecord->ExceptionInformation[i]);
			writeString(file, tempo);
		}
	}
	if (exceptionRecord->ExceptionRecord)
		dumpException(file, exceptionRecord->ExceptionRecord);
#else
	writeBreak(file);
	writeString(file, "Cannot get exception information on this CE version");
#endif
}

bool CEException::writeException(TCHAR *path, EXCEPTION_POINTERS *exceptionPointers) {
	HANDLE dumpFile;
	TCHAR dumpFileName[MAX_PATH];
	SYSTEMTIME systemTime;

	GetSystemTime(&systemTime);
	wsprintf(dumpFileName, TEXT("%s_%.2d_%.2d_%.4d_%.2d_%.2d_%.2d.txt"),
	         path, systemTime.wDay, systemTime.wMonth, systemTime.wYear,
	         systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
	dumpFile = CreateFile(dumpFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (dumpFile == INVALID_HANDLE_VALUE)
		return false;

	dumpException(dumpFile, exceptionPointers->ExceptionRecord);
	dumpContext(dumpFile, GetCurrentProcess(), exceptionPointers->ContextRecord);

	CloseHandle(dumpFile);

	return true;
}
