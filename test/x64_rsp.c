int main()  {
//    int s = 1;
//    int t = 15 - 3 * (2 * 2 + (7 - 2));
//    int d = add1(1);
//    int a = add2(1,2);
    int b = add3(1, 2, 3);
//    int c = add4(1, 2, 3, 4);
    return 12;
}
//int add1(int a) {
//    func2(a);
//    return 0;
//}
//int add2(int a, int b) {
//    int s = a + b;
//    func2(a);
//    func2(b);
//    func2(s);
////    printf("%d + %d = %d", a, b, s);
//    return s;
//}
int add3(int a, int b, int c) {
    int s = a + b + c;
    func2(a,8,9,7);
    func2(b);
    func2(c);
    func2(s);
//    printf("%d + %d +%d = %d", a, b, s);
    
    return s;
}
//int add4(int a, int b, int c,int d) {
//    func2(a);
//    func2(b);
//    func2(c);
//    func2(d);
//    int s = a + b + c + d;
//    func2(s);
////    printf("%d + %d +%d = %d", a, b, s);
//    
//    return a;
//}
