# maan
`maan` is a C++ library binding to LuaJIT. It attempts to make use of modern C++ features in hopes to achieve a similar
feature set to [Sol2](https://github.com/ThePhD/sol2/), while maintaining a much more reasonable size. (Both binary and
Codebase size)

# Cloning
The project uses the [LuaJIT GitHub mirror](https://github.com/LuaJIT/LuaJIT.git) as a git submodule. 
```commandline
git clone --recurse-submodules https://github.com/iamtimmy/maan.git
```
or
```commandline
git clone https://github.com/iamtimmy/maan.git
git submodule update --init --recursive
```

# Building on Windows
1. Install Visual Studio 2022 [link](https://visualstudio.microsoft.com/downloads/)
2. Open a `Native Tools Command Prompt for VS 2022` for your target architecture (`x86` or `x64`)
3. Navigate to `<clone location>/luajit/LuaJIT/src/`
   - Optional: Have a look at the build script and the one in `<clone location>/luajit/msvcbuild.bat`
     for some extra compiler options
4. compile with `.\msvcbuild.bat static`
5. Navigate to `<clone location>/luajit/`
6. gather the required headers with `.\gatherinclude.bat`
7. gather the compiler output with `.\gathercompile.bat <target architecture>`

# Building on Linux
Linux support is currently untested
The library is written in standard C++, please submit an issue if anything comes up

# Building on other platforms:
I cannot test this currently, please submit issues/pull requests to fix problems as they come up

# References
- Videos
  - [Killing C++ Serialization Overhead & Complexity - Eyal Zedaka - CppCon 2022](https://youtu.be/G7-GQhCw8eE)
- Documentation
  - [Lua Manual - 5.1](https://www.lua.org/manual/5.1/manual.html)
  - [Lua Manual - 5.2](https://www.lua.org/manual/5.2/manual.html)
  - [LuaJit Documentation](https://repo.or.cz/w/luajit-2.0.git/blob_plain/v2.1:/doc/luajit.html)
  - [LuaJit Wiki](http://web.archive.org/web/20220517052639/http://wiki.luajit.org/Home)