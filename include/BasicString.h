#ifndef ISPA_BASICSTRING_H
#define ISPA_BASICSTRING_H
#include <Node.h>
#include <Tree.h>
#include <vector>
#include <string>
#include <functional>
#include <ranges>
#include <cstring>

namespace {
    template<typename CharT, typename Tree>
    class iterator {
    public:
        using value_type        = CharT;
        using difference_type   = std::ptrdiff_t;
        using pointer           = CharT*;
        using reference         = CharT&;
        using iterator_category = std::forward_iterator_tag;

        explicit iterator(Tree &tree, std::size_t pos = 0) : tree_(tree) {
            if (pos >= tree.size()) {
                current = nullptr;
                this->pos = pos;
                return;
            }
            std::size_t offset = 0;
            std::size_t new_pos = pos;
            current = tree.getLeafByIndex(new_pos, offset);
            this->pos = new_pos;
        }

        auto operator*() const -> CharT {
            return current->str[pos];
        }

        auto operator++() -> iterator& {
            if (pos >= current->str.size()) {
                current = current->right ? current->right.get() : current->top;
                pos = 0;
            } else pos++;
            return *this;
        }

        auto operator++(int) -> iterator {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        auto operator==(const iterator& other) const -> bool {
            return pos == other.pos;
        }

        auto operator!=(const iterator& other) const -> bool {
            return !(*this == other);
        }

        auto position() const -> std::size_t { return pos; }

    protected:
        Tree &tree_;
        std::size_t pos;
        Tree::Node *current = nullptr;
    };
    template<typename CharT, typename Tree>
    class reverse_iterator : public iterator<CharT, Tree> {
        using Base = iterator<CharT, Tree>;
    public:
        explicit reverse_iterator(Tree &tree, std::size_t pos = 0) : Base(tree, tree.size() - 1 - pos) {}
        auto operator++() -> reverse_iterator& {
            if (Base::pos == Base::current->str.size()) {
                Base::current = Base::current->left ? Base::current->left.get() : Base::current->top;
                Base::pos = Base::current->str.size();
            } else Base::pos--;
            return *this;
        }

        auto operator++(int) -> reverse_iterator {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
    };
}
namespace Rope {
    template<typename CharT, typename Traits = std::char_traits<CharT>, typename Allocator = std::allocator<CharT>>
    class BasicString {
        using StringType = std::basic_string<CharT, Traits, Allocator>;
        using NodeType = Node<CharT, Traits, Allocator>;
        using TreeType = Tree<CharT, Traits, Allocator>;
    public:
        using traits_type = Traits;
        using value_type = CharT;
        using allocator_type = Allocator;
        using size_type = std::allocator_traits<Allocator>::size_type;
        using difference_type = std::allocator_traits<Allocator>::difference_type;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = std::allocator_traits<Allocator>::pointer;
        using const_pointer = const std::allocator_traits<Allocator>::pointer;
        using iterator = ::iterator<CharT, Tree<CharT, Traits, Allocator>>;
        using const_iterator = ::iterator<const CharT, const Tree<CharT, Traits, Allocator>>;
        using reverse_iterator = ::reverse_iterator<CharT, Tree<CharT, Traits, Allocator>>;
        using reverse_const_iterator = ::reverse_iterator<const CharT, const Tree<CharT, Traits, Allocator>>;
        static constexpr auto npos = StringType::npos;
        BasicString() {}
        BasicString(const Allocator &alloc) : tree(alloc) {}
        BasicString(size_type count, CharT ch, const Allocator& alloc = Allocator()) {
            tree.push(StringType(count, ch, alloc));
        }
        template<typename InputIt>
        BasicString(const InputIt first, InputIt last, Allocator alloc = Allocator()) {
            tree.push(StringType(first, last, alloc));
        }
#ifdef __cpp_lib_from_range
        template<std::ranges::range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, CharT>
        BasicString(std::from_range_t, R&& rg, const Allocator& alloc = Allocator()) : tree(alloc) {
            for (auto&& c : rg) {
                tree.push(StringType(1, c, alloc));
            }
        }
#endif
        BasicString( const CharT* s, size_type count, const Allocator& alloc = Allocator() ) {
            tree.push(StringType(s, count, alloc));
        }
        BasicString( const CharT* s, const Allocator& alloc = Allocator() ) {
            tree.push(StringType(s, alloc));
        }
        BasicString(std::nullptr_t) = delete;
        template<typename StringViewLike>
        explicit BasicString( const StringViewLike& t, const Allocator& alloc = Allocator() ) {
            tree.push(StringType(t, alloc));
        }
        template<typename StringViewLike>
        BasicString(const StringViewLike& t, size_type pos, size_type count, const Allocator& alloc = Allocator() ) {
            tree.push(StringType(t, pos, count, alloc));
        }

