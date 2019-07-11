
#include <string>

namespace util {
class AVL_tree {
  using key_type = int;
  using value_type = std::string;
  using pair_type = std::pair<key_type, value_type>;

public:
  value_type find(key_type k);
  void insert(pair_type kvp);
  pair_type delete (key_type k);
  pair_type minimum();
  pair_type maximum();
  pair_type succ(key_type k);
  pair_type pred(key_type k);

}; // class AVL_tree
}; // namespace util
