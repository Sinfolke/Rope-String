#ifndef ISPA_TREE_H
#define ISPA_TREE_H
#include <Node.h>
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
        std::vector<std::shared_ptr<NodeType>> roots;
        std::vector<std::size_t> sizes;
        Allocator allocator;

        void insertAfter(NodeType* leaf, std::shared_ptr<NodeType> new_leaf, std::size_t root_index) {
            new_leaf->right = leaf->right;
            leaf->right = new_leaf;

            // Update size of the current root
            sizes[root_index] = roots[root_index]->size();

            // Split root if it exceeds max_root_size
            if (sizes[root_index] > max_root_size) {
                roots.insert(roots.begin() + root_index + 1, new_leaf);
                sizes.insert(sizes.begin() + root_index + 1, new_leaf->size());
            }
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

        void push(const std::basic_string<CharT, Traits, Allocator> &str) {
            *roots.back() += str;
            sizes.back() = roots.back()->size();
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
            std::size_t local_pos = index - offset;
            leaf->str.insert(leaf->str.begin() + local_pos, str.begin(), str.end());

            while (leaf->str.size() > max_leaf_size) {
                std::string right_part(leaf->str.begin() + max_leaf_size, leaf->str.end());
                leaf->str.erase(leaf->str.begin() + max_leaf_size, leaf->str.end());

                auto new_leaf = std::make_shared<NodeType>(NodeType {.str = StringType(right_part, allocator), .top = leaf});
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

        auto getLeafByIndex(std::size_t &index, std::size_t &offset) const -> NodeType* {
            for (std::size_t i = 0; i < roots.size(); ++i) {
                if (index < offset + sizes[i])
                    return roots[i]->getLeafByIndex(index -= offset);
                offset += sizes[i];
            }
            throw std::out_of_range("index out of range");
        }

        auto size() const {
            return std::accumulate(sizes.begin(), sizes.end(), 0);
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