        BasicString( const BasicString& other ) {
            tree = other.tree;
        }
        BasicString(BasicString &&other) noexcept {
            tree = std::move(other.tree);
        }
        BasicString( const BasicString& other, const Allocator &alloc) : tree(alloc){
            tree = other.tree;
        }
        BasicString(BasicString &&other, const Allocator &alloc) : tree(alloc) {
            tree = std::move(other.tree);
        }
        BasicString( const BasicString& other, size_type pos, const Allocator& alloc = Allocator() ) : tree(alloc) {
            tree.push(StringType(other, pos, alloc));
        };
        BasicString( const BasicString& other, size_type pos, size_type count, const Allocator& alloc = Allocator() ) : tree(alloc) {
            tree.push(StringType(other, pos, count, alloc));
        };
        BasicString(std::initializer_list<CharT> ilist, const Allocator& alloc = Allocator() ) : tree(alloc) {
            tree.push(StringType(ilist, alloc));
        }

        // (1) assign from const basic_string&
        auto assign(const BasicString& str) -> BasicString& {
            tree.clear();
            for (auto& root : str.tree.roots) {
                tree.push(root->str); // copy each root string
            }
            return *this;
        }

        // (2) assign from rvalue basic_string
        auto assign(BasicString&& str) noexcept -> BasicString& {
            tree = std::move(str.tree);
            return *this;
        }

        // (3) assign count copies of a char
        auto assign(size_type count, CharT ch) -> BasicString& {
            tree.clear();
            tree.push(StringType(count, ch));
            return *this;
        }

        // (4) assign from const CharT* with count
        auto assign(const CharT* s, size_type count) -> BasicString& {
            tree.clear();
            tree.push(StringType(s, count));
            return *this;
        }

        // (5) assign from const CharT* null-terminated
        auto assign(const CharT* s) -> BasicString& {
            tree.clear();
            tree.push(StringType(s));
            return *this;
        }

        // (6) assign from StringViewLike
        template<typename SV>
        auto assign(const SV& t) -> BasicString& {
            tree.clear();
            tree.push(StringType(t));
            return *this;
        }

        // (7) assign from StringViewLike with pos/count
        template<typename SV>
        auto assign(const SV& t, size_type pos, size_type count = StringType::npos) -> BasicString& {
            tree.clear();
            tree.push(StringType(t, pos, count));
            return *this;
        }

        // (8) assign from basic_string with pos/count
        auto assign(const BasicString& str, size_type pos, size_type count = StringType::npos) -> BasicString& {
            tree.clear();
            StringType tmp = str.tree.substr(pos, count);
            tree.push(tmp);
            return *this;
        }

        // (9) assign from input iterators
        template<typename InputIt>
        auto assign(InputIt first, InputIt last) -> BasicString& {
            tree.clear();
            tree.push(StringType(first, last));
            return *this;
        }

