# C Software Testing Tool
A console-based C code testing tool for Windows. This tool can:
- Detect lexical and syntactic errors
- Analyze execution paths and generate respective test cases
- Identify unreachable code segments
- Produce bug reports and code coverage metrics

This app requires the Z3 library to run.\

Examples of the testing tool output:
- Lexical error detection <br> <img width="392" height="102" alt="source1" src="https://github.com/user-attachments/assets/7f809363-4df7-4564-9a11-b4746fe3e679" /> <br> <img width="632" height="108" alt="output1" src="https://github.com/user-attachments/assets/61f1ef64-db04-4932-be18-5e8d61120bb3" />
- Syntactic error detection <br> <img width="392" height="129" alt="source2" src="https://github.com/user-attachments/assets/d729d756-1a5a-4e30-9c07-f3482bcd5a70" /> <br> <img width="477" height="121" alt="output2" src="https://github.com/user-attachments/assets/e1696433-84a9-4797-87dd-1ffbe9f63c0e" />
- Semantic error detection <br> <img width="539" height="159" alt="source3" src="https://github.com/user-attachments/assets/baecc52a-55b7-48e5-b2e9-9b8e5b26bfd3" /> <br> <img width="721" height="228" alt="output3" src="https://github.com/user-attachments/assets/58f47eda-b3a5-4a4d-9034-089a7ec71285" />
- Unreachable code report <br> <img width="484" height="389" alt="source4" src="https://github.com/user-attachments/assets/460c86c0-a007-4847-a15b-89e7d1f18b00" /> <br> <img width="488" height="259" alt="output4" src="https://github.com/user-attachments/assets/7bd21084-3446-41a9-87f3-40d3acfce84f" />
- Full code coverage report <br> <img width="423" height="324" alt="source5" src="https://github.com/user-attachments/assets/efac8082-8ba5-49b8-b4cb-c61a0f34ebf2" /> <br> <img width="481" height="213" alt="output5" src="https://github.com/user-attachments/assets/4bf7cd7a-2f6e-46fd-84ed-973ec355dd8b" />


_Tested on Windows 10 with Visual Studio 2022._


# __To run the testing tool on Windows:__
1. Download this repository
2. Download the latest 'x64-win' zip from the [official Z3 GitHub release page](https://github.com/Z3Prover/z3/releases)
4. Create a 'z3' directory in the testing tool root, and extract all contents of the downloaded z3 zip archive there
5. Create a 'x64\Debug' directory, copy the 'libz3.dll' and 'libz3.lib' files from the 'z3\bin' folder, and paste them into 'x64\Debug'
6. Open _C_Testing_Tool.sln_
7. Press Ctrl + F5 to run the testing tool
