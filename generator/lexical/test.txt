// 测试C99词法分析器
#include <stdio.h>

int calculate_sum(int a, int b) {
    return a + b;
}

int main() {
    // 变量声明
    int num1 = 10;
    int num2 = 20;
    float pi = 3.14159f;
    char ch = 'A';
    
    // 关键字测试
    if (num1 > 0) {
        num1 = num1 + 5;
    }
    
    while (num2 < 100) {
        num2++;
    }
    
    for (int i = 0; i < 10; i++) {
        num1 += i;
    }
    
    // 函数调用
    int result = calculate_sum(num1, num2);
    
    /* 多行
       注释测试 */
    
    return 0;
}
