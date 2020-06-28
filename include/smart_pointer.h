//
// Created by chj on 2020/6/28.
//

#ifndef SMART_POINTER_H
#define SMART_POINTER_H

#include <cstddef>
#include <utility>

namespace sp {

namespace details {

template <typename T>
class DeleteDx {
 public:
  void operator()(T* object_ptr) { delete (object_ptr); }
};

template <typename T>
struct BlockBase {
 public:
  explicit BlockBase(T* object_ptr)
      : object_ptr_(object_ptr), object_use_count_(1), block_use_count_(1) {}

  void add_object_ref() { object_use_count_++; }
  void add_block_ref() { block_use_count_++; }
  int release_block() { return --block_use_count_; }

  T* get_object() const {
    if (object_use_count_ == 0) return nullptr;

    return object_ptr_;
  }

  [[nodiscard]] int object_use_count() const { return object_use_count_; }
  [[nodiscard]] int block_use_count() const { return block_use_count_; }

 protected:
  T* object_ptr_{nullptr};
  int object_use_count_{0};
  int block_use_count_{0};
};

template <typename T, typename Dx>
struct Block;

template <typename T, typename Dx>
class Block : public BlockBase<T> {
 public:
  using Super = BlockBase<T>;

  Block() = default;
  explicit Block(T* object_ptr, Dx dx)
      : BlockBase<T>(object_ptr), dx_(std::move(dx)) {}

  // 释放对象
  void release_object() {
    Super::object_use_count_--;

    // 计数为零，销毁对象
    if (Super::object_use_count_ == 0) {
      dx_(Super::object_ptr_);
      Super::object_ptr_ = nullptr;
    }
  }

 private:
  Dx dx_;
};

// 针对 DeleteDx 做的偏特化
template <typename T>
struct Block<T, DeleteDx<T>> : public BlockBase<T> {
  using Super = BlockBase<T>;
  Block() = default;
  explicit Block(T* object_ptr) : BlockBase<T>(object_ptr) {}

  void release_object() {
    Super::object_use_count_--;

    // 计数为零，销毁对象
    if (Super::object_use_count_ == 0) {
      details::DeleteDx<T>()(Super::object_ptr_);
      Super::object_ptr_ = nullptr;
    }
  }
};

template <typename T, typename Dx, typename... Args>
Block<T, Dx>* make_block(Args&&... args) {
  return new Block<T, Dx>(std::forward<Args&&>(args)...);
}

template <typename T, typename Dx>
void destroy_block(Block<T, Dx>* block_ptr) {
  delete block_ptr;
}

// 增加对 block 的引用
template <typename T, typename Dx>
void add_block_ref(Block<T, Dx>* block_ptr) {
  if (block_ptr) {
    block_ptr->add_block_ref();
  }
}

// 释放对 block 的引用
template <typename T, typename Dx>
Block<T, Dx>* release_block(Block<T, Dx>* block_ptr) {
  if (!block_ptr) {
    return nullptr;
  }

  // 释放引用计数块
  if (block_ptr->release_block() == 0) {
    delete block_ptr;
    block_ptr = nullptr;
  }

  return block_ptr;
}

template <typename T, typename Dx>
void add_object_ref(Block<T, Dx>* block_ptr) {
  if (!block_ptr) {
    return;
  }

  block_ptr->add_object_ref();
  block_ptr->add_block_ref();
}

// 释放对象
template <typename T, typename Dx>
Block<T, Dx>* release_object(Block<T, Dx>* block_ptr) {
  if (!block_ptr) {
    return block_ptr;
  }

  block_ptr->release_object();  // 释放对象

  return release_block(block_ptr);
}

}  // namespace details

template <typename T, typename Dx = details::DeleteDx<T>>
class SharedPtr {
 public:
  using BlockT = details::Block<T, Dx>;

  SharedPtr() = default;
  // 从对象指针构造 SharedPtr
  explicit SharedPtr(T* object_ptr, Dx dx)
      : block_ptr_(details::make_block<T, Dx>(object_ptr, std::move(dx))) {}
  // TODO enable_if
  explicit SharedPtr(T* object_ptr)
      : block_ptr_(details::make_block<T, details::DeleteDx<T>>(object_ptr)) {}
  // 析构函数
  ~SharedPtr() { reset(); }
  // 用引用技术块指针构造 SharedPtr
  explicit SharedPtr(BlockT* block_ptr) { set(block_ptr); }
  // 拷贝构造函数
  SharedPtr(const SharedPtr<T, Dx>& other) { set(other.block_ptr_); }
  // 移动构造函数
  SharedPtr(SharedPtr<T, Dx>&& other) noexcept {
    block_ptr_ = other.block_ptr_;
    other.block_ptr_ = nullptr;
  }

  // 赋值操作符
  SharedPtr<T, Dx>& operator=(std::nullptr_t) {
    reset();
    return *this;
  }
  // 从另一个 SharedPtr 复制
  SharedPtr<T, Dx>& operator=(const SharedPtr<T, Dx>& rhs) {
    if (&rhs == this) {
      return *this;
    }

    set(rhs.block_ptr_);
    return *this;
  }
  // 从另一个 SharedPtr 移动
  SharedPtr<T, Dx>& operator=(SharedPtr<T, Dx>&& rhs) noexcept {
    if (&rhs == this) {
      return *this;
    }

    reset();

    block_ptr_ = rhs.block_ptr_;
    rhs.block_ptr_ = nullptr;

    return *this;
  }