        // (10) assign from initializer_list
        auto assign(std::initializer_list<CharT> ilist) -> BasicString& {
            tree.clear();
            tree.push(StringType(ilist.begin(), ilist.end()));
            return *this;
        }
#ifdef __cpp_lib_from_range
        template<std::ranges::range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, CharT>
        auto assign_range(R&& rg) -> BasicString& {
            for (auto&& c : rg) {
                tree.push(StringType(1, c, tree.get_allocator()));
            }
            return *this;
        }
#endif
        auto get_allocator() -> BasicString& {
            return tree.get_allocator();
        }
        auto at(size_type pos) -> CharT {
            auto &sizes = tree.getSizes();
            if (pos >= tree.size()) {
                throw std::out_of_range("Rope::BasicString::at");
            }
            return getAtPos(pos);
        }
        auto at(size_type pos) const -> const CharT {
            auto &sizes = tree.getSizes();
            if (pos >= tree.size()) {
                throw std::out_of_range("Rope::BasicString::at");
            }
            return getAtPos(pos);
        }
        auto operator[](size_type pos) -> CharT {
            return getAtPos(pos);
        }
        auto operator[](size_type pos) const -> const CharT {
            return getAtPos(pos);
        }
        auto front() -> CharT& {
            auto &roots = tree.getRoots();
            auto &sizes = tree.getSizes();
            // get root node this string is located
            NodeType *root = roots.front().get();
            // get leaf and index in it
            while (root->left) {
                root = root->left.get();
            }
            return root->str.front();
        }
        auto front() const -> const CharT {
            auto &roots = tree.getRoots();
            auto &sizes = tree.getSizes();
            // get root node this string is located
            NodeType *root = roots.front().get();
            // get leaf and index in it
            while (root->left) {
                root = root->left.get();
            }
            return root->str.front();
        }
        auto back() -> CharT& {
            auto &roots = tree.getRoots();
            auto &sizes = tree.getSizes();
            // get root node this string is located
            NodeType *root = roots.back().get();
            // get leaf and index in it
            while (root->right) {
                root = root->right.get();
            }
            return root->str.back();
        }
        auto back() const -> const CharT {
            auto &roots = tree.getRoots();
            auto &sizes = tree.getSizes();
            // get root node this string is located
            NodeType *root = roots.back().get();
            // get leaf and index in it
            while (root->right) {
                root = root->right.get();
            }
            return root->str.back();
        }
        /*
         * Return raw tree reference
         */
        auto data() const -> const TreeType& {
            return tree;
        }
        /*Get C String. Note to use output to print into stream instead */
        auto c_str() const -> std::unique_ptr<CharT[], std::function<void(CharT*)>> {
            using AllocTraits = std::allocator_traits<Allocator>;
            Allocator alloc;

            std::size_t len = tree.size();
            CharT* cstr = AllocTraits::allocate(alloc, len + 1);

            std::size_t pos = 0;
            for (auto& root : tree.getRoots()) {
                for (auto leaf = root.get(); leaf; leaf = leaf->right.get()) {
                    std::copy(leaf->str.begin(), leaf->str.end(), cstr + pos);
                    pos += leaf->str.size();
                }
            }

            cstr[len] = CharT{};

            // wrap with unique_ptr and custom deleter
            return std::unique_ptr<CharT[], std::function<void(CharT*)>>(
                cstr,
                [alloc, len](CharT* p) mutable { AllocTraits::deallocate(alloc, p, len + 1); }
            );
        }
        auto begin() -> iterator {
            return iterator(tree, 0);
        }
        auto begin() const -> const_iterator {
            return const_iterator(tree, 0);
        }
        auto end() -> iterator {
            return iterator(tree, tree.size());
        }
        auto end() const -> const_iterator {
            return const_iterator(tree, tree.size());
        }
        auto cbegin() const -> const_iterator {
            return const_iterator(tree, 0);
        }
        auto cend() const -> const_iterator {
            return const_iterator(tree, tree.size());
        }
        auto rbegin() -> reverse_iterator {
            return reverse_iterator(tree, 0);
        }
        auto rend() -> reverse_iterator {
            return reverse_iterator(tree, tree.size());
        }
        auto crbegin() const -> reverse_const_iterator {
            return reverse_const_iterator(tree, 0);
        }
        auto crend() const -> reverse_const_iterator {
            return reverse_const_iterator(tree, tree.size());
        }
        auto empty() const -> bool {
            return tree.getSizes().front() == 0;
        }
        auto size() const -> size_type {
            return tree.size();
        }
        auto length() const -> size_type {
            return size();
        }
        auto clear() -> void {
            tree.clear();
        }
        // (1) insert count copies of a char at index
        auto insert(size_type index, size_type count, CharT ch) -> BasicString& {
            for (size_type i = 0; i < count; ++i)
                tree.insert(index + i, StringType(1, ch));
            return *this;
        }

        // (2) insert null-terminated char array at index
        auto insert(size_type index, const CharT* s) -> BasicString& {
            tree.insert(index, StringType(s));
            return *this;
        }

        // (3) insert char array with count at index
        auto insert(size_type index, const CharT* s, size_type count) -> BasicString& {
            tree.insert(index, StringType(s, count));
            return *this;
        }

