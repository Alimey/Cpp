#include <iostream>
#include <type_traits>
#include <utility>

template <typename T>
class EnableSharedFromThis;

template <typename T>
class WeakPtr;

struct BaseControlBlock {
    size_t shared_count;
    size_t weak_count;

    virtual ~BaseControlBlock() = default;
    virtual void DestroyObject() = 0;
    virtual void DeallocateAll() = 0;

    BaseControlBlock(size_t sh_cnt, size_t w_cnt)
        : shared_count(sh_cnt),
          weak_count(w_cnt) {
    }
};

template <typename T>
class SharedPtr {
    template <typename U, typename Deleter = std::default_delete<U>,
              typename Alloc = std::allocator<U>>
    struct ControlBlockConstructor : BaseControlBlock {
        [[no_unique_address]] Deleter del;
        [[no_unique_address]] Alloc alloc;
        T* obj_ptr;

        ControlBlockConstructor(size_t sh_cnt, size_t w_cnt, Deleter del, Alloc alloc, T* ptr)
            : BaseControlBlock{sh_cnt, w_cnt},
              del(del),
              alloc(alloc),
              obj_ptr(ptr) {
        }

        void DestroyObject() override {
            del(obj_ptr);
        }

        void DeallocateAll() override {
            using AllocTraits = std::allocator_traits<Alloc>;
            using BlockAlloc = AllocTraits::template rebind_alloc<ControlBlockConstructor>;
            using BlockAllocTraits = std::allocator_traits<BlockAlloc>;

            auto block_alloc = BlockAlloc(alloc);
            BlockAllocTraits::deallocate(block_alloc, this, 1);
        }
    };

    template <typename U, typename Alloc = std::allocator<U>>
    struct ControlBlockMakeShared : BaseControlBlock {
        Alloc alloc;
        U value;

        template <typename... Args>
        ControlBlockMakeShared(const Alloc& alloc, Args&&... args)
            : BaseControlBlock{1, 0},
              alloc(alloc),
              value(std::forward<Args>(args)...) {
        }

        void DestroyObject() override {
            std::allocator_traits<Alloc>::destroy(alloc, &value);
        }

        void DeallocateAll() override {
            using AllocTraits = std::allocator_traits<Alloc>;
            using BlockAlloc = AllocTraits::template rebind_alloc<ControlBlockMakeShared>;
            using BlockAllocTraits = std::allocator_traits<BlockAlloc>;

            auto block_alloc = BlockAlloc(alloc);

            BlockAllocTraits::deallocate(block_alloc, this, 1);
        }
    };

    T* ptr_;
    BaseControlBlock* cb_ptr_;

    struct field_filler_copy_constructor_tag {};
    struct field_filler_move_constructor_tag {};

    template <typename U, typename Alloc = std::allocator<U>>
    void check_enable_shared_from_this() const {
        if constexpr (std::is_base_of_v<EnableSharedFromThis<T>, T>) {
            if (ptr_ == nullptr) {
                static_cast<ControlBlockMakeShared<U, Alloc>*>(cb_ptr_)->value.self_ptr_ = *this;
                return;
            }
            ptr_->self_ptr_ = *this;
        }
    }

    template <typename U>
    SharedPtr(field_filler_copy_constructor_tag, const SharedPtr<U>& value)
        : ptr_(value.ptr_),
          cb_ptr_(value.cb_ptr_) {
        if (cb_ptr_ == nullptr) {
            return;
        }
        ++cb_ptr_->shared_count;
        check_enable_shared_from_this<U>();
    }

    template <typename U>
    SharedPtr(field_filler_move_constructor_tag, SharedPtr<U>&& another)
        : ptr_(std::exchange(another.ptr_, nullptr)),
          cb_ptr_(std::exchange(another.cb_ptr_, nullptr)) {
        check_enable_shared_from_this<U>();
    }

    // приватный конструктор для make shared
    template <typename U, typename Alloc>
    SharedPtr(ControlBlockMakeShared<U, Alloc>* cb_ptr)
        : ptr_(&cb_ptr->value),
          cb_ptr_(cb_ptr) {
        check_enable_shared_from_this<U, Alloc>();
    }

    // приватный конструктор для weak_ptr
    SharedPtr(BaseControlBlock* cb_ptr, T* ptr)
        : ptr_(ptr),
          cb_ptr_(cb_ptr) {
        if (cb_ptr_ == nullptr) {
            return;
        }
        ++cb_ptr_->shared_count;
        check_enable_shared_from_this<T>();
    }

public:
    /// КОНСТРУКТОРЫ ///

    /* по умолчанию */

    SharedPtr()
        : ptr_(nullptr),
          cb_ptr_(nullptr) {
    }

    /* от С-style указателя */