  // 重置
  void reset() { set(nullptr); }
  T* get() const {
    if (!block_ptr_) return nullptr;

    return block_ptr_->get_object();
  }
  T* operator->() const { return get(); }
  BlockT* block() const { return block_ptr_; }

 private:
  BlockT* block_ptr_{nullptr};

  // 设置智能指针
  void set(BlockT* block_ptr) {
    details::add_object_ref(block_ptr);
    details::release_object(block_ptr_);
    block_ptr_ = block_ptr;
  }
};

template <typename T, typename Dx = details::DeleteDx<T>>
class WeakPtr {
 public:
  using BlockT = details::Block<T, Dx>;
  WeakPtr() = default;
  // 析构函数
  ~WeakPtr() { set(nullptr); }
  // 从引用计数块构造
  explicit WeakPtr(BlockT* block_ptr) { set(block_ptr); }
  // 从 SharedPtr 构造
  explicit WeakPtr(const SharedPtr<T, Dx>& shared_ptr) {
    set(shared_ptr.block());
  }
  // 拷贝赋值
  WeakPtr(const WeakPtr<T, Dx>& other) { set(other.block_ptr_); }
  // 移动赋值
  WeakPtr(WeakPtr<T, Dx>&& other) noexcept {
    block_ptr_ = other.block_ptr_;
    other.block_ptr_ = nullptr;
  }

  // 赋值
  WeakPtr<T, Dx>& operator=(const WeakPtr<T, Dx>& rhs) {
    if (&rhs == this) {
      return *this;
    }

    set(rhs.block_ptr_);
    return *this;
  }
  WeakPtr<T, Dx>& operator=(WeakPtr<T, Dx>&& rhs) noexcept {
    if (&rhs == this) {
      return *this;
    }

    block_ptr_ = rhs.block_ptr_;
    rhs.block_ptr_ = nullptr;

    return *this;
  }

  // 获取 SharedPtr
  SharedPtr<T, Dx> lock() const { return SharedPtr<T, Dx>(block_ptr_); }

  void set(BlockT* block_ptr) {
    details::add_block_ref(block_ptr);   //
    details::release_block(block_ptr_);  // 释放旧引用

    block_ptr_ = block_ptr;
  }

 private:
  BlockT* block_ptr_{nullptr};
};

template <typename T, typename Dx = details::DeleteDx<T>>
class UniquePtr;

// 针对 DeleteDx 的偏特化
template <typename T>
class UniquePtr<T, details::DeleteDx<T>> {
 public:
  UniquePtr() = default;
  // 从一个指针构造 UniquePtr
  explicit UniquePtr(T* object_ptr) : object_ptr_(object_ptr) {}

  // 析构函数
  ~UniquePtr() { reset(); }

  // 禁止拷贝和赋值
  UniquePtr(const UniquePtr<T>&) = delete;
  UniquePtr<T>& operator=(const UniquePtr<T>&) = delete;

  // 移动语义
  UniquePtr(UniquePtr<T>&& other) noexcept {
    object_ptr_ = other.object_ptr_;
    other.object_ptr_ = nullptr;
  }
  UniquePtr<T>& operator=(UniquePtr<T>&& rhs) noexcept {
    reset();

    object_ptr_ = rhs.object_ptr_;
    rhs.object_ptr_ = nullptr;
    return *this;
  }

  T* operator->() const { return object_ptr_; }
  T* get() const { return object_ptr_; }

  // 释放
  void reset() {
    details::DeleteDx<T>()(object_ptr_);
    object_ptr_ = nullptr;
  }

 private:
  T* object_ptr_{nullptr};
};

// 自定义的销毁函数
template <typename T, typename Dx>
class UniquePtr {
 public:
  UniquePtr() = default;
  explicit UniquePtr(T* object_ptr, Dx dx)
      : object_ptr_(object_ptr), dx_(std::move(dx)) {}

  // 析构函数
  ~UniquePtr() { reset(); }

  // 禁止拷贝构造函数
  UniquePtr(const UniquePtr<T, Dx>&) = delete;
  UniquePtr<T, Dx>& operator=(const UniquePtr<T, Dx>&) = delete;

  // 移动构造函数
  UniquePtr(UniquePtr<T, Dx>&& other) noexcept : dx_(std::move(other.dx_)) {
    object_ptr_ = other.object_ptr_;
    other.object_ptr_ = nullptr;
  }
  UniquePtr<T, Dx>& operator=(UniquePtr<T, Dx>&& rhs) noexcept {
    object_ptr_ = rhs.object_ptr_;
    rhs.object_ptr_ = nullptr;

    dx_ = std::move(rhs.dx_);

    return *this;
  }

  T* operator->() const { return object_ptr_; }
  T* get() const { return object_ptr_; }

  void reset() {
    dx_(object_ptr_);
    object_ptr_ = nullptr;
  }

 private:
  T* object_ptr_ = nullptr;
  Dx dx_;
};

// 侵入式引用计数, TODO
template <typename T>
class IntrusivePtr {};

}  // namespace sp

#endif  // SMART_POINTER_H
