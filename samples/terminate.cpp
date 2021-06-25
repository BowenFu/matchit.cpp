#include "matchit.h"
#include <iostream>

enum class Op { Add, Sub, Mul, Div };

Op parseOp(char t) {
    Id<char> token;
    return match(t) ( 
        pattern | '+' = expr(Op::Add),
        pattern | '-' = expr(Op::Sub),
        pattern | '*' = expr(Op::Mul),
        pattern | '/' = expr(Op::Div),
        pattern | token = [&]{
            std::cerr << "Unexpected: " << *token;
            std::terminate();
        }
    );
}


int32_t main()
{
    std::cout << static_cast<int>(parseOp('*')) << std::endl;
    return 0;
}
