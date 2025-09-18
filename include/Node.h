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
        bool ending_node = false; // when a next node is new root
        Node() = default;
        Node(const std::basic_string<CharT, Traits, Allocator> &str, Allocator allocator) : str(str, allocator) {}

        auto size() const -> std::size_t {
            std::size_t size = str.size();
            if (left) size += left->size();
            if (right && !ending_node) size += right->size();
            return size;
        }
        auto contains(Node *node) -> bool {
            return left.get() == node || right.get() == node || (left && left->contains(node)) || (right && right->contains(node));
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
        void operator+=(const std::string &append_str) {
            if (str.size() + append_str.size() > 128) {
                if (ending_node)
                    throw std::runtime_error("Right is root what should be handled by tree, not the Node");
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
            if (right && !ending_node) weight += right->size();
        }
        static std::shared_ptr<Node> buildTree(const std::vector<std::shared_ptr<Node>> &leaves, size_t start, size_t end) {
            if (start >= end) return nullptr;
            if (start + 1 == end) return leaves[start]; // leaf node

            size_t mid = start + (end - start) / 2;
            auto node = leaves[mid];
            node->left = buildTree(leaves, start, mid);
            node->right = buildTree(leaves, mid + 1, end);

            // Update weight = size of left + own string
            node->weight = (node->left ? node->left->size() : 0) + node->str.size();
            return node;
        }
    };
}
#endif //ISPA_NODE_H