        // (4) insert whole BasicString at index
        auto insert(size_type index, const BasicString& str) -> BasicString& {
            for (auto& root : str.tree.getRoots())
                tree.insert(index, root->str);
            return *this;
        }

        // (5) insert part of BasicString [s_index, s_index+count) at index
        auto insert(size_type index, const BasicString& str, size_type s_index, size_type count = std::string::npos) -> BasicString& {
            StringType tmp = str.tree.substr(s_index, count);
            tree.insert(index, tmp);
            return *this;
        }

        // (6) insert single char at iterator position
        auto insert(iterator pos, CharT ch) -> iterator {
            size_type index = pos - begin();
            insert(index, 1, ch);
            return begin() + index;
        }

        // (7) insert count copies of char at iterator position
        auto insert(const_iterator pos, size_type count, CharT ch) -> iterator {
            size_type index = pos - cbegin();
            insert(index, count, ch);
            return begin() + index;
        }

        // (8) insert range [first, last) at iterator position
        template<typename InputIt>
        auto insert(const_iterator pos, InputIt first, InputIt last) -> iterator {
            size_type index = pos - cbegin();
            for (auto it = first; it != last; ++it) {
                tree.insert(index++, StringType(1, *it));
            }
            return begin() + (pos - cbegin());
        }

        // (9) insert initializer_list at iterator position
        auto insert(const_iterator pos, std::initializer_list<CharT> ilist) -> iterator {
            return insert(pos, ilist.begin(), ilist.end());
        }

