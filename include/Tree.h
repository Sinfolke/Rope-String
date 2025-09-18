#ifndef ISPA_TREE_H
#define ISPA_TREE_H
#include <Node.h>
#include <iostream>
#include <vector>
#include <numeric>

#ifndef ROPE_STRING_MAX_ROOT_SIZE
#define ROPE_STRING_MAX_ROOT_SIZE 512
#endif
namespace Rope {
    constexpr std::size_t max_root_size = ROPE_STRING_MAX_ROOT_SIZE;
    template<typename CharT, typename Traits = std::char_traits<CharT>, typename Allocator = std::allocator<CharT>>
    class Tree {
        using NodeType = Node<CharT, Traits, Allocator>;
        using StringType = std::basic_string<CharT, Traits, Allocator>;
        std::vector<std::shared_ptr<NodeType>> roots;
        std::vector<std::size_t> sizes;
        Allocator allocator;

        void insertAfter(NodeType* leaf, std::shared_ptr<NodeType> new_leaf, std::size_t root_index) {
            if (sizes[root_index] + new_leaf->str.size() >= max_root_size) {
                leaf->ending_node = true;
                roots.push_back(new_leaf);
                sizes.push_back(new_leaf->str.size());
                return;
            }
            new_leaf->right = leaf->right;
            leaf->right = new_leaf;

            // Update size of the current root
            sizes[root_index] = roots[root_index]->size();
        }

    public:
        using Node = NodeType;

        Tree() {
            roots.push_back(std::make_shared<NodeType>("", allocator));
            sizes.push_back(0);
        }
        Tree(Allocator allocator) : allocator(allocator) {
            roots.push_back(std::make_shared<NodeType>("", allocator));
            sizes.push_back(0);
        }

        void push(const StringType &str) {
            std::size_t index = 0;

            while (index < str.size()) {
                // If no roots exist or last root is full, create a new root
                if (roots.empty() || sizes.back() >= max_root_size) {
                    roots.push_back(std::make_shared<NodeType>(StringType(), allocator));
                    sizes.push_back(0);
                }

                auto &current_root = roots.back();
                std::size_t space_in_root = max_root_size - sizes.back();
                std::size_t chunk_size = std::min(space_in_root, str.size() - index);

                // Split chunk into leaves
                std::vector<std::shared_ptr<NodeType>> leaves;
                for (std::size_t i = 0; i < chunk_size; i += max_leaf_size) {
                    size_t leaf_size = std::min(max_leaf_size, chunk_size - i);
                    leaves.push_back(std::make_shared<NodeType>(StringType(str, index + i, leaf_size), allocator));
                }

                // If no leaves (shouldn’t happen), skip
                if (leaves.empty()) break;
                // If current_root.str is empty, move first leaf’s str into it
                if (current_root->str.empty()) {
                    current_root->str = std::move(leaves.front()->str);
                    leaves.front()->str.clear();
                }

                // Attach subtree to the rightmost leaf of current_root
                NodeType* right_most = current_root.get();
                while (right_most->right) right_most = right_most->right.get();

                // Attach all remaining leaves except the first (already moved)
                for (std::size_t i = 1; i < leaves.size(); ++i) {
                    right_most->right = leaves[i];
                    leaves[i]->top = right_most;
                    right_most = leaves[i].get();
                }

                // Update weights along the path
                NodeType* current = right_most;
                while (current) {
                    current->weight = (current->left ? current->left->size() : 0) + current->str.size();
                    if (current->right && !current->ending_node) current->weight += current->right->size();
                    current = current->top;
                }

                // Update root size
                sizes.back() = current_root->size();

                index += chunk_size;
            }
        }

