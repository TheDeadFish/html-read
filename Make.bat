:: build library
@call cmake_gcc %*
@libmerge %prefix%\%LIBX%\libexshit.a %BUILD_DIR%\libhtmlRead.a

:: copy include files
copy /Y src\*.h %PROGRAMS%\local\include