        // (10) insert StringViewLike at index
        template<typename SV>
        auto insert(size_type index, const SV& t) -> BasicString& {
            tree.insert(index, StringType(t));
            return *this;
        }
        // (10b) insert part of StringViewLike at index
        template<typename SV>
        auto insert(size_type index, const SV& t, size_type t_index, size_type count = npos) -> BasicString& {
            tree.insert(index, StringType(t, t_index, count));
            return *this;
        }
#ifdef __cpp_lib_from_range
        template<std::ranges::range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, CharT>
        auto insert_range(const_iterator pos, R&& rg) -> iterator {
            // compute index from iterator
            size_type index = pos - cbegin();

            // construct a temporary BasicString from the range
            BasicString tmp(std::from_range, std::forward<R>(rg), this->get_allocator());

            // insert it at index
            insert(index, tmp);

            // return iterator to the first inserted element
            return begin() + index;
        }
#endif
        auto erase(size_type index = 0, size_type count = StringType::npos) -> void {
            auto &roots = tree.getRoots();
            auto &sizes = tree.getSizes();

            std::size_t remaining = count;
            std::size_t global_index = index;

            while (remaining != 0 && global_index < tree.size()) {
                // find leaf and adjust index to local
                NodeType* leaf = nullptr;
                std::size_t local_index = global_index;
                for (auto& root : roots) {
                    leaf = root->getLeafByIndex(local_index);
                    if (leaf) break;
                }
                if (!leaf) break; // index out of range

                std::size_t erase_count = std::min(remaining, leaf->str.size() - local_index);
                leaf->str.erase(local_index, erase_count);

                remaining -= erase_count;
                global_index += erase_count;
            }
        }
        auto erase(const_iterator pos) {

        }
        auto erase(const_iterator begin, const_iterator end) {

        }
        auto push_back(CharT ch) -> void {
            tree.push(ch);
        }
        auto pop_back() -> void {
            auto &root = tree.getRoots().back();
            while (root->right) {
                root = root->right;
            }
            root->str.pop_back();
        }
        auto append(size_type count, CharT ch) -> BasicString& {
            tree.push(StringType {count, ch});
            return *this;
        }
        auto append(CharT *s, size_type count) -> BasicString& {
            tree.push(StringType {s, count});
            return *this;
        }
        auto append(CharT *s) -> BasicString& {
            tree.push(StringType {s});
            return *this;
        }
        template<typename SV>
        auto append(const SV &t) -> BasicString& {
            tree.push(StringType(t));
            return *this;
        }
        template<typename SV>
        auto append(const SV &t, size_type pos, size_type count = StringType::npos) -> BasicString& {
            tree.push(StringType(t, pos, count));
            return *this;
        }
        auto append(const StringType &s) -> BasicString& {
            tree.push(s);
            return *this;
        }
        auto append(const StringType &s, size_type pos, size_type count) -> BasicString& {
            tree.push(StringType(s, pos, count));
            return *this;
        }
        template< class InputIt >
        auto append(InputIt begin, InputIt end) -> BasicString& {
            tree.push(StringType(begin, end));
            return *this;
        }
        auto append(std::initializer_list<CharT> ilist) -> BasicString& {
            tree.push(StringType(ilist));
            return *this;
        }
#ifdef __cpp_lib_from_range
        template<std::ranges::range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, CharT>
        auto append_range(const_iterator pos, R&& rg) -> iterator {
            tree.push(StringType(pos, rg));
            return *this;
        }
#endif
        auto replace(size_type pos, size_type count, const BasicString &str) -> BasicString {
            NodeType *root = tree.getRootByIndex(pos);
            NodeType *node = root->getLeafByIndex(pos);
            node->str.replace(pos, count, str);
            return *this;
        }
        auto replace(const_iterator begin, const_iterator end, const BasicString &str) {}
        auto replace(size_type pos, size_type count, const BasicString& str, size_type pos2, size_type count2 = StringType::npos) {
            NodeType *root = tree.getRootByIndex(pos);
            NodeType *node = root->getLeafByIndex(pos);
            node->str.replace(pos, count, str, pos2, count2);
            return *this;
        }
        // (4) replace(pos, count, const CharT* cstr, size_type count2)
        auto replace(size_type pos, size_type count, const CharT* cstr, size_type count2) -> BasicString& {
            StringType tmp(cstr, count2);
            NodeType* root = tree.getRootByIndex(pos);
            NodeType* node = root->getLeafByIndex(pos);
            node->str.replace(pos, count, tmp);
            return *this;
        }
        auto replace(const_iterator first, const_iterator last, const CharT* cstr, size_type count2 ) -> BasicString {}
        // (6) replace(pos, count, const CharT* cstr)
        auto replace(size_type pos, size_type count, const CharT* cstr) -> BasicString& {
            StringType tmp(cstr);
            NodeType* root = tree.getRootByIndex(pos);
            NodeType* node = root->getLeafByIndex(pos);
            node->str.replace(pos, count, tmp);
            return *this;
        }
        auto replace( const_iterator first, const_iterator last, const CharT* cstr) -> BasicString& {};
        // (8) replace(pos, count, size_type count2, CharT ch)
        auto replace(size_type pos, size_type count, size_type count2, CharT ch) -> BasicString& {
            StringType tmp(count2, ch);
            NodeType* root = tree.getRootByIndex(pos);
            NodeType* node = root->getLeafByIndex(pos);
            node->str.replace(pos, count, tmp);
            return *this;
        }
        auto replace(const_iterator first, const_iterator last, size_type count2, CharT ch ) -> BasicString& {};
        template<typename InputIt>
        auto replace(const_iterator first, const_iterator last, InputIt first2, InputIt last2) -> BasicString& {}

        // (11) replace(pos, count, std::initializer_list<CharT> ilist)
        auto replace(size_type pos, size_type count, std::initializer_list<CharT> ilist) -> BasicString& {
            StringType tmp(ilist);
            NodeType* root = tree.getRootByIndex(pos);
            NodeType* node = root->getLeafByIndex(pos);
            node->str.replace(pos, count, tmp);
            return *this;
        }

