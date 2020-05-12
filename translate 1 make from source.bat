rem this batch file assumes you have xgettext utility program installed
rem in C:\Program Files (x86)\GnuWin32\bin
rem if not, correct program location accordingly

rem How to make .po file for new language:
rem    create new language dir heekscad/translations/<langcode>
rem    change dir to freshly created (cd <langcode>)
rem    launch translate_1_make_from_source.bat (this batch script)
rem    then go to step 2 (translate_2_...bat)

"C:\Program Files (x86)\GnuWin32\bin\xgettext.exe" -C -n -k_ -o HeeksCAD.po ..\..\src\*.cpp ..\..\src\*.h ..\..\interface\*.cpp ..\..\interface\*.h ..\..\PyHeeksCAD\*.cpp ..\..\PyHeeksCAD\*.h ..\..\..\heekscnc\src\*.cpp ..\..\..\heekscnc\src\*.h

pause
