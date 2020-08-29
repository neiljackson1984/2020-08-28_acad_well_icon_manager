; loads the file passed as an argument, in autocad.

#singleInstance FORCE
#NoEnv  ; Recommended for performance and compatibility with future AutoHotkey releases.
SendMode Input  ; Recommended for new scripts due to its superior speed and reliability.
;; SetWorkingDir %A_ScriptDir%  ; Ensures a consistent starting directory.


SetTitleMatchMode 2



;resolve fully qualified path to file
Loop, %1%
{
  inputFile=%A_LoopFileLongPath%
}
SplitPath, inputFile, inputFileName, inputFileDirectory, inputFileExtension, inputFileBasename
tempFile:=A_Temp . "\" . inputFileBasename . A_TickCount . "." . inputFileExtension
FileCopy, %inputFile%, %tempFile%

;Get the currently active window  
WinGet, originalActiveWindow, ID, A

WinGet, acadMainWindowHandle, ID, AutoCAD ahk_exe acad.exe

if (acadMainWindowHandle == "")
{
  MsgBox, "Acad needs to be running for this script to work. Please start Acad and try again."
	 ExitApp
}

WinActivate, ahk_id %acadMainWindowHandle%
Sleep, 100



acadCommand:="(command-s "".NETLOAD""  " . """" . addSlashes(tempFile) . """" . ")(princ)" . " "
; acadCommand:="(command-s "".NETLOAD""  " . """" . addSlashes(inputFile) . """" . ")(princ)" . " "

Send {Esc}{Esc} ;;cancels any running autocad command
SendRaw %acadCommand%
;Send {Enter}
Sleep, 120
;ControlSend, , asdfafasdfasdf, ahk_id %acadMainWindowHandle%

WinActivate, ahk_id %originalActiveWindow%

addSlashes(x)
{
	return RegExReplace(x, "\\", "\\")
}

