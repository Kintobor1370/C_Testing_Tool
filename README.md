# C Software Testing Tool
A tool for testing C code functions.\
This app requires the Z3 library to run.\
\
_Tested on Windows 10 with Visual Studio 2022._
\
To run the testing tool on Windows:
1. Download this repository
2. Download the latest 'x64-win' zip from the [official Z3 GitHub release page](https://github.com/Z3Prover/z3/releases)
4. Create a 'z3' folder in the testing tool root, and extract all contents of the downloaded z3 zip archive there
5. Create an 'x64\Debug' directory, copy the 'libz3.dll' and 'libz3.lib' files from the 'z3\bin' folder, and paste them into 'x64\Debug'
6. Open 'C_Testing_Tool.sln'
7. Press Ctrl + F5 to run the testing tool
