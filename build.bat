windres VRChat-YT-DLP-Fix.rc -O coff -o resource.o
g++ VRChat-YT-DLP-Fix.cpp resource.o -o VRChat-YT-DLP-Fix.exe -std=c++20 -mwindows
pause