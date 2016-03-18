$env:CMAKE = $env:CMAKE_PATH + "\bin\cmake.exe"
if ( $env:PLATFORM -eq "x64" ) {
  $env:CMAKE_GENERATOR = "Visual Studio 12 2013 Win64"
  $env:CUSTOM_FLAG = "-DWIN64:Bool=True"
} else {
  $env:CMAKE_GENERATOR = "Visual Studio 12 2013"
  $env:CUSTOM_FLAG = ""
}

mkdir build
cd build

c:\projects\TapTools\cmake-3.4.1-win32-x86\bin\cmake.exe -G $env:CMAKE_GENERATOR $env:CUSTOM_FLAG .. > c:\projects\TapTools\configure.log
c:\projects\TapTools\cmake-3.4.1-win32-x86\bin\cmake.exe --build . --config Release > c:\projects\TapTools\build.log
