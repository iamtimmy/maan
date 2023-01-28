@echo off

if not exist include (
	mkdir include 2>nul
)


cp .\luajit\src\lauxlib.h .\include\lauxlib.h
cp .\luajit\src\lua.h .\include\lua.h
cp .\luajit\src\lua.hpp .\include\lua.hpp
cp .\luajit\src\luaconf.h .\include\luaconf.h
cp .\luajit\src\luajit.h .\include\luajit.h
cp .\luajit\src\lualib.h .\include\lualib.h