    template <typename U, typename Deleter = std::default_delete<U>,
              typename Alloc = std::allocator<U>>
        requires std::is_same_v<T, U> || std::is_convertible_v<U, T>
    SharedPtr(U* ptr, Deleter del = Deleter(), Alloc alloc = Alloc())
        : ptr_(static_cast<T*>(ptr)) {
        using AllocTraits = std::allocator_traits<Alloc>;
        using BlockAlloc =
            AllocTraits::template rebind_alloc<ControlBlockConstructor<U, Deleter, Alloc>>;
        using BlockAllocTraits = std::allocator_traits<BlockAlloc>;

        BlockAlloc block_alloc = BlockAlloc(alloc);
        auto block_ptr = BlockAllocTraits::allocate(block_alloc, 1);

        new (block_ptr) typename SharedPtr<U>::template ControlBlockConstructor<U, Deleter, Alloc>(
            1, 0, del, alloc, ptr_);
        cb_ptr_ = block_ptr;

        check_enable_shared_from_this<U>();
    }

    /* копирования */

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    SharedPtr(const SharedPtr<U>& another)
        : SharedPtr(field_filler_copy_constructor_tag(), another) {
    }

    SharedPtr(const SharedPtr& another)
        : SharedPtr(field_filler_copy_constructor_tag(), another) {
    }

    /* мув */

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    SharedPtr(SharedPtr<U>&& another)
        : SharedPtr(field_filler_move_constructor_tag(), std::move(another)) {
    }

    SharedPtr(SharedPtr&& another)
        : SharedPtr(field_filler_move_constructor_tag(), std::move(another)) {
    }

    /* aliasing */

    template <typename U>
    SharedPtr(const SharedPtr<U>& another, T* ptr_to_hold)
        : ptr_(ptr_to_hold),
          cb_ptr_(another.cb_ptr_) {
        ++cb_ptr_->shared_count;
        check_enable_shared_from_this<U>();
    }

    template <typename U>
    SharedPtr(SharedPtr<U>&& another, T* ptr_to_hold)
        : ptr_(ptr_to_hold),
          cb_ptr_(std::exchange(another.cb_ptr_, nullptr)) {
        ++cb_ptr_->shared_count;
        check_enable_shared_from_this<U>();
    }

    /* от weak_ptr */

    template <typename U>
    SharedPtr(const WeakPtr<U>& another) {
        if (another.expired()) {
            throw std::bad_weak_ptr();
        }

        ptr_ = another.ptr_;
        cb_ptr_ = another.cb_ptr_;

        ++cb_ptr_->shared_count;
        check_enable_shared_from_this<U>();
    }

    /// ОПЕРАТОРЫ ПРИСВАИВАНИЯ ///

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    SharedPtr& operator=(const SharedPtr<U>& another) {
        SharedPtr copy = another;
        swap(copy);
        return *this;
    }

    SharedPtr& operator=(const SharedPtr& another) {
        if (this == &another) {
            return *this;
        }
        SharedPtr copy = another;
        swap(copy);
        return *this;
    }

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    SharedPtr& operator=(SharedPtr<U>&& another) {
        SharedPtr copy = std::move(another);
        swap(copy);
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& another) {
        SharedPtr copy = std::move(another);
        swap(copy);
        return *this;
    }

    /// ДЕСТРУКТОР ///
    ~SharedPtr() {
        if (cb_ptr_ == nullptr) {
            return;
        }

        --cb_ptr_->shared_count;

        if (cb_ptr_->shared_count > 0) {
            return;
        }

        ++cb_ptr_->weak_count;
        cb_ptr_->DestroyObject();
        --cb_ptr_->weak_count;

        if (cb_ptr_->weak_count == 0) {
            cb_ptr_->DeallocateAll();
        }
    }

    /// USE COUNT ///
    size_t use_count() const {
        if (cb_ptr_ == nullptr) {
            return 0;
        }
        return cb_ptr_->shared_count;
    }

    /// RESET AND SWAP ///
    template <typename U, typename Deleter = std::default_delete<U>,
              typename Alloc = std::allocator<U>>
        requires std::is_same_v<T, U> || std::is_convertible_v<U, T>
    void reset(U* another_ptr, Deleter del = Deleter(), Alloc alloc = Alloc()) {
        SharedPtr another_shared(another_ptr, del, alloc);
        swap(another_shared);
    }

    void reset() {
        SharedPtr another;
        swap(another);
    }

    void swap(SharedPtr& another) {
        std::swap(ptr_, another.ptr_);
        std::swap(cb_ptr_, another.cb_ptr_);
    }

    /// GET ///

    T* get() const {
        return ptr_;
    }

    /// СТРЕЛОЧКА И ЗВЕЗДОЧКА ///

    T& operator*() const {
        return *ptr_;
    }

    T* operator->() const {
        return ptr_;
    }

    /// FRIENDS ///

    template <typename U>
    friend class WeakPtr;

    template <typename U, typename... Args>
    friend SharedPtr<U> makeShared(Args&&... args);

    template <typename U>
    friend class SharedPtr;

    template <typename U, typename Alloc, typename... Args>
    friend SharedPtr<U> allocateShared(const Alloc& alloc, Args&&... args);
};

