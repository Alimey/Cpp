#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>

template <size_t N>
class StackStorage {
public:
    alignas(std::max_align_t) std::byte storage_[N];
    size_t slider_ = 0;

    StackStorage() = default;

    StackStorage(const StackStorage&) = delete;
    StackStorage& operator=(const StackStorage&) = delete;

    template <typename T>
    T* AllocateMemory(size_t count) {
        void* objects_begining = static_cast<void*>(storage_ + slider_);
        size_t storage_size = N - slider_;

        T* result = static_cast<T*>(
            std::align(alignof(T), count * sizeof(T), objects_begining, storage_size));

        if (result == nullptr) {
            throw std::bad_alloc();
        }

        slider_ = N - storage_size + sizeof(T) * count;

        return result;
    }

    ~StackStorage() = default;
};

template <typename T, size_t N>
class StackAllocator {
public:
    StackStorage<N>* memory_;

    StackAllocator(StackStorage<N>& buffer)
        : memory_(&buffer) {
    }

    StackAllocator() = delete;

    StackAllocator(const StackAllocator& another) = default;

    StackAllocator& operator=(StackAllocator another) {
        memory_ = another.memory_;
        return *this;
    }

    T* allocate(size_t count) {
        T* result = memory_->template AllocateMemory<T>(count);
        return result;
    }

    void deallocate(T*, size_t) {
    }

    bool operator==(const StackAllocator&) const = default;

    template <typename U>
    StackAllocator(const StackAllocator<U, N>& another)
        : memory_(another.memory_) {
    }

    template <typename U>
    struct rebind {
        using other = StackAllocator<U, N>;
    };

    using value_type = T;

    ~StackAllocator() = default;
};

template <typename T, typename Alloc = std::allocator<T>>
class List {
    struct BaseNode {
        BaseNode* prev;
        BaseNode* next;
    };

    struct Node : BaseNode {
        T value;

        Node(const T& value)
            : BaseNode{nullptr, nullptr},
              value(value) {
        }

        Node() = default;
    };

    BaseNode fake_node_;
    size_t size_;

    using basic_allocator_traits = std::allocator_traits<Alloc>;
    using NodeAlloc = typename basic_allocator_traits::template rebind_alloc<Node>;
    using allocator_traits = std::allocator_traits<NodeAlloc>;

    [[no_unique_address]] NodeAlloc alloc_;

public:
    template <bool is_const>
    struct BaseIterator {
        friend List;
        using value_type = T;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ssize_t;
        using pointer = std::conditional_t<is_const, const T*, T*>;
        using reference = std::conditional_t<is_const, const T&, T&>;

        using BaseNodePointerType = std::conditional_t<is_const, const BaseNode*, BaseNode*>;
        using NodePointerType = std::conditional_t<is_const, const Node*, Node*>;

    private:
        BaseNodePointerType base_node_;

        BaseNode* getNotConstBase() const {
            return const_cast<BaseNode*>(base_node_);
        }

    public:
        operator BaseIterator<true>() const {
            return BaseIterator<true>(base_node_);
        }

        BaseIterator(BaseNodePointerType base)
            : base_node_(base) {
        }

        BaseIterator& operator++() {
            base_node_ = base_node_->next;
            return *this;
        }

        BaseIterator& operator--() {
            base_node_ = base_node_->prev;
            return *this;
        }

        BaseIterator operator++(int) {
            auto remember = *this;
            base_node_ = base_node_->next;
            return remember;
        }

        BaseIterator operator--(int) {
            auto remember = *this;
            base_node_ = base_node_->prev;
            return remember;
        }

        reference operator*() const {
            NodePointerType node = static_cast<NodePointerType>(base_node_);
            return node->value;
        }

        pointer operator->() const {
            NodePointerType node = static_cast<NodePointerType>(base_node_);
            return &(operator*());
        }

        bool operator==(const BaseIterator& another) const = default;
    };

    using value_type = T;
    using iterator = BaseIterator<false>;
    using const_iterator = BaseIterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() {
        return iterator(fake_node_.next);
    }

    iterator end() {
        return iterator(&fake_node_);
    }

    const_iterator begin() const {
        return const_iterator(fake_node_.next);
    }

    const_iterator end() const {
        return const_iterator(&fake_node_);
    }

    const_iterator cbegin() const {
        return const_iterator(begin());
    }

