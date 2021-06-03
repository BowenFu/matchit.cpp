#include <iostream>
#include "include/core.h"
#include "include/patterns.h"
#include "include/utility.h"
using namespace matchit;

enum class Kind { kONE, kTWO };

class Num
{
public:
    virtual ~Num() = default;
    virtual Kind kind() const = 0;
};

class One : public Num
{
public:
    constexpr static auto k = Kind::kONE;
    Kind kind() const override
    {
        return k;
    }
};

class Two : public Num
{
public:
    constexpr static auto k = Kind::kTWO;
    Kind kind() const override
    {
        return k;
    }
};

template <Kind k>
auto constexpr kind = app(&Num::kind, k);

template <typename T>
class NumAsPointer
{
public:
    auto operator()(Num const& num) const
    {
        std::cout << "custom as pointer." << std::endl;
        return num.kind() == T::k ? static_cast<T const *>(std::addressof(num)) : nullptr;
    }
};

template <>
class matchit::impl::CustomAsPointer<One> : public NumAsPointer<One> {};

template <>
class matchit::impl::CustomAsPointer<Two> : public NumAsPointer<Two> {};

int staticCastAs(Num const& input)
{
    return match(input)(
        pattern(as<One>(_)) = [] { return 1; },
        pattern(kind<Kind::kTWO>) = [] { return 2; },
        pattern(_) = [] { return 3; });
}

int main()
{
    std::cout << staticCastAs(One{}) << std::endl;
    return 0;
}
