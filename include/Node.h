#ifndef ROPE_NODE_H
#define ROPE_NODE_H

#include <vector>
#include <string>
#include <memory>
#ifndef ROPE_STRING_MAX_LEAF_SIZE
#define ROPE_STRING_MAX_LEAF_SIZE 128
#endif
namespace Rope {
    constexpr std::size_t max_leaf_size = ROPE_STRING_MAX_LEAF_SIZE;
    template<typename CharT, typename Traits = std::char_traits<CharT>, typename Allocator = std::allocator<CharT>>
    struct Node : std::enable_shared_from_this<Node<CharT, Traits, Allocator>> {
        using StringType = std::basic_string<CharT, Traits, Allocator>;
        std::basic_string<CharT, Traits, Allocator> str;
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;
        std::size_t weight = 0;
        Node *top = nullptr;
        bool ending_node = false; // when a next node is new root
        Node() = default;
        Node(const std::basic_string<CharT, Traits, Allocator> &str, Allocator allocator) : str(str, allocator) {}

        auto size() const -> std::size_t {
            std::size_t size = str.size();
            if (left) size += left->size();
            if (right && !ending_node) size += right->size();
            return size;
        }
        auto getLeafByIndex(std::size_t &index) -> Node* {
            if (left && index < left->size())
                return left->getLeafByIndex(index);

            if (left) index -= left->size();

            if (index < str.size())
                return this;

            if (right && !ending_node) {
                index -= str.size();
                return right->getLeafByIndex(index);
            }

            throw std::out_of_range("index out of range");
        }
        Node* leftmostLeaf() const {
            Node* n = const_cast<Node*>(this);
            while (n->left) n = n->left.get();
            return n;
        }
        Node* rightmostLeaf() const {
            Node* n = const_cast<Node*>(this);
            while (n->right) n = n->right.get();
            return n;
        }

        auto operator==(const Node &other) {
            return  weight == other.weight &&
                    str == other.str &&
                    (left == other.left || left.get() == other.left.get()) &&
                    (ending_node == other.ending_node || right == other.right || right.get() == other.right.get());
        }
    };
}
#endif //ROPE_NODE_H