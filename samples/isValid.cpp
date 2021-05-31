#include "include/core.h"
#include "include/patterns.h"
using namespace matchit;

bool isValid(int32_t n)
{
    return match(n)(
        pattern(or_(1, 3, 5)) = []{ return true; },
        pattern(_)            = []{ return false; }
    );
}

int main()
{
    printf("%d\n", isValid(3));
    return 0;
}
