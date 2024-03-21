g++ -Iinc src/main.cpp src/FileParser.cpp src/FunctionParser.cpp src/Label.cpp src/x86FunctionBuilder.cpp src/FunctionBuilder.cpp src/JITEngine.cpp src/Scaner.cpp src/x64FunctionBuilder.cpp -o tc
./tc
