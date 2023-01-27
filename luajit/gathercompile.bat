@echo off
set platform=%1

if not exist %platform% (
  mkdir %platform% 2>nul
  mkdir %platform%\bin\ 2>nul
  mkdir %platform%\lib\ 2>nul
)

mv luajit\src\luajit.exe %platform%\bin\luajit.exe

mv luajit\src\lua51.lib %platform%\lib\lua51.lib
mv luajit\src\luajit.exp %platform%\lib\luajit.exp
mv luajit\src\luajit.lib %platform%\lib\luajit.lib



