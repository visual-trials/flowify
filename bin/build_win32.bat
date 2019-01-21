@echo off
REM SET PROJECT_NAME=flowify
REM SET PROJECT_NAME=testing/testing_file
REM SET PROJECT_NAME=testing/testing_parser
REM SET PROJECT_NAME=testing/testing_memory
REM SET PROJECT_NAME=testing/testing_text
REM SET PROJECT_NAME=testing/testing_animation
REM SET PROJECT_NAME=testing/testing_keyboard
REM SET PROJECT_NAME=testing/testing_touch
REM SET PROJECT_NAME=testing/testing_mouse
SET PROJECT_NAME=testing/testing_renderer

REM TODO: add more warnings!

REM TODO: we turned off: C4576 "a parenthesized type followed by an initializer list is a non-standard explicit type conversion syntax"
REM       how should we initialize struct constants?
cl /EHsc /Od /Zi -wd4576 /Fddist\vc.pdb src\platform\win32_main.cpp User32.lib d2d1.lib Dwrite.lib Msimg32.lib Comdlg32.lib /DINCLUDE_PROJECT_FILE="\"../%PROJECT_NAME%.c\"" /Fodist\%PROJECT_NAME%.obj /Fedist\%PROJECT_NAME%.exe

REM mt.exe -manifest bin\dpi_aware.manifest -outputresource:dist\%PROJECT_NAME%.exe;1
