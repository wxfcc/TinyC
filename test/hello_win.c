int hello(){
    puts("hello mod");
}
int main()  {
    int start = clock();
    puts("hello world");
    printf("hello %d\n", start);
    int STD_OUTPUT_HANDLE = 0-11;
    int h = GetStdHandle(STD_OUTPUT_HANDLE);
    printf("h=%d\n", h);
    WriteConsoleA(h, "hello console\n", 14, 0, 0);
    puts("=================");
    //runFile("..\\test\\hello_mod.c");
    return 0;
}