        // (12) replace(pos, count, const StringViewLike& t)
        template<class StringViewLike>
        auto replace(size_type pos, size_type count, const StringViewLike& t) -> BasicString& {
            StringType tmp(t);
            NodeType* root = tree.getRootByIndex(pos);
            NodeType* node = root->getLeafByIndex(pos);
            node->str.replace(pos, count, tmp);
            return *this;
        }
        template<class StringViewLike>
        auto replace( const_iterator first, const_iterator last, const StringViewLike& t) {}
        // (14) replace(pos, count, const StringViewLike& t, size_type pos2, size_type count2 = StringType::npos)
        template<class StringViewLike>
        auto replace(size_type pos, size_type count, const StringViewLike& t,
                     size_type pos2, size_type count2 = StringType::npos) -> BasicString& {
            StringType tmp(t);
            tmp = tmp.substr(pos2, count2);
            NodeType* root = tree.getRootByIndex(pos);
            NodeType* node = root->getLeafByIndex(pos);
            node->str.replace(pos, count, tmp);
            return *this;
        }
#ifdef __cpp_lib_from_range
        template<std::ranges::range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, CharT>
        auto replace_range(const_iterator begin, const_iterator end, R&& rg) -> iterator {
            return replace(begin, end, StringType(std::from_range, std::forward<R>(rg), get_allocator()));
        }
#endif
        auto copy(CharT* dest, size_type count, size_type pos = 0) const -> size_type {
            if (pos >= tree.size()) {
                return 0; // nothing to copy if starting beyond size
            }

            size_type written = 0;      // number of chars written
            size_type remaining = count; // how many still need copying
            size_type global_index = 0;  // global position while walking leaves

            for (auto& root : tree.getRoots()) {
                for (auto leaf = root; leaf; leaf = leaf->right.get()) {
                    auto leaf_size = leaf->str.size();

                    // skip this leaf if still before pos
                    if (global_index + leaf_size <= pos) {
                        global_index += leaf_size;
                        continue;
                    }

                    // compute where in this leaf we start
                    size_type local_start = (pos > global_index) ? (pos - global_index) : 0;
                    size_type available = leaf_size - local_start;
                    size_type to_copy = std::min(available, remaining);

                    std::copy_n(leaf->str.data() + local_start, to_copy, dest + written);

                    written += to_copy;
                    remaining -= to_copy;
                    global_index += leaf_size;

                    if (remaining == 0) {
                        return written;
                    }
                }
            }

            return written;
        }
        auto find(const BasicString& str, size_type pos = 0) const -> size_type {
            if (str.empty()) {
                return pos <= size() ? pos : StringType::npos;
            }
            if (pos >= size() || str.size() > size() - pos) {
                return StringType::npos;
            }

            size_type haystack_size = size();
            size_type needle_size   = str.size();

            // Walk the rope character by character (slow but correct)
            for (size_type i = pos; i + needle_size <= haystack_size; ++i) {
                bool match = true;
                for (size_type j = 0; j < needle_size; ++j) {
                    if (getAtPos(i + j) != str[j]) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    return i;
                }
            }

            return StringType::npos;
        }
        auto operator=(const BasicString& other) -> BasicString& {
            tree = other.tree;
            return *this;
        }
        auto operator=(const BasicString&& other) noexcept -> BasicString& {
            tree = std::move(other.tree);
            return *this;
        }
        auto operator=(const CharT *s) -> BasicString& {
            tree.clear();
            tree.push(StringType(s));
            return *this;
        }
        auto operator=(CharT ch) -> BasicString& {
            tree.clear();
            tree.push(StringType(1, ch));
            return *this;
        }
        auto operator=(std::initializer_list<CharT> ilist) {
            tree.clear();
            tree.push(StringType(ilist));
        }
        template<class StringViewLike>
        auto operator=(const StringViewLike& t) -> BasicString& {
            tree.clear();
            tree.push(StringType(t));
            return *this;
        }
        auto operator=(std::nullptr_t) -> BasicString& = delete;
        auto operator==(const BasicString& other) const -> bool {
            return tree == other.tree;
        }
        auto operator==(const StringType &other) const -> bool {
            if (size() != other.size()) {
                return false;
            }
            auto this_ptr = begin();
            auto other_ptr = other.begin();
            while (this_ptr != end()) {
                if (*this_ptr != *other_ptr) {
                    return false;
                }
            }
            return true;
        }
        auto operator==(const CharT *s) const -> bool {
            if (size() != std::strlen(s)) {
                return false;
            }
            std::size_t count = 0;
            for (const auto c : *this) {
                if (c != s[count++])
                    return false;
            }
            return true;
        }
        auto operator==(std::nullptr_t) const -> bool = delete;
    private:
        Tree<CharT, Traits, Allocator> tree;

        auto getAtPos(size_type pos) const -> CharT& {
            auto &roots = tree.getRoots();
            auto &sizes = tree.getSizes();
            // get root node this string is located
            auto root_index = tree.getRootByIndex(pos);
            // get leaf and index in it
            NodeType *leaf = roots[root_index]->getLeafByIndex(pos);
            // return character at that index
            return leaf->str[pos];
        }
    };
}
#endif //ISPA_BASICSTRING_H