//
// Created by chj on 2020/6/28.
//

#include <gtest/gtest.h>
#include <smart_pointer.h>

class TestObject {
 public:
  explicit TestObject(bool* flag_ptr) : flag_ptr_(flag_ptr) {
    *flag_ptr = true;
  }
  ~TestObject() { *flag_ptr_ = false; }

 private:
  bool* flag_ptr_{nullptr};
};

TEST(SharedPtr, DeleteDx) {
  bool alive = false;
  // 构造对象
  auto object_ptr = new TestObject(&alive);
  ASSERT_TRUE(alive);

  // 构造智能指针
  sp::SharedPtr<TestObject> shared1(object_ptr);
  ASSERT_TRUE(shared1.get() == object_ptr);
  ASSERT_TRUE(shared1.block()->object_use_count() == 1);

  // 拷贝
  decltype(shared1) shared2 = shared1;
  ASSERT_TRUE(alive);
  ASSERT_TRUE(shared1.block()->object_use_count() == 2);
  ASSERT_TRUE(shared2.block()->object_use_count() == 2);

  // 移动构造函数
  decltype(shared1) shared3 = std::move(shared2);
  ASSERT_TRUE(alive);
  ASSERT_TRUE(shared1.block()->object_use_count() == 2);
  ASSERT_TRUE(shared2.get() == nullptr);
  ASSERT_TRUE(shared3.get() == object_ptr);
  ASSERT_TRUE(shared3.block()->object_use_count() == 2);

  // 赋值函数
  decltype(shared1) shared4;
  shared4 = shared3;
  ASSERT_TRUE(shared1.block()->object_use_count() == 3);
  ASSERT_TRUE(shared4.get() == object_ptr);

  // 移动赋值
  shared4 = std::move(shared3);
  ASSERT_TRUE(shared1.block()->object_use_count() == 2);

  // 析构
  shared1.reset();
  shared2.reset();
  shared3.reset();
  shared4.reset();
  ASSERT_TRUE(!alive);
}

TEST(SharedPtr, UserDefinedDx) {
  bool alive = false;
  // 构造对象
  auto object_ptr = new TestObject(&alive);
  ASSERT_TRUE(alive);
  auto dx = [](TestObject* object_ptr) { delete object_ptr; };

  // 构造智能指针
  sp::SharedPtr<TestObject, decltype(dx)> shared1(object_ptr, dx);
  ASSERT_TRUE(shared1.get() == object_ptr);
  ASSERT_TRUE(shared1.block()->object_use_count() == 1);

  // 析构
  shared1.reset();
  ASSERT_TRUE(!alive);
}

TEST(SharedPtr, RAII_DeleteDx) {
  bool alive = false;
  {
    auto object_ptr = new TestObject(&alive);
    ASSERT_TRUE(alive);

    sp::SharedPtr<TestObject> shared1(object_ptr);
    ASSERT_TRUE(shared1.get() == object_ptr);
  }

  ASSERT_TRUE(!alive);
}

TEST(Shared, RAII_UserDefinedDx) {
  bool alive = false;
  {
    auto object_ptr = new TestObject(&alive);
    auto dx = [](TestObject* object_ptr) { delete object_ptr; };
    ASSERT_TRUE(alive);

    sp::SharedPtr<TestObject, decltype(dx)> shared1(object_ptr, dx);
    ASSERT_TRUE(shared1.get() == object_ptr);
    ASSERT_TRUE(shared1.block()->object_use_count() == 1);

    sp::WeakPtr<TestObject, decltype(dx)> weak1(shared1);

    auto shared2 = weak1.lock();
    ASSERT_TRUE(shared2.get() == object_ptr);
    ASSERT_TRUE(shared2.block()->object_use_count() == 2);
  }

  ASSERT_TRUE(!alive);
}

TEST(UniquePtr, DeleteDx) {
  // 测试 UniquePtr 的 Dx

  bool object_alive = false;
  TestObject* object_ptr = new TestObject(&object_alive);

  // 构造对象
  sp::UniquePtr<TestObject> uniq1(object_ptr);
  ASSERT_TRUE(uniq1.get() == object_ptr);
  ASSERT_TRUE(object_alive);

  // 测试移动构造函数
  sp::UniquePtr<TestObject> uniq2 = std::move(uniq1);
  ASSERT_TRUE(uniq1.get() == nullptr);
  ASSERT_TRUE(uniq2.get() == object_ptr);
  ASSERT_TRUE(object_alive);

  // 释放对象
  uniq2.reset();
  ASSERT_TRUE(!object_alive);
  ASSERT_TRUE(uniq2.get() == nullptr);
}

// 用户定义的 Dx
TEST(UniquePtr, UserDefinedDx) {
  bool alive = false;
  const auto dx = [](TestObject* object_ptr) { delete object_ptr; };
  TestObject* object_ptr = new TestObject(&alive);

  // 构造 UniquePtr
  sp::UniquePtr<TestObject, decltype(dx)> uniq1(object_ptr, dx);
  ASSERT_TRUE(alive);
  ASSERT_TRUE(uniq1.get() == object_ptr);

  // 移动构造函数
  decltype(uniq1) uniq2 = std::move(uniq1);
  ASSERT_TRUE(uniq1.get() == nullptr);
  ASSERT_TRUE(uniq2.get() == object_ptr);
  ASSERT_TRUE(alive);

  // 释放
  uniq2.reset();
  ASSERT_TRUE(uniq2.get() == nullptr);
  ASSERT_TRUE(!alive);
}

TEST(UniquePtr, RAII_DeleteDx) {
  bool alive = false;
  {
    // 构造对象
    sp::UniquePtr<TestObject> uniq1(new TestObject(&alive));
    ASSERT_TRUE(alive);

    // 移动
    decltype(uniq1) uniq2 = std::move(uniq1);
    ASSERT_TRUE(uniq1.get() == nullptr);
    ASSERT_TRUE(uniq2.get() != nullptr);

    // 离开作用域前检查一次
    ASSERT_TRUE(alive);
  }
  // 检查对象是否被析构
  ASSERT_TRUE(!alive);
}

TEST(UniquePtr, RAII_UserDefinedDx) {
  bool alive = false;
  {
    // 自定义的对象销毁函数
    auto dx = [](TestObject* object) { delete object; };

    // 构造对象
    sp::UniquePtr<TestObject, decltype(dx)> uniq1(new TestObject(&alive), dx);
    ASSERT_TRUE(alive);

    // 移动
    decltype(uniq1) uniq2 = std::move(uniq1);
    ASSERT_TRUE(uniq1.get() == nullptr);
    ASSERT_TRUE(uniq2.get() != nullptr);

    // 离开作用域前检查一次
    ASSERT_TRUE(alive);
  }
  // 检查对象是否被析构
  ASSERT_TRUE(!alive);
}

int main() {
  ::testing::InitGoogleTest();
  return ::RUN_ALL_TESTS();
}
