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
; tempFile:=A_Temp . "\" . inputFileBasename . "2cb063a62e874d5f8fbe9cebb9a93881" . "." . inputFileExtension
; tempFile:=A_Temp . "\" . inputFileBasename  . "." . inputFileExtension





;Get the currently active window  
WinGet, originalActiveWindow, ID, A

WinGet, acadMainWindowHandle, ID, AutoCAD ahk_exe acad.exe

if (acadMainWindowHandle == "")
{
  MsgBox, "Acad needs to be running for this script to work. Please start Acad and try again."
	 ExitApp
}


; acadCommand:="(command-s "".NETLOAD""  " . """" . addSlashes(tempFile) . """" . ")(princ)" . " "

acadCommand:= "(mapcar"
    . "    'arxunload"
    . "    (vl-remove-if-not"
    . "        '(lambda (x) "
    . "            (="
    . "                (vl-string-mismatch"
    . "                    x"
    . "                    " . """" . inputFileBasename . """" . ""
    . "                )"
    . "                (strlen " . """" . inputFileBasename . """" . ")"
    . "            )"
    . "        )"
    . "        (arx)"
    . "    )"
    . ")"

MsgBox, %acadCommand%
; acadCommand:="(arxunload " . """" . addSlashes(tempFile) . """" . ")(princ)" . " "
WinActivate, ahk_id %acadMainWindowHandle%
Sleep, 100
Send {Esc}{Esc} ;;cancels any running autocad command
SendRaw %acadCommand%
;Send {Enter}
Sleep, 120



FileCopy, %inputFile%, %tempFile%


acadCommand:="(arxload " . """" . addSlashes(tempFile) . """" . ")(princ)" . " "
WinActivate, ahk_id %acadMainWindowHandle%
Sleep, 100
Send {Esc}{Esc} ;;cancels any running autocad command
SendRaw %acadCommand%
;Send {Enter}
Sleep, 120


WinActivate, ahk_id %originalActiveWindow%

addSlashes(x)
{
	return RegExReplace(x, "\\", "\\")
}

