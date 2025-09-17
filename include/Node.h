#ifndef ISPA_NODE_H
#define ISPA_NODE_H

#include <vector>
#include <string>
#include <memory>
#ifndef ROPE_STRING_MAX_LEAF_SIZE
#define ROPE_STRING_MAX_LEAF_SIZE 128
#endif
namespace Rope {
    constexpr std::size_t max_leaf_size = ROPE_STRING_MAX_LEAF_SIZE;
    template<typename CharT, typename Traits = std::char_traits<CharT>, typename Allocator = std::allocator<CharT>>
    struct Node {
        using StringType = std::basic_string<CharT, Traits, Allocator>;
        std::basic_string<CharT, Traits, Allocator> str;
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;
        std::size_t weight = 0;
        Node *top = nullptr;
        Node() = default;
        Node(const std::basic_string<CharT, Traits, Allocator> &str, Allocator allocator) : str(str, allocator) {}

        auto size() const -> std::size_t {
            std::size_t size = str.size();
            if (left) size += left->size();
            if (right) size += right->size();
            return size;
        }
        auto getLeafByIndex(std::size_t &index) const -> Node* {
            // if leaf node
            if (!left && !right) {
                return const_cast<Node*>(this);
            }

            if (left && index < left->weight) {
                return left->getLeafByIndex(index);
            } else if (right) {
                if (left)
                    index -= left->weight;
                return right->getLeafByIndex(index);
            }

            return nullptr; // index out of range
        }

        auto operator==(const Node &other) {
            return weight == other.weight && str == other.str && (left == other.left || left.get() == other.left.get()) && (right == other.right || right.get() == other.right.get());
        }
        void operator+=(const std::string &append_str) {
            if (str.size() + append_str.size() > 128) {
                if (right)
                    *right.get() += append_str;
                else {
                    Node node;
                    node.str = StringType(append_str, str.get_allocator());
                    node.top = this;
                    right = std::make_shared<Node>(node);
                }
            } else {
                str += append_str;
            }
            weight = str.size() + (left ? left->size() : 0);
            if (right) weight += right->size();
        }
    };
}
#endif //ISPA_NODE_H