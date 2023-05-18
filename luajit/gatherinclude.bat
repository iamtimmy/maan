@echo off

if not exist include (
	mkdir include 2>nul
)

copy .\luajit\src\lauxlib.h .\include\lauxlib.h
copy .\luajit\src\lua.h .\include\lua.h
copy .\luajit\src\lua.hpp .\include\lua.hpp
copy .\luajit\src\luaconf.h .\include\luaconf.h
copy .\luajit\src\luajit.h .\include\luajit.h
copy .\luajit\src\lualib.h .\include\lualib.h
