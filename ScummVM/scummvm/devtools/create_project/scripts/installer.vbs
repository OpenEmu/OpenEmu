'
' ScummVM - Graphic Adventure Engine
'
' ScummVM is the legal property of its developers, whose names
' are too numerous to list here. Please refer to the COPYRIGHT
' file distributed with this source distribution.
'
' This program is free software; you can redistribute it and/or
' modify it under the terms of the GNU General Public License
' as published by the Free Software Foundation, version 2
' of the License.
'
' This program is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
' GNU General Public License for more details.
'
' You should have received a copy of the GNU General Public License
' along with this program; if not, write to the Free Software
' Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
'
'/

''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
' This script calls the makensis tool to generate a NSIS Windows installer for ScummVM
'
' It tries to read the NSIS installation folder from the registry and then calls the
' command line script compiler to create the installer.
'
' This is called from the postbuild.cmd batch file
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

'================================================================
' TODO: Reduce duplication with revision.vbs script 
'       (ReadRegistryKey and ParseCommandLine are identical)
'================================================================

Option Explicit

Dim FSO : Set FSO = CreateObject("Scripting.FileSystemObject")
Dim WshShell : Set WshShell = CreateObject("WScript.Shell")

' Folders
Dim rootFolder : rootFolder = ""
Dim targetFolder : targetFolder = ""
Dim arch : arch = ""

' Parse our command line arguments
If ParseCommandLine() Then
	CreateInstaller()
End If

'////////////////////////////////////////////////////////////////
'// Installer creation
'////////////////////////////////////////////////////////////////
Sub CreateInstaller()
	' Get nsis installation folder
	Dim nsisPath : nsisPath = GetNSISPath()
	If (nsisPath = "") Then
		Exit Sub
	End If

	' Preprocess architecture
	Select Case arch
	Case "x86"
		arch = "win32"

	Case "x64"
		arch = "win64"
	End Select

	' Build command line
	Dim commandLine : commandLine = """" & nsisPath & "\makensis.exe"" /V2" & _
	                                " /Dtop_srcdir=""" & rootFolder & """" & _
	                                " /Dstaging_dir=""" & targetFolder & """" & _
	                                " /DARCH=""" & arch & """" & _
	                                " """ & rootFolder & "\dists\win32\scummvm.nsi"""

	Dim oExec: Set oExec = WshShell.Exec(commandline)
	If Err.Number <> 0 Then
		Wscript.StdErr.WriteLine "Error running makensis.exe!"
		Exit Sub
	End If

	' Wait till the application is finished ...
	Dim ostdOut : Set oStdOut = oExec.StdOut
	Do While oExec.Status = 0
		If Not ostdOut.AtEndOfStream Then
			Wscript.StdErr.WriteLine ostdOut.ReadAll
		End If

		WScript.Sleep 100
	Loop

	If oExec.ExitCode <> 0 Then
		Wscript.StdErr.WriteLine "Error while creating installer!"
		Exit Sub
	End If
End Sub

Function GetNSISPath()
	' Get the directory where NSIS (should) reside(s)
	Dim sNSIS

	' First, try with 32-bit architecture
	sNSIS = ReadRegistryKey("HKLM", "SOFTWARE\NSIS", "", 32)

	If sNSIS = "" Or IsNull(sNSIS) Then
		' No 32-bit version of TortoiseSVN installed, try 64-bit version (doesn't hurt on 32-bit machines, it returns nothing or is ignored)
		sNSIS = ReadRegistryKey("HKLM", "SOFTWARE\NSIS", "", 64)
	End If

	' Check if Tortoise is present
	If sNSIS = "" Then
		Wscript.StdErr.WriteLine "NSIS not installed!"
		Exit Function
	End If

	GetNSISPath = sNSIS
End Function

'////////////////////////////////////////////////////////////////
'// Utilities
'////////////////////////////////////////////////////////////////
Function ParseCommandLine()
	ParseCommandLine = True

	If Wscript.Arguments.Count <> 3 Then
		Wscript.StdErr.WriteLine "[Error] Invalid number of arguments (was: " & Wscript.Arguments.Count & ", expected: 3)"

		ParseCommandLine = False
		Exit Function
	End If

	' Get our arguments
	rootFolder = Wscript.Arguments.Item(0)
	targetFolder = Wscript.Arguments.Item(1)
	arch = Wscript.Arguments.Item(2)

	' Check that the folders are valid
	If Not FSO.FolderExists(rootFolder) Then
		Wscript.StdErr.WriteLine "[Error] Invalid root folder (" & rootFolder & ")"

		ParseCommandLine = False
		Exit Function
	End If

	If Not FSO.FolderExists(targetFolder) Then
		Wscript.StdErr.WriteLine "[Error] Invalid target folder (" & targetFolder & ")"

		ParseCommandLine = False
		Exit Function
	End If

	' Set absolute paths
	rootFolder = FSO.GetAbsolutePathName(rootFolder)
	targetFolder = FSO.GetAbsolutePathName(targetFolder)
End Function

Function ReadRegistryKey(shive, subkey, valuename, architecture)
	Dim hiveKey, objCtx, objLocator, objServices, objReg, Inparams, Outparams

	' First, get the Registry Provider for the requested architecture
	Set objCtx = CreateObject("WbemScripting.SWbemNamedValueSet")
	objCtx.Add "__ProviderArchitecture", architecture ' Must be 64 of 32
	Set objLocator = CreateObject("Wbemscripting.SWbemLocator")
	Set objServices = objLocator.ConnectServer("","root\default","","",,,,objCtx)
	Set objReg = objServices.Get("StdRegProv")

	' Check the hive and give it the right value
	Select Case shive
		Case "HKCR", "HKEY_CLASSES_ROOT"
			hiveKey = &h80000000
		Case "HKCU", "HKEY_CURRENT_USER"
			hiveKey = &H80000001
		Case "HKLM", "HKEY_LOCAL_MACHINE"
			hiveKey = &h80000002
		Case "HKU", "HKEY_USERS"
			hiveKey = &h80000003
		Case "HKCC", "HKEY_CURRENT_CONFIG"
			hiveKey = &h80000005
		Case "HKDD", "HKEY_DYN_DATA" ' Only valid for Windows 95/98
			hiveKey = &h80000006
		Case Else
			MsgBox "Hive not valid (ReadRegistryKey)"
	End Select

	Set Inparams = objReg.Methods_("GetStringValue").Inparameters
	Inparams.Hdefkey = hiveKey
	Inparams.Ssubkeyname = subkey
	Inparams.Svaluename = valuename
	Set Outparams = objReg.ExecMethod_("GetStringValue", Inparams,,objCtx)

	ReadRegistryKey = Outparams.SValue
End Function
