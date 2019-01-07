@echo off
SET PROJECT_NAME=flowify
REM SET PROJECT_NAME=testing_parser

REM TODO: add more warnings!

REM TODO: we turned off: C4576 "a parenthesized type followed by an initializer list is a non-standard explicit type conversion syntax"
REM       how should we initialize struct constants?
cl /EHsc /Od /Zi -wd4576 /Fddist\vc.pdb src\win32\main.cpp User32.lib d2d1.lib Dwrite.lib Msimg32.lib Comdlg32.lib /DINCLUDE_PROJECT_FILE="\"../%PROJECT_NAME%.cpp\"" /DBUILD_FOR_NATIVE_PLATFORM=1 /Fodist\%PROJECT_NAME%.obj /Fedist\%PROJECT_NAME%.exe

REM mt.exe -manifest bin\dpi_aware.manifest -outputresource:dist\%PROJECT_NAME%.exe;1
