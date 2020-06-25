@echo off

dck32.exe %1 %2 %3 %4 %5 %6 %7 %8 %9

:checklevel
if errorlevel 200 goto exit
if errorlevel 100 goto play
:restart
@echo off
del _PLAY.BAT
echo Restarting DCK...
dck32.exe /quiet
goto checklevel

:play
mouse/s08
call _PLAY.BAT
mouse/s03
goto restart

:exit
echo.
