@echo off
setlocal enabledelayedexpansion

REM 遍历E:\File\2408\osgb目录下的所有osgb文件
for %%f in (E:\work\2409\Data\NNU-MiniCIM\osgb\*.osgb) do (
    REM 提取文件名（不含路径和扩展名）
    set "filename=%%~nf"
    
    REM 设置输入路径
    set "input=%%f"
    
    REM 设置输出路径
    set "output=E:\work\2409\Data\NNU-MiniCIM\out\!filename!.osgb"
    
    REM 执行命令
    ..\build\bin\osgpmesh.exe 0.9 2 "!input!" "!output!"
)

endlocal