        void insert(std::size_t index, const std::basic_string<CharT, Traits, Allocator> &str) {
            if (index >= size()) {
                push(str);
                return;
            }

            std::size_t offset = 0;
            auto* leaf = getLeafByIndex(index, offset);
            auto root_index = getRootByIndex(index);

            // Local position in leaf
            leaf->str.insert(leaf->str.begin() + offset, str.begin(), str.end());

            while (leaf->str.size() > max_leaf_size) {
                std::string right_part(leaf->str.begin() + max_leaf_size, leaf->str.end());
                leaf->str.erase(leaf->str.begin() + max_leaf_size, leaf->str.end());
                NodeType node;
                node.str = StringType(std::move(right_part), allocator);
                node.top = leaf;
                auto new_leaf = std::make_shared<NodeType>(node);
                insertAfter(leaf, new_leaf, root_index);

                leaf->weight = leaf->str.size() + (leaf->left ? leaf->left->size() : 0);
                new_leaf->weight = new_leaf->str.size();

                leaf = new_leaf.get();
            }

            sizes[root_index] = roots[root_index]->size();
        }

        auto getRootByIndex(std::size_t index) const -> std::size_t {
            std::size_t offset = 0;
            for (std::size_t i = 0; i < roots.size(); ++i) {
                if (index < offset + sizes[i]) return i;
                offset += sizes[i];
            }
            return roots.size() - 1;
        }

        auto getLeafByIndex(std::size_t index, std::size_t &offset) const -> NodeType* {
            for (std::size_t i = 0; i < roots.size(); ++i) {
                if (index < offset + sizes[i]) {
                    offset = index - offset;
                    return roots[i]->getLeafByIndex(offset);
                }
                offset += sizes[i];
            }
            throw std::out_of_range("Rope::Tree::getLeadByIndex");
        }
        Node* nextLeaf(Node* node) const {
            if (!node) return nullptr;

            Node* cur = node;

            // 1. If right subtree exists, go to its leftmost leaf
            if (cur->right) {
                return cur->right->leftmostLeaf();
            }

            // 2. Otherwise climb up
            Node* p = cur->top;
            while (p && (p->right.get() == cur || !p->right)) {
                cur = p;
                p = p->top;
            }

            if (p && p->right) {
                return p->right->leftmostLeaf();
            }

            // 3. If no parent → move to next root
            for (std::size_t i = 0; i + 1 < roots.size(); ++i) {
                if (roots[i].get() == cur) { // <--- must check original node
                    return roots[i+1]->leftmostLeaf();
                }
            }

            return nullptr; // end
        }
        Node* prevLeaf(Node* node) const {
            if (!node) return nullptr;

            if (node->left) {
                return node->left->rightmostLeaf();
            } else {
                if (node->top && node->top->right.get() == node) {
                    return node->top;
                }
                // it is top's left node
                while (node->top)
                    node = node->top;
                for (std::size_t i = 1; i < roots.size(); ++i) {
                    if (roots[i].get() == node) {
                        return roots[i-1]->rightmostLeaf();
                    }
                }
            }
            return nullptr;
        }
        void printTree(const Node* node, int depth = 0) const {
            if (!node) return;
            std::string indent(depth * 2, ' ');
            std::cout << indent << "- Node(";
            if (!node->str.empty()) {
                std::cout << "\"" << node->str << "\"";
            } else {
                std::cout << "internal";
            }
            std::cout << "), size=" << node->size() << "\n";

            if (node->left) {
                std::cout << indent << "  L:\n";
                printTree(node->left.get(), depth + 2);
            }
            if (node->right) {
                std::cout << indent << "  R:\n";
                printTree(node->right.get(), depth + 2);
            }
        }
        void printForest() const {
            for (std::size_t i = 0; i < roots.size(); ++i) {
                std::cout << "Root[" << i << "], size=" << sizes[i] << ":\n";
                printTree(roots[i].get(), 1);
            }
        }
        auto size() const -> std::size_t {
            return std::accumulate(sizes.begin(), sizes.end(), static_cast<std::size_t>(0));
        }

        void clear() {
            roots = { std::make_shared<NodeType>("", allocator) };
            sizes = {0};
        }
        auto operator==(const Tree &other) const -> bool {
            return size() == other.size() && roots == other.roots;
        }
        auto get_allocator() -> Allocator { return allocator; }
        auto &getRoots() { return roots; }
        auto &getSizes() { return sizes; }
        auto &getRoots() const { return roots; }
        auto &getSizes() const { return sizes; }
    };
}

#endif //ISPA_TREE_H