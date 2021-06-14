#include <iostream>
#include "matchit/core.h"
#include "matchit/patterns.h"
#include "matchit/utility.h"
using namespace matchit;

enum class Kind { kONE, kTWO };

class Num
{
public:
    virtual Kind kind() const = 0;
protected:
    ~Num() = default;
};

class One : public Num
{
public:
    constexpr static auto k = Kind::kONE;
    constexpr Kind kind() const override
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
    constexpr auto operator()(Num const& num) const
    {
        // std::cout << "custom as pointer." << std::endl;
        return num.kind() == T::k ? static_cast<T const *>(std::addressof(num)) : nullptr;
    }
};

template <>
class matchit::impl::CustomAsPointer<One> : public NumAsPointer<One> {};

template <>
class matchit::impl::CustomAsPointer<Two> : public NumAsPointer<Two> {};

constexpr int staticCastAs(Num const& input)
{
    return match(input)(
        pattern(as<One>(_)) = [] { return 1; },
        pattern(kind<Kind::kTWO>) = [] { return 2; },
        pattern(_) = [] { return 3; });
}

static_assert(staticCastAs(One{}) == 1);

int main()
{
    std::cout << staticCastAs(One{}) << std::endl;
    return 0;
}
