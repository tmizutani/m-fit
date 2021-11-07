set PATH=%PATH%;C:\Dev-cpp\bin;c:\Qt\4.4.3\bin\;
set MAKE=make
taskkill /IM "MFIT.EXE"

qmake
%MAKE%
