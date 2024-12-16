@echo off
setlocal enabledelayedexpansion

REM Set the command line encoding to UTF-8
chcp 65001

REM Output a message to confirm the script has started
echo Script execution started...

REM Set the absolute path to osgpmesh.exe
set "exePath=E:\work\2409\C++\osgpmeh\build\bin\osgpmesh.exe"

REM Start iterating over the files recursively in the directory
echo Starting to process files recursively...
for /r E:\work\2409\Data\NNU-MiniCIM_09_24\NNU-MiniCIM_09_24\NNU-MiniCIM_09_24 %%f in (*.osgb) do (
    REM Extract the filename (without path and extension)
    set "filename=%%~nf"
    
    REM Set the input path
    set "input=%%f"
    
    REM Set the output path
    set "output=E:\work\2409\Data\NNU-MiniCIM_09_24\out2\!filename!_9_3.osgb"
    
    REM Output the file being processed
    echo Processing file: !input!
    
    REM Execute the command and redirect output to a log file
    "%exePath%" 0.9 3 "!input!" "!output!" > output.log 2>&1
    
    REM Check if the command was successful
    if errorlevel 1 (
        echo Error: osgpmesh.exe execution failed, input file: !input!
    ) else (
        echo Success: Processed file !input!
    )
)

endlocal

REM Output a message that the script has finished
echo Script execution completed!

REM Keep the command window open to view the output
pause
