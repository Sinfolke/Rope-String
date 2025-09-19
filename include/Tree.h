#ifndef ROPE_TREE_H
#define ROPE_TREE_H
#include <Node.h>
#include <algorithm>
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
        std::vector<std::pair<std::shared_ptr<NodeType>, std::size_t>> roots;
        Allocator allocator;

        void insertAfter(NodeType* leaf, std::shared_ptr<NodeType> new_leaf, std::size_t root_index) {
            if (roots[root_index].second + new_leaf->str.size() >= max_root_size) {
                leaf->ending_node = true;
                roots.emplace_back(new_leaf, new_leaf->str.size());
                return;
            }
            // Link the new leaf into the right chain of `leaf` (sibling chain)
            new_leaf->right = leaf->right;
            new_leaf->top = leaf->top; // keep same parent context if used
            leaf->right = new_leaf;

            // Update size of the current root incrementally
            roots[root_index].second += new_leaf->str.size();
        }
        void shiftLeaf(NodeType* leaf, std::size_t n) {
            if (!leaf || n == 0) return;

            auto old_right = leaf->right;

            // We need a shared_ptr reference to "leaf" to continue chaining
            std::shared_ptr<NodeType> prev = leaf->shared_from_this();

            for (std::size_t i = 0; i < n; ++i) {
                auto new_leaf = std::make_shared<NodeType>();
                new_leaf->str.clear();

                // Insert to the right of `prev` in sibling chain
                prev->right = new_leaf;
                new_leaf->right = nullptr;
                new_leaf->top = leaf->top;

                prev = new_leaf; // move forward
            }

            // Connect the tail back to the old right chain
            prev->right = old_right;
        }

    public:
        using Node = NodeType;

        Tree() {
            roots.emplace_back(std::make_shared<NodeType>("", allocator), 0);
        }
        Tree(Allocator allocator) : allocator(allocator) {
            roots.emplace_back(std::make_shared<NodeType>("", allocator), 0);
        }

        void push(const StringType &str) {
            std::size_t index = 0;

            while (index < str.size()) {
                // If no roots exist or last root is full, create a new root
                if (roots.empty() || roots.back().second >= max_root_size) {
                    roots.emplace_back(std::make_shared<NodeType>(StringType(), allocator), 0);
                }

                auto &current_root = roots.back();
                std::size_t space_in_root = max_root_size - roots.back().second;
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
                if (current_root.first->str.empty()) {
                    current_root.first->str = std::move(leaves.front()->str);
                    leaves.front()->str.clear();
                }

                // Attach subtree to the rightmost leaf of current_root
                NodeType* right_most = current_root.first.get();
                while (right_most->right) right_most = right_most->right.get();

                // Attach all remaining leaves except the first (already moved)
                for (std::size_t i = 1; i < leaves.size(); ++i) {
                    right_most->right = leaves[i];
                    leaves[i]->top = right_most;
                    right_most = leaves[i].get();
                }

                // Update weights along the path (best-effort, not critical for current usage)
                NodeType* current = right_most;
                while (current) {
                    current->weight = (current->left ? current->left->size() : 0) + current->str.size();
                    if (current->right && !current->ending_node) current->weight += current->right->size();
                    current = current->top;
                }

                // Update root size incrementally by the amount appended into this root
                current_root.second += chunk_size;

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

            // Split the leaf at insertion point to preserve order
            StringType tail;
            if (offset < leaf->str.size()) {
                tail.assign(leaf->str.begin() + offset, leaf->str.end());
                leaf->str.erase(leaf->str.begin() + offset, leaf->str.end());
            }

            // Fill current leaf up to max_leaf_size
            std::size_t begin_in_str = 0;
            std::size_t can_take_here = std::min<std::size_t>(max_leaf_size - leaf->str.size(), str.size());
            if (can_take_here > 0) {
                leaf->str.insert(leaf->str.end(), str.begin(), str.begin() + can_take_here);
                begin_in_str = can_take_here;
            }
            if (begin_in_str == str.size() && tail.empty()) {
                // everything fits into current leaf; root size grows by the inserted amount
                roots[root_index].second += str.size();
                return;
            }

            // How many extra leaves needed for remaining insertion (+1 for tail if exists)?
            std::size_t remaining = str.size() - begin_in_str;
            std::size_t chunks = (remaining + max_leaf_size - 1) / max_leaf_size;
            std::size_t extra = chunks + (tail.empty() ? 0 : 1);
            shiftLeaf(leaf, extra);

            // Fill new leaves with remaining insertion
            auto current = leaf->right;
            for (std::size_t i = begin_in_str; i < str.size(); i += max_leaf_size) {
                auto end = std::min(i + max_leaf_size, str.size());
                current->str.assign(str.begin() + i, str.begin() + end);
                current = current->right;
            }

            // Place the old tail after the inserted content
            if (!tail.empty()) {
                current->str = std::move(tail);
                // advance to next (old right)
                current = current->right;
            }

            // Finally, root size increases by the total inserted length
            roots[root_index].second += str.size();

        }

        auto getRootByIndex(std::size_t index) const -> std::size_t {
            std::size_t offset = 0;
            for (std::size_t i = 0; i < roots.size(); ++i) {
                if (index < offset + roots[i].second) return i;
                offset += roots[i].second;
            }
            return roots.size() - 1;
        }

        auto getLeafByIndex(std::size_t index, std::size_t &offset) const -> NodeType* {
            for (std::size_t i = 0; i < roots.size(); ++i) {
                if (index < offset + roots[i].second) {
                    offset = index - offset;
                    return roots[i].first->getLeafByIndex(offset);
                }
                offset += roots[i].second;
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
                if (roots[i].first.get() == cur) { // <--- must check original node
                    return roots[i+1].first->leftmostLeaf();
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
                    if (roots[i].first.get() == node) {
                        return roots[i-1].first->rightmostLeaf();
                    }
                }
            }
            return nullptr;
        }
        auto size() const -> std::size_t {
            return std::accumulate(roots.begin(),  roots.end(), static_cast<std::size_t>(0), [](std::size_t acc, const auto &value) {
                return acc + value.second;
            });
        }

        void clear() {
            roots = { std::make_pair<std::shared_ptr<NodeType>, std::size_t>({ std::make_shared<NodeType>("", allocator) }, 0) };
        }
        auto operator==(const Tree &other) const -> bool {
            return size() == other.size() && roots == other.roots;
        }
        auto get_allocator() -> Allocator { return allocator; }
        auto &getRoots() { return roots; }
        auto &getRoots() const { return roots; }
    };
}

#endif //ROPE_TREE_H