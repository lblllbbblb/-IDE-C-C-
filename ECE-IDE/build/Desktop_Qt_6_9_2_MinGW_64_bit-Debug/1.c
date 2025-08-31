#include <iostream>
using namespace std;

int main() {
    const int count = 10;
    long long a = 0, b = 1, next;
    
    cout << "前" << count << "个斐波那契数: ";
    cout << a << " " << b << " ";
    
    for (int i = 3; i <= count; ++i) {
        next = a + b;
        a = b;
        b = next;
        cout << next << " ";
    }
    cout << endl;
    
    return 0;
}
    