int main() {
    int a = add(9, 2);
    printf("9+2 = %d\n", a);
    echoInt(10);
    return 123;
}
int add(int a, int b) {
    return a + b;
}
void echoInt(int a) {
    printf("a = %d\n", a);
}
