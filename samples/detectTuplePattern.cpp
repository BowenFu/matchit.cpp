#include <array>
#include "matchit/core.h"
#include "matchit/patterns.h"
using namespace matchit;

template <typename Tuple>
int32_t detectTuplePattern(Tuple const& tuple)
{
    return match(tuple)
    (
        pattern(ds(ooo(3)))             = []{return 1;}, // all 3
        pattern(ds(_, ooo(3)))          = []{return 2;}, // all 3 except the first one
        pattern(ds(ooo(3), _))          = []{return 3;}, // all 3 except the last one
        pattern(ds(_, ooo(3), _))       = []{return 4;}, // all 3 except the first and the last one
        pattern(ds(3, ooo(not_(3)), 3)) = []{return 5;}, // all non 3 except the first and the last one
        pattern(ds(3, ooo(_), 3))       = []{return 6;}, // first and last being 3, mxied by 3 and non-3 in the middle.
        pattern(_)                      = []{return 0;}  // mismatch
    );
}

int main()
{
    printf("%d\n", detectTuplePattern(std::make_tuple(3, 3, 3, 3, 3)));
    printf("%d\n", detectTuplePattern(std::make_tuple(2, 3, 3, 3, 3)));
    printf("%d\n", detectTuplePattern(std::make_tuple(3, 3, 3, 2)));
    printf("%d\n", detectTuplePattern(std::make_tuple(2, 3, 3, 3, 2)));
    printf("%d\n", detectTuplePattern(std::make_tuple(3, 4, 2, 4, 3)));
    printf("%d\n", detectTuplePattern(std::make_tuple(3, 3, 2, 3, 3)));
    printf("%d\n", detectTuplePattern(std::make_tuple(2, 3, 2, 3, 3)));
    return 0;
}
