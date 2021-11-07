cd "C:\Documents and Settings\Mizu\Meus documentos\FEI\TCC\LATEX\Manual"
set FOLDER_EN=C:\Program Files
set FOLDER_BR=C:\Arquivos de Programas
set FOLDER_BASE=MiKTex 2.7\miktex\bin\

if exist "%FOLDER_BR%" goto br
if exist "%FOLDER_EN%" goto en

:br
set FOLDER=%FOLDER_BR%\%FOLDER_BASE%
goto main

:en
set FOLDER=%FOLDER_EN%\%FOLDER_BASE%
goto main

:main
"%FOLDER%\pdflatex.exe" "Manual_Master.tex"
"%FOLDER%\bibtex.exe" "Manual"
"%FOLDER%\pdflatex.exe" "Manual_Master.tex"
"%FOLDER%\pdflatex.exe" "Manual_Master.tex"
taskkill /IM "FOXITR~1.EXE"
Manual_Master.pdf
