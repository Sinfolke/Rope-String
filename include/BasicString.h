#ifndef ROPE_BASICSTRING_H
#define ROPE_BASICSTRING_H
#include <Node.h>
#include <Tree.h>
#include <vector>
#include <string>
#include <functional>
#include <ranges>
#include <cstring>

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
        template<typename CharType>
        class iterator {
            using TreeType = Tree<CharType, Traits, Allocator>;
        public:
            using value_type        = CharType;
            using difference_type   = std::ptrdiff_t;
            using pointer           = CharType*;
            using reference         = CharType&;
            using iterator_category = std::forward_iterator_tag;

            explicit iterator(TreeType &tree, std::size_t pos) : tree_(tree) {
                auto size = tree_.size();
                if (pos >= size) {
                    current = nullptr;
                    global_pos = size;
                    return;
                }
                std::size_t offset = 0;
                global_pos = pos;
                current = tree.getLeafByIndex(pos, offset);
                this->pos = offset;
            }
            explicit iterator(TreeType &tree) : tree_(tree) {}
            auto operator*() const -> CharT {
                return current->str[pos];
            }

            auto operator++() -> iterator& {
                auto size = tree_.size();
                if (global_pos + 1 >= size) {
                    global_pos = size;
                    current = nullptr;
                    return *this;
                }
                global_pos++;
                if (pos + 1 >= current->str.size()) {
                    current = tree_.nextLeaf(current);
                    pos = 0;
                } else {
                    ++pos;
                }
                return *this;
            }

            auto operator++(int) -> iterator {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            auto operator==(const iterator& other) const -> bool {
                return current == other.current && global_pos == other.global_pos;
            }

            auto operator!=(const iterator& other) const -> bool {
                return !(*this == other);
            }

            auto position() const -> std::size_t { return global_pos; }

        protected:
            TreeType &tree_;
            std::size_t pos;
            std::size_t global_pos;
            NodeType *current = nullptr;
        };
        template<typename CharType>
        class reverse_iterator : public iterator<CharType> {
            using Base = iterator<CharType>;
            using TreeType = Tree<CharType, Traits, Allocator>;
        public:
            explicit reverse_iterator(TreeType &tree, std::size_t pos = 0) : Base(tree) {
                auto size = tree.size();
                if (size - pos - 1 == std::size_t(-1)) {
                    Base::current = nullptr;
                    Base::global_pos = std::size_t(-1);
                    return;
                }
                std::size_t offset = 0;
                Base::global_pos = size - pos - 1;
                Base::current = tree.getLeafByIndex(Base::global_pos, offset);
                Base::pos = Base::current->str.size() - offset;
            }
            auto operator++() -> reverse_iterator& {
                if (Base::global_pos - 1 == std::size_t(-1)) {
                    Base::global_pos = std::size_t(-1);
                    Base::current = nullptr;
                    return *this;
                }
                Base::global_pos--;
                if (Base::pos - 1 == std::size_t(-1)) {
                    Base::current = Base::tree_.prevLeaf(Base::current);
                    Base::pos = Base::current->str.size() - 1;
                } else {
                    --Base::pos;
                }
                return *this;
            }

            auto operator++(int) -> reverse_iterator {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }
        };
        using const_iterator = iterator<const CharT>;
        using reverse_const_iterator = reverse_iterator<const CharT>;
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

        void print() {
            // no-op: debug print suppressed
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
        auto get_allocator() const -> allocator_type {
            return tree.get_allocator();
        }
        auto at(size_type pos) -> CharT {
            if (pos >= tree.size()) {
                throw std::out_of_range("Rope::BasicString::at");
            }
            return getAtPos(pos);
        }
        auto at(size_type pos) const -> const CharT {
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
            for (auto &r : tree.getRoots()) {
                NodeType* leaf = r.first.get();
                while (leaf->left) leaf = leaf->left.get();
                if (!leaf->str.empty()) return leaf->str.front();
            }
            throw std::out_of_range("rope is empty");
        }
        auto front() const -> const CharT& {
            for (auto &r : tree.getRoots()) {
                NodeType* leaf = r.first.get();
                while (leaf->left) leaf = leaf->left.get();
                if (!leaf->str.empty()) return leaf->str.front();
            }
            throw std::out_of_range("rope is empty");
        }
        auto back() -> CharT& {
            for (auto it = tree.getRoots().rbegin(); it != tree.getRoots().rend(); ++it) {
                NodeType* leaf = it->first.get();
                while (leaf->right) leaf = leaf->right.get();
                if (!leaf->str.empty()) return leaf->str.back();
            }
            throw std::out_of_range("rope is empty");
        }
        auto back() const -> const CharT& {
            for (auto it = tree.getRoots().rbegin(); it != tree.getRoots().rend(); ++it) {
                NodeType* leaf = it->first.get();
                while (leaf->right) leaf = leaf->right.get();
                if (!leaf->str.empty()) return leaf->str.back();
            }
            throw std::out_of_range("rope is empty");
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
                for (auto leaf = root.first.get(); leaf; leaf = leaf->right.get()) {
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
        auto begin() -> iterator<CharT> {
            return iterator<CharT>(tree, 0);
        }
        auto begin() const -> const_iterator {
            return const_iterator(tree, 0);
        }
        auto end() -> iterator<CharT> {
            return iterator<CharT>(tree, tree.size());
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
        auto rbegin() -> reverse_iterator<CharT> {
            return reverse_iterator<CharT>(tree, 0);
        }
        auto rend() -> reverse_iterator<CharT> {
            return reverse_iterator<CharT>(tree, tree.size());
        }
        auto crbegin() const -> reverse_const_iterator {
            return reverse_const_iterator(tree, 0);
        }
        auto crend() const -> reverse_const_iterator {
            return reverse_const_iterator(tree, tree.size());
        }
        auto empty() const -> bool {
            return tree.getRoots().front().second == 0;
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
        auto insert(iterator<CharT> pos, CharT ch) -> iterator<CharT> {
            size_type index = pos.position();
            insert(index, 1, ch);
            return iterator<CharT>(tree, index);
        }

        // (7) insert count copies of char at iterator position
        auto insert(const_iterator pos, size_type count, CharT ch) -> iterator<CharT> {
            size_type index = pos.position();
            insert(index, count, ch);
            return iterator<CharT>(tree, index);
        }

        // (8) insert range [first, last) at iterator position
        template<typename InputIt>
        auto insert(const_iterator pos, InputIt first, InputIt last) -> iterator<CharT> {
            size_type index = pos.position();
            for (auto it = first; it != last; ++it) {
                tree.insert(index++, StringType(1, *it));
            }
            return iterator<CharT>(tree, pos.position());
        }

        // (9) insert initializer_list at iterator position
        auto insert(const_iterator pos, std::initializer_list<CharT> ilist) -> iterator<CharT> {
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
            size_type total = size();
            if (index >= total) return;
            StringType flat;
            flat.resize(total);
            copy(flat.data(), total, 0);
            flat.erase(index, count);
            tree.clear();
            if (!flat.empty()) {
                tree.push(flat);
            }
        }
        auto erase(const_iterator pos) -> void {
            auto idx = pos.position();
            erase(idx, 1);
        }
        auto erase(const_iterator first, const_iterator last) -> void {
            auto b = first.position();
            auto e = last.position();
            if (e <= b) return;
            erase(b, e - b);
        }
        auto push_back(CharT ch) -> void {
            tree.push(StringType (1, ch));
        }
        auto pop_back() -> void {
            auto root = tree.getRoots().back().first;
            while (root->right) {
                root = root->right;
            }
            root->str.pop_back();
            tree.getRoots().back().second--;
        }
        auto append(size_type count, CharT ch) -> BasicString& {
            tree.push(StringType(count, ch));
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
        auto replace(const_iterator first, const_iterator last, const BasicString &str) -> BasicString& {
                    auto b = first.position();
                    auto e = last.position();
                    if (e < b) std::swap(b, e);
                    size_type total = size();
                    std::basic_string<CharT, Traits, Allocator> flat;
                    flat.resize(total);
                    copy(flat.data(), total, 0);
                    // build replacement from str by flattening
                    std::basic_string<CharT, Traits, Allocator> repl;
                    repl.resize(str.size());
                    str.copy(repl.data(), repl.size(), 0);
                    flat.replace(b, e - b, repl);
                    tree.clear();
                    if (!flat.empty()) tree.push(flat);
                    return *this;
                }
        auto replace(size_type pos, size_type count, const BasicString& str, size_type pos2, size_type count2 = StringType::npos) {
            NodeType *root = tree.getRootByIndex(pos);
            NodeType *node = root->getLeafByIndex(pos);
            node->str.replace(pos, count, str, pos2, count2);
            return *this;
        }
        // (4) replace(pos, count, const CharT* cstr, size_type count2)
        auto replace(size_type pos, size_type count, const CharT* cstr, size_type count2) -> BasicString& {
            // Flatten, replace, rebuild
            size_type total = size();
            if (pos > total) return *this; // nothing to do if pos beyond end
            StringType flat;
            flat.resize(total);
            copy(flat.data(), total, 0);
            flat.replace(pos, count, cstr, count2);
            tree.clear();
            if (!flat.empty()) tree.push(flat);
            return *this;
        }
        auto replace(const_iterator first, const_iterator last, const CharT* cstr, size_type count2 ) -> BasicString& {
            auto b = first.position();
            auto e = last.position();
            if (e < b) std::swap(b, e);
            size_type total = size();
            std::basic_string<CharT, Traits, Allocator> flat;
            flat.resize(total);
            copy(flat.data(), total, 0);
            flat.replace(b, e - b, cstr, count2);
            tree.clear();
            if (!flat.empty()) tree.push(flat);
            return *this;
        }
        // (6) replace(pos, count, const CharT* cstr)
        auto replace(size_type pos, size_type count, const CharT* cstr) -> BasicString& {
            // Flatten, replace, rebuild
            size_type total = size();
            if (pos > total) return *this; // nothing to do if pos beyond end
            StringType flat;
            flat.resize(total);
            copy(flat.data(), total, 0);
            flat.replace(pos, count, cstr);
            tree.clear();
            if (!flat.empty()) tree.push(flat);
            return *this;
        }
        auto replace( const_iterator first, const_iterator last, const CharT* cstr) -> BasicString& {
            auto b = first.position();
            auto e = last.position();
            if (e < b) std::swap(b, e);
            size_type total = size();
            std::basic_string<CharT, Traits, Allocator> flat;
            flat.resize(total);
            copy(flat.data(), total, 0);
            flat.replace(b, e - b, cstr);
            tree.clear();
            if (!flat.empty()) tree.push(flat);
            return *this;
        }
        // (8) replace(pos, count, size_type count2, CharT ch)
        auto replace(size_type pos, size_type count, size_type count2, CharT ch) -> BasicString& {
            StringType tmp(count2, ch);
            NodeType* root = tree.getRootByIndex(pos);
            NodeType* node = root->getLeafByIndex(pos);
            node->str.replace(pos, count, tmp);
            return *this;
        }
        auto replace(const_iterator first, const_iterator last, size_type count2, CharT ch ) -> BasicString& {
            auto b = first.position();
            auto e = last.position();
            if (e < b) std::swap(b, e);
            size_type total = size();
            std::basic_string<CharT, Traits, Allocator> flat;
            flat.resize(total);
            copy(flat.data(), total, 0);
            std::basic_string<CharT, Traits, Allocator> repl(count2, ch);
            flat.replace(b, e - b, repl);
            tree.clear();
            if (!flat.empty()) tree.push(flat);
            return *this;
        }
        template<typename InputIt>
        auto replace(const_iterator first, const_iterator last, InputIt first2, InputIt last2) -> BasicString& {
            auto b = first.position();
            auto e = last.position();
            if (e < b) std::swap(b, e);
            size_type total = size();
            std::basic_string<CharT, Traits, Allocator> flat;
            flat.resize(total);
            copy(flat.data(), total, 0);
            std::basic_string<CharT, Traits, Allocator> repl(first2, last2);
            flat.replace(b, e - b, repl);
            tree.clear();
            if (!flat.empty()) tree.push(flat);
            return *this;
        }

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
        auto replace( const_iterator first, const_iterator last, const StringViewLike& t) -> BasicString& {
            auto b = first.position();
            auto e = last.position();
            if (e < b) std::swap(b, e);
            size_type total = size();
            std::basic_string<CharT, Traits, Allocator> flat;
            flat.resize(total);
            copy(flat.data(), total, 0);
            std::basic_string<CharT, Traits, Allocator> repl(t);
            flat.replace(b, e - b, repl);
            tree.clear();
            if (!flat.empty()) tree.push(flat);
            return *this;
        }
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
        auto replace_with_range(const_iterator begin, const_iterator end, R&& rg) -> iterator {
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
                for (auto leaf = root.first.get(); leaf; leaf = leaf->right.get()) {
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
            if (size() != other.size()) {
                return false;
            }
            auto this_ptr = begin();
            auto other_ptr = other.begin();
            while (this_ptr != end()) {
                if (*this_ptr++ != *other_ptr++) {
                    return false;
                }
            }
            return true;        }
        auto operator==(const StringType &other) const -> bool {
            if (size() != other.size()) {
                return false;
            }
            auto this_ptr = begin();
            auto other_ptr = other.begin();
            while (this_ptr != end()) {
                if (!Traits::eq(*this_ptr, *other_ptr)) {
                    return false;
                }
                ++this_ptr;
                ++other_ptr;
            }
            return true;
        }
        auto operator==(const CharT *s) const -> bool {
            if (!s) return false;
            size_type n = Traits::length(s);
            if (size() != n) {
                return false;
            }
            // Compare without relying on iterators
            std::vector<CharT> buf(n);
            copy(buf.data(), n, 0);
            for (size_type i = 0; i < n; ++i) {
                if (!Traits::eq(buf[i], s[i])) return false;
            }
            return true;
        }
        auto operator==(std::nullptr_t) const -> bool = delete;

        // operator+= overloads
        auto operator+=(const BasicString& str) -> BasicString& {
            // append contents of another rope without flattening
            for (auto& root : str.tree.getRoots()) {
                tree.push(root->str);
            }
            return *this;
        }
        auto operator+=(CharT ch) -> BasicString& {
            push_back(ch);
            return *this;
        }
        auto operator+=(const CharT* s) -> BasicString& {
            if (!s) return *this; // ignore null pointer
            tree.push(StringType(s));
            return *this;
        }
        auto operator+=(std::initializer_list<CharT> ilist) -> BasicString& {
            tree.push(StringType(ilist));
            return *this;
        }
        template<class StringViewLike>
        auto operator+=(const StringViewLike& t) -> BasicString& {
            tree.push(StringType(t));
            return *this;
        }
        // resize to count, filling with value-initialized CharT if growing
        auto resize(size_type count) -> void {
            size_type cur = size();
            if (count == cur) return;
            if (count < cur) {
                erase(count, npos);
            } else {
                append(count - cur, CharT{});
            }
        }
        // resize to count with specified fill character
        auto resize(size_type count, CharT ch) -> void {
            size_type cur = size();
            if (count == cur) return;
            if (count < cur) {
                erase(count, npos);
            } else {
                append(count - cur, ch);
            }
        }
        // C++23 resize_and_overwrite: provide buffer of size count and let op return new size
        template<class Operation>
        auto resize_and_overwrite(size_type count, Operation op) -> void {
            // prepare buffer of requested size, seeded with current contents up to count
            std::basic_string<CharT, Traits, Allocator> buf;
            buf.resize(count);
            size_type to_copy = std::min(count, size());
            if (to_copy) {
                copy(buf.data(), to_copy, 0);
            }
            // call user operation; it should return the number of chars written [0..count]
            auto new_size = static_cast<size_type>(op(buf.data(), count));
            if (new_size > count) new_size = count;
            // rebuild rope from buffer prefix
            tree.clear();
            if (new_size) {
                tree.push(std::basic_string<CharT, Traits, Allocator>(buf.data(), new_size));
            }
        }
        // swap contents
        auto swap(BasicString& other) noexcept -> void {
            using std::swap;
            swap(this->tree, other.tree);
        }
        // substring
        auto substr(size_type pos = 0, size_type count = npos) const -> BasicString {
            size_type total = size();
            if (pos > total) pos = total; // yield empty
            size_type len = (count == npos) ? (total - pos) : std::min(count, total - pos);
            std::basic_string<CharT, Traits, Allocator> flat;
            flat.resize(total);
            copy(flat.data(), total, 0);
            BasicString result;
            if (len) {
                result.assign(flat.data() + pos, len);
            } else {
                result.clear();
            }
            return result;
        }
        // compare helpers
        auto compare(const BasicString& other) const -> int {
            size_type n1 = size();
            size_type n2 = other.size();
            size_type nmin = std::min(n1, n2);
            // compare via incremental traversal
            for (size_type i = 0; i < nmin; ++i) {
                CharT a = getAtPos(i);
                CharT b = other.getAtPos(i);
                if (Traits::lt(a, b)) return -1;
                if (Traits::lt(b, a)) return 1;
            }
            if (n1 < n2) return -1;
            if (n1 > n2) return 1;
            return 0;
        }
        auto compare(const std::basic_string<CharT, Traits, Allocator>& s) const -> int {
            size_type n1 = size();
            size_type n2 = s.size();
            size_type nmin = std::min(n1, n2);
            for (size_type i = 0; i < nmin; ++i) {
                CharT a = getAtPos(i);
                CharT b = s[i];
                if (Traits::lt(a, b)) return -1;
                if (Traits::lt(b, a)) return 1;
            }
            if (n1 < n2) return -1;
            if (n1 > n2) return 1;
            return 0;
        }
        auto compare(const CharT* s) const -> int {
            if (!s) return 1; // non-empty > null
            size_type n2 = Traits::length(s);
            size_type n1 = size();
            size_type nmin = std::min(n1, n2);
            for (size_type i = 0; i < nmin; ++i) {
                CharT a = getAtPos(i);
                CharT b = s[i];
                if (Traits::lt(a, b)) return -1;
                if (Traits::lt(b, a)) return 1;
            }
            if (n1 < n2) return -1;
            if (n1 > n2) return 1;
            return 0;
        }
        // starts_with / ends_with / contains
        auto starts_with(const std::basic_string<CharT, Traits, Allocator>& s) const -> bool {
            return find(s, 0) == 0 || (s.empty() && true);
        }
        auto starts_with(const BasicString& other) const -> bool {
            return find(other, 0) == 0 || (other.empty() && true);
        }
        auto starts_with(const CharT* s) const -> bool {
            if (!s) return false;
            std::basic_string<CharT, Traits, Allocator> tmp(s);
            return starts_with(tmp);
        }
        auto starts_with(CharT ch) const -> bool {
            return size() > 0 && Traits::eq(getAtPos(0), ch);
        }
        auto ends_with(const std::basic_string<CharT, Traits, Allocator>& s) const -> bool {
            size_type n = size();
            size_type m = s.size();
            if (m > n) return false;
            return substr(n - m).operator==(s);
        }
        auto ends_with(const BasicString& other) const -> bool {
            size_type n = size();
            size_type m = other.size();
            if (m > n) return false;
            return substr(n - m).operator==(other);
        }
        auto ends_with(const CharT* s) const -> bool {
            if (!s) return false;
            std::basic_string<CharT, Traits, Allocator> tmp(s);
            return ends_with(tmp);
        }
        auto ends_with(CharT ch) const -> bool {
            return size() > 0 && Traits::eq(getAtPos(size() - 1), ch);
        }
        auto contains(const std::basic_string<CharT, Traits, Allocator>& s) const -> bool {
            return find(s, 0) != npos;
        }
        auto contains(const BasicString& other) const -> bool {
            return find(other, 0) != npos;
        }
        auto contains(const CharT* s) const -> bool {
            if (!s) return false;
            std::basic_string<CharT, Traits, Allocator> tmp(s);
            return contains(tmp);
        }
        auto contains(CharT ch) const -> bool {
            return find(ch) != npos;
        }
        // find overloads using flattened string for simplicity
        auto find(CharT ch, size_type pos = 0) const -> size_type {
            size_type n = size();
            for (size_type i = pos; i < n; ++i) if (Traits::eq(getAtPos(i), ch)) return i;
            return npos;
        }
        auto find(const std::basic_string<CharT, Traits, Allocator>& s, size_type pos = 0) const -> size_type {
            // use flattened haystack to delegate to std::basic_string::find
            if (s.empty()) return pos <= size() ? pos : npos;
            std::basic_string<CharT, Traits, Allocator> flat;
            size_type total = size();
            flat.resize(total);
            copy(flat.data(), total, 0);
            return flat.find(s, pos);
        }
        auto find(const CharT* s, size_type pos = 0) const -> size_type {
            if (!s) return npos;
            std::basic_string<CharT, Traits, Allocator> needle(s);
            return find(needle, pos);
        }
        auto rfind(CharT ch, size_type pos = npos) const -> size_type {
            if (empty()) return npos;
            size_type i = std::min(pos, size() - 1);
            for (;;){
                if (Traits::eq(getAtPos(i), ch)) return i;
                if (i == 0) break;
                --i;
            }
            return npos;
        }
        auto rfind(const std::basic_string<CharT, Traits, Allocator>& s, size_type pos = npos) const -> size_type {
            // flatten
            std::basic_string<CharT, Traits, Allocator> flat;
            size_type total = size();
            flat.resize(total);
            copy(flat.data(), total, 0);
            return flat.rfind(s, pos);
        }
        auto rfind(const CharT* s, size_type pos = npos) const -> size_type {
            if (!s) return npos;
            return rfind(std::basic_string<CharT, Traits, Allocator>(s), pos);
        }
        auto find_first_of(const std::basic_string<CharT, Traits, Allocator>& s, size_type pos = 0) const -> size_type {
            size_type n = size();
            for (size_type i = pos; i < n; ++i) {
                if (s.find(getAtPos(i)) != std::basic_string<CharT, Traits, Allocator>::npos) return i;
            }
            return npos;
        }
        auto find_first_of(const CharT* s, size_type pos = 0) const -> size_type {
            if (!s) return npos;
            return find_first_of(std::basic_string<CharT, Traits, Allocator>(s), pos);
        }
        auto find_first_not_of(const std::basic_string<CharT, Traits, Allocator>& s, size_type pos = 0) const -> size_type {
            size_type n = size();
            for (size_type i = pos; i < n; ++i) {
                if (s.find(getAtPos(i)) == std::basic_string<CharT, Traits, Allocator>::npos) return i;
            }
            return npos;
        }
        auto find_first_not_of(const CharT* s, size_type pos = 0) const -> size_type {
            if (!s) return npos;
            return find_first_not_of(std::basic_string<CharT, Traits, Allocator>(s), pos);
        }
        auto find_last_of(const std::basic_string<CharT, Traits, Allocator>& s, size_type pos = npos) const -> size_type {
            if (empty()) return npos;
            size_type i = std::min(pos, size() - 1);
            for (;;){
                if (s.find(getAtPos(i)) != std::basic_string<CharT, Traits, Allocator>::npos) return i;
                if (i == 0) break;
                --i;
            }
            return npos;
        }
        auto find_last_of(const CharT* s, size_type pos = npos) const -> size_type {
            if (!s) return npos;
            return find_last_of(std::basic_string<CharT, Traits, Allocator>(s), pos);
        }
        auto find_last_not_of(const std::basic_string<CharT, Traits, Allocator>& s, size_type pos = npos) const -> size_type {
            if (empty()) return npos;
            size_type i = std::min(pos, size() - 1);
            for (;;){
                if (s.find(getAtPos(i)) == std::basic_string<CharT, Traits, Allocator>::npos) return i;
                if (i == 0) break;
                --i;
            }
            return npos;
        }
        auto find_last_not_of(const CharT* s, size_type pos = npos) const -> size_type {
            if (!s) return npos;
            return find_last_not_of(std::basic_string<CharT, Traits, Allocator>(s), pos);
        }
    private:
        Tree<CharT, Traits, Allocator> tree;

        auto getAtPos(size_type pos) const -> CharT& {
            auto &roots = tree.getRoots();
            std::size_t offset = 0;
            NodeType *leaf = nullptr;

            // Use cached root sizes to locate the leaf quickly
            for (std::size_t i = 0; i < roots.size(); ++i) {
                auto root_size = roots[i].second;
                if (pos < offset + root_size) {
                    pos -= offset; // make pos local to the selected root
                    leaf = roots[i].first->getLeafByIndex(pos);
                    break;
                }
                offset += root_size;
            }

            return leaf->str[pos]; // pos is now local to the leaf
        }
    };
}
#endif //ROPE_BASICSTRING_H