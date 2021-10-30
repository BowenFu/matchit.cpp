#include "matchit.h"
#include <iostream>
#include <memory>
#include <vector>
#include <string>

template <typename T>
struct Node
{
  T value;
  std::vector<Node *> parents;
};

/*
  A
  |
  B
  | \
  |  C
  | /
  D
*/

bool matchGraph()
{
  using StrNode = Node<std::string>;
  auto A = std::make_unique<StrNode>(StrNode{"A", {}});
  auto B = std::make_unique<StrNode>(StrNode{"B", {A.get()}});
  auto C = std::make_unique<StrNode>(StrNode{"C", {B.get()}});
  auto D = std::make_unique<StrNode>(StrNode{"D", {B.get(), C.get()}});

  using namespace matchit;
  // FIXME, moving dsN into someDsN will cause segfault.
  constexpr auto dsN = dsVia(&StrNode::value, &StrNode::parents);
  constexpr auto someDsN = [dsN](auto... pats)
  {
    return some(dsN(pats...));
  };
  Id<StrNode *> b;
  return match(D)(
      pattern | someDsN("D",
                        ds(
                            b.at(
                                someDsN("B",
                                         ds(someDsN("A", ds())))),
                            someDsN("C", ds(b)))) = []
      {
        std::cout << "Matched!" << std::endl;
        return true;
      });
}

int main()
{
  assert(matchGraph());
  return 0;
}