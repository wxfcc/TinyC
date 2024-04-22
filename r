g++ -Wno-terminate -Iinc src/main.cpp src/FileParser.cpp src/FunctionParser.cpp src/Label.cpp src/Function.cpp src/FunctionX86.cpp src/FunctionX64.cpp src/FunctionX64Linux.cpp src/JITEngine.cpp src/Scaner.cpp src/Test.cpp -o tc
./tc