    const_iterator cend() const {
        return const_iterator(end());
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }

    reverse_iterator crbegin() const {
        return reverse_iterator(cend());
    }

    reverse_iterator crend() const {
        return reverse_iterator(cbegin());
    }

private:
    void primitive_swap(List& another) {
        std::swap(another.size_, size_);
        std::swap(another.fake_node_.next, fake_node_.next);
        std::swap(another.fake_node_.prev, fake_node_.prev);
        fake_node_.prev->next = &fake_node_;
        fake_node_.next->prev = &fake_node_;
    }

    void swap_no_alloc(List& another) {
        if (empty() && another.empty()) {
            return;
        }

        primitive_swap(another);
        if (another.empty() || empty()) {
            another.fake_node_.next = &another.fake_node_;
            another.fake_node_.prev = &another.fake_node_;
            return;
        }

        another.fake_node_.prev->next = &another.fake_node_;
        another.fake_node_.next->prev = &another.fake_node_;
    }

public:
    void swap(List& another) {
        if constexpr (allocator_traits::propagate_on_container_swap::value) {
            std::swap(alloc_, another.alloc_);
        }
        swap_no_alloc(another);
    }

    explicit List(const NodeAlloc& alloc)
        : fake_node_{&fake_node_, &fake_node_},
          size_(0),
          alloc_(alloc) {
    }

    List()
        : List(NodeAlloc()) {
    }

    List(size_t count, const NodeAlloc& alloc = NodeAlloc())
        : List(alloc) {
        for (size_t i = 0; i < count; ++i) {
            push_back();
        }
    }

    List(size_t count, const T& value, const NodeAlloc& alloc = NodeAlloc())
        : List(alloc) {
        for (size_t i = 0; i < count; ++i) {
            push_back(value);
        }
    }

    List(const List& another)
        : List(allocator_traits::select_on_container_copy_construction(another.alloc_)) {
        for (auto it = another.begin(); it != another.end(); ++it) {
            push_back(*it);
        }
    }

    List& operator=(const List& another) {
        if (&another == this) {
            return *this;
        }

        NodeAlloc new_alloc = allocator_traits::propagate_on_container_copy_assignment::value
                                  ? another.alloc_
                                  : alloc_;

        List copy(new_alloc);
        for (auto it = another.begin(); it != another.end(); ++it) {
            copy.push_back(*it);
        }

        swap(copy);
        if constexpr (!allocator_traits::propagate_on_container_swap::value) {
            std::swap(alloc_, copy.alloc_);
        }

        return *this;
    }

    template <typename... Args>
    iterator emplace(const_iterator it, Args&&... args) {
        Node* node = allocator_traits::allocate(alloc_, 1);

        try {
            allocator_traits::construct(alloc_, node, std::forward<Args>(args)...);
        } catch (...) {
            allocator_traits::deallocate(alloc_, node, 1);
            throw;
        }

        node->prev = it.getNotConstBase()->prev;
        it.getNotConstBase()->prev = node;

        node->next = it.getNotConstBase();
        node->prev->next = node;

        ++size_;

        return iterator{node};
    }

    iterator insert(const_iterator it, const T& value) {
        return emplace(it, value);
    }

    iterator insert(const_iterator it) {
        return emplace(it);
    }

    iterator erase(const_iterator it) {
        auto next_it = std::next(it);

        iterator ans{next_it.getNotConstBase()};

        auto prev = it.getNotConstBase()->prev;
        auto next = it.getNotConstBase()->next;

        prev->next = next;
        next->prev = prev;

        allocator_traits::destroy(alloc_, static_cast<Node*>(it.getNotConstBase()));
        allocator_traits::deallocate(alloc_, static_cast<Node*>(it.getNotConstBase()), 1);

        --size_;

        return ans;
    }

    void push_back() {
        auto it = end();
        insert(it);
    }

    void push_back(const T& value) {
        auto it = end();
        insert(it, value);
    }

    void push_front() {
        insert(begin());
    }

    void push_front(const T& value) {
        insert(begin(), value);
    }

    void pop_back() {
        erase(std::prev(end()));
    }

    void pop_front() {
        erase(begin());
    }

    NodeAlloc get_allocator() const {
        return alloc_;
    }

    bool empty() const {
        return size_ == 0;
    }

    size_t size() const {
        return size_;
    }

    ~List() {
        while (size_ > 0) {
            pop_back();
        }
    }
};