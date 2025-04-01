#ifndef SmartPointer_H
#define SmartPointer_H

#include <functional>
#include <type_traits>
template <typename T>
class MySharedPtr {
  public:
    MySharedPtr();
    explicit MySharedPtr(T*);
    MySharedPtr(T*, std::function<void(T*)>);
    ~MySharedPtr();
    MySharedPtr(const MySharedPtr& other);
    MySharedPtr& operator=(const MySharedPtr& other);
    T& operator*(); 
    T* operator->() const;
    T* get() const;
    bool isUnique() const;

    explicit operator bool() const;
    void reset();
    void reset(T*);
    void reset(T*,std::function<void(T*)>);  
    T* release(); 
  private:
    static std::function<void(T*)> default_deleter;
  private:
   T* d_data {nullptr};
   std::atomic<std::size_t>* refCntPtr {nullptr};
   std::function<void(T*)> d_deleter {default_deleter};
};

template <typename T>
std::function<void(T*)> MySharedPtr<T>::default_deleter = [](T* data){ delete data; data = nullptr; }; 

template <typename T, typename... Args>
MySharedPtr<T> make_my_shared(Args&&... args)
{
  MySharedPtr<T> tmp(new T(std::forward<Args>(args)...));
  return tmp;
}

template <typename T>
MySharedPtr<T>::MySharedPtr() {}

template <typename T>
MySharedPtr<T>::MySharedPtr(T* p) : d_data(p),refCntPtr(new std::atomic<std::size_t>(1)) {}

template <typename T>
MySharedPtr<T>::MySharedPtr(T* p, std::function<void(T*)> func) : d_data(p),refCntPtr(new std::atomic<std::size_t>(1)) {
  d_deleter = func;
}

template <typename T>
MySharedPtr<T>::MySharedPtr(const MySharedPtr& other) : d_data(other.d_data),refCntPtr(other.refCntPtr),d_deleter(other.d_deleter)
{
   refCntPtr->fetch_add(1);
}

template <typename T>
MySharedPtr<T>& MySharedPtr<T>::operator=(const MySharedPtr& other)
{
  if(this!=&other)
  {
    if(refCntPtr->load()==1)
    {
       d_deleter(d_data);
       delete refCntPtr;
    }
    d_data = other.d_data;
    refCntPtr = other.refCntPtr;
    refCntPtr->fetch_add(1);
    d_deleter = other.d_deleter;
  }
  return *this;
}

template <typename T>
T& MySharedPtr<T>::operator*() 
{
  return *d_data;
}

template <typename T>
T* MySharedPtr<T>::operator->() const
{
  return this->d_data;
}

template <typename T>
MySharedPtr<T>::~MySharedPtr()
{
  if(refCntPtr!=nullptr)
  {
   refCntPtr->fetch_sub(1);
    if(refCntPtr->load()==0)
   {
    d_deleter(d_data);
    d_data = nullptr;
    delete refCntPtr;
    refCntPtr = nullptr;
   } 
  }
}

template <typename T>
bool MySharedPtr<T>::isUnique() const
{
  if(refCntPtr!=nullptr && refCntPtr->load()>1)
  {
   return false;
  }
  return true;
}

template <typename T>
void MySharedPtr<T>::reset()
{
  if(refCntPtr!=nullptr)
  {
  refCntPtr->fetch_sub(1);
  if(refCntPtr->load()==0)
  {
    d_deleter(d_data);
    delete refCntPtr;
  }
  }
  refCntPtr = nullptr;
  d_data = nullptr;
  d_deleter = default_deleter;
}

template <typename T>
void MySharedPtr<T>::reset(T* t)
{
  if(refCntPtr)
  {
  refCntPtr->fetch_sub(1);
  if(refCntPtr->load()==0)
   {
    d_deleter(d_data);
    delete refCntPtr;
   }
  }
  d_data = t;
  d_deleter = default_deleter;
  refCntPtr = new std::atomic<std::size_t>(1);
}

template <typename T>
void MySharedPtr<T>::reset(T* t, std::function<void(T*)> deleter)
{
  reset(t);
  d_deleter = deleter;
}

template <typename T>
T* MySharedPtr<T>::release()
{
  if(refCntPtr!=nullptr)
  {
  refCntPtr->fetch_sub(1);
  if(refCntPtr->load()==0)
   {
    delete refCntPtr;
    refCntPtr=nullptr;
   }
  }
  auto ret = d_data;
  d_data = nullptr;
  return ret; 
}

template <typename T>
T* MySharedPtr<T>::get() const
{
  return d_data;
}

template <typename T>
MySharedPtr<T>::operator bool() const
{
  return d_data!=nullptr;
}

template <typename T>
class MyUniquePtr {
 public:
  MyUniquePtr() {} 
  explicit MyUniquePtr(T*);
  MyUniquePtr(T*,std::function<void(T*)>); 
  MyUniquePtr(const MyUniquePtr&) = delete;
  MyUniquePtr(MyUniquePtr&& other) noexcept;
  MyUniquePtr(std::nullptr_t) noexcept;

  template <typename U, typename std::enable_if<std::is_base_of<T,U>::value,void>::type* = nullptr>
  MyUniquePtr(MyUniquePtr<U>&& other) noexcept
  {
     MyUniquePtr<T> tmp(other.release());
     swap(tmp);    
  }
  MyUniquePtr& operator=(const MyUniquePtr& other) = delete;
  MyUniquePtr& operator=(MyUniquePtr&& other);
  MyUniquePtr& operator=(std::nullptr_t ptr);

  template <typename U, typename std::enable_if<std::is_base_of<T,U>::value,void>::type* = nullptr>
  MyUniquePtr& operator=(MyUniquePtr<U>&& other) noexcept
  {
    MyUniquePtr<T> tmp(other.release());
    swap(tmp);
    return *this;
  }

  ~MyUniquePtr();
  MyUniquePtr& swap(MyUniquePtr& other);
  T* get();
  T* release();
  T& operator*();
  T* operator->();
  void reset();
  void reset(T*);
  void reset(T*,std::function<void(T*)>);
  explicit operator bool();
  bool operator==(const MyUniquePtr& other) const
  {
    return d_data == other.d_data;
  }
 private:
  static std::function<void(T*)> default_deleter;
 private:
  T* d_data {nullptr};
  std::function<void(T*)> d_deleter {default_deleter};
};

template <typename T>
std::function<void(T*)> MyUniquePtr<T>::default_deleter = [](T* data){delete data;data=nullptr;}; 

template <typename T>
MyUniquePtr<T>::MyUniquePtr(T* t)
{
  d_data = t;
}

template <typename T>
MyUniquePtr<T>::MyUniquePtr(std::nullptr_t ptr) noexcept 
{
  d_data = nullptr;
}


template <typename T>
MyUniquePtr<T>::MyUniquePtr(T* data,std::function<void(T*)> deleter)
{
  d_data = data;
  d_deleter = deleter;
}

template <typename T>
MyUniquePtr<T>::MyUniquePtr(MyUniquePtr&& other) noexcept
{
  d_data = other.d_data;
  other.d_data = nullptr;
}

template <typename T>
MyUniquePtr<T>& MyUniquePtr<T>::operator=(MyUniquePtr&& other)
{
  if(&other!=this)
  {
    d_deleter(d_data);
    d_data = other.d_data;
    other.d_data = nullptr;
  }
  return *this;
}

template <typename T>
MyUniquePtr<T>& MyUniquePtr<T>::operator=(std::nullptr_t ptr)
{
  d_deleter(d_data);
  d_data = nullptr;
  return *this;
}

template <typename T>
MyUniquePtr<T>::~MyUniquePtr()
{
  d_deleter(d_data);  
}

template <typename T>
MyUniquePtr<T>& MyUniquePtr<T>::swap(MyUniquePtr& other)
{
  if(&other!=this)
  {
    using std::swap;
    swap(d_data,other.d_data);
  }
  return *this;
}

template <typename T>
T* MyUniquePtr<T>::get()
{
  return d_data;
}

template <typename T>
T* MyUniquePtr<T>::release()
{
  auto ret = d_data;
  d_data = nullptr;
  return ret;
}

template <typename T>
T& MyUniquePtr<T>::operator*()
{
  return *d_data;
}

template <typename T>
T* MyUniquePtr<T>::operator->()
{
  return d_data;
}

template <typename T>
void MyUniquePtr<T>::reset()
{
  d_deleter(d_data);
  d_data = nullptr;
}

template <typename T>
void MyUniquePtr<T>::reset(T* data)
{
  d_data = data;  
}

template <typename T>
void MyUniquePtr<T>::reset(T* data,std::function<void(T*)> deleter)
{
  reset(data);
  d_deleter = deleter;
}

template <typename T>
MyUniquePtr<T>::operator bool()
{
  return d_data!=nullptr;
}

template <typename T>
void swap(MyUniquePtr<T>& ptr1, MyUniquePtr<T>& ptr2)
{
  ptr1.swap(ptr2);
}

#endif
