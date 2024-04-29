int main()  {
    int s = 1;
    int t = 15 - 3 * (2 * 2 + (7 - 2));
    int a = add2(9,2);
    printf("9+2=%d,s=%d,t=%d\n", a,s,t);
    int b = add3(9, 2, 1);
    printf("9+2+1=%d,s=%d,t=%d\n", b,s,t);
    return 12;
}
int add2(int a, int b) {
    return a + b;
}
int add3(int a, int b, int c) {
    return a + b + c;
}
