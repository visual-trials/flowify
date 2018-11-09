REM SET PROJECT_NAME=flowify
SET PROJECT_NAME=testing_text

cl /EHsc /O2 /Zi /Fddist\vc.pdb src\win32\main.cpp User32.lib d2d1.lib Dwrite.lib Msimg32.lib Comdlg32.lib /DINCLUDE_PROJECT_FILE="\"../%PROJECT_NAME%.cpp\"" /DBUILD_FOR_NATIVE_PLATFORM=1 /Fodist\%PROJECT_NAME%.obj /Fedist\%PROJECT_NAME%.exe