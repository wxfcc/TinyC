int main()  {
    int a = add2(1,2);
    int b = add3(3, 4, 5);
    return 1;
    func1(a);
    func1(b);
    return 12;
}
int add2(int a, int b) {
    return a + b;
}
int add3(int a, int b, int c) {
    return a + b + c;
}