/// MAKE SHARED ///
template <typename U, typename... Args>
SharedPtr<U> makeShared(Args&&... args) {
    auto block_ptr = new SharedPtr<U>::template ControlBlockMakeShared<U, std::allocator<U>>(
        std::allocator<U>(), std::forward<Args>(args)...);
    // вызываем приватный конструктор от указателя на control block
    SharedPtr<U> shared_ptr(block_ptr);
    return shared_ptr;
}

/// ALLOCATE SHARED ///
template <typename U, typename Alloc, typename... Args>
SharedPtr<U> allocateShared(const Alloc& alloc, Args&&... args) {
    using AllocTraits = std::allocator_traits<Alloc>;
    using BlockAlloc = typename AllocTraits::template rebind_alloc<
        typename SharedPtr<U>::template ControlBlockMakeShared<U, Alloc>>;
    using BlockAllocTraits = std::allocator_traits<BlockAlloc>;

    auto block_alloc = BlockAlloc(alloc);
    auto block_ptr = BlockAllocTraits::allocate(block_alloc, 1);
    BlockAllocTraits::construct(block_alloc, block_ptr, alloc, std::forward<Args>(args)...);
    // вызываем приватный конструктор от указателя на control block
    SharedPtr<U> shared_ptr(block_ptr);
    return shared_ptr;
}

template <typename T>
class WeakPtr {
    T* ptr_;
    BaseControlBlock* cb_ptr_;

public:
    /// КОНСТРУКТОРЫ ///

    /* по умолчанию */
    WeakPtr()
        : ptr_(nullptr),
          cb_ptr_(nullptr) {
    }

    /* от shared_ptr */

    template <typename U>
        requires std::is_same_v<T, U> || std::is_convertible_v<U, T>
    WeakPtr(const SharedPtr<U>& sh_ptr)
        : ptr_(sh_ptr.ptr_),
          cb_ptr_(sh_ptr.cb_ptr_) {
        ++sh_ptr.cb_ptr_->weak_count;
    }

    /* копирования */

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    WeakPtr(const WeakPtr<U>& another)
        : ptr_(another.ptr_),
          cb_ptr_(another.cb_ptr_) {
        ++cb_ptr_->weak_count;
    }

    WeakPtr(const WeakPtr& another)
        : ptr_(another.ptr_),
          cb_ptr_(another.cb_ptr_) {
        ++cb_ptr_->weak_count;
    }

    /* мув */

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    WeakPtr(WeakPtr<U>&& another)
        : ptr_(std::exchange(another.ptr_, nullptr)),
          cb_ptr_(std::exchange(another.cb_ptr_, nullptr)) {
    }

    WeakPtr(WeakPtr&& another)
        : ptr_(std::exchange(another.ptr_, nullptr)),
          cb_ptr_(std::exchange(another.cb_ptr_, nullptr)) {
    }

    /// ОПЕРАТОРЫ ПРИСВАИВАНИЯ ///

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    WeakPtr& operator=(const WeakPtr<U>& another) {
        auto copy = another;
        swap(copy);
        return *this;
    }

    WeakPtr& operator=(const WeakPtr& another) {
        if (this == &another) {
            return *this;
        }
        auto copy = another;
        swap(copy);
        return *this;
    }

    template <typename U>
        requires std::is_convertible_v<U*, T*>
    WeakPtr& operator=(WeakPtr<U>&& another) {
        auto copy = std::move(another);
        swap(copy);
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& another) {
        auto copy = std::move(another);
        swap(copy);
        return *this;
    }

    /// ДЕСТРУКТОР ///
    ~WeakPtr() {
        if (cb_ptr_ == nullptr) {
            return;
        }

        --cb_ptr_->weak_count;
        if (cb_ptr_->weak_count == 0 && cb_ptr_->shared_count == 0) {
            cb_ptr_->DeallocateAll();
        }
    }

    /// EXPIRED ///
    bool expired() const {
        return use_count() == 0;
    }

    /// LOCK ///
    SharedPtr<T> lock() const {
        return SharedPtr<T>(cb_ptr_, ptr_);
    }

    /// USE COUNT AND SWAP ///

    size_t use_count() const {
        if (cb_ptr_ == nullptr) {
            return 0;
        }
        return cb_ptr_->shared_count;
    }

    void swap(WeakPtr& another) {
        std::swap(ptr_, another.ptr_);
        std::swap(cb_ptr_, another.cb_ptr_);
    }

    /// FRIENDS ///
    template <typename U>
    friend class WeakPtr;

    template <typename U>
    friend class EnableSharedFromThis;

    template <typename U>
    friend class SharedPtr;
};

template <typename T>
class EnableSharedFromThis {
    template <typename U>
    friend class SharedPtr;

    WeakPtr<T> self_ptr_;

public:
    SharedPtr<T> shared_from_this() const {
        if (self_ptr_.cb_ptr_ == nullptr) {
            throw std::bad_weak_ptr();
        }
        return self_ptr_;
    }
};
