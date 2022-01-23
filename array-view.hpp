///@file   array_view.hpp
///@author Chris Newman
///@brief  Defines a class to allow for non-owning view of an array.

#pragma once
#include <stdexcept>

template<typename T>
class TArrayView
{
public:
  TArrayView(T* ptr, size_t cnt) : data_{ptr}, count_{cnt} {}
  ~TArrayView() {}

  T* begin() const noexcept { return data_; }
  const T* cbegin() const noexcept { return data_; }
  T* end() const noexcept { return data_ + count_; }
  const T* cend() const noexcept { return data_ + count_; }

  size_t size() { return count_; }
  T* data() { return data_; }

  T& operator[](size_t index) { return *(data_ + index); }
  T& at(size_t index) 
  { 
    if (index < count_ && index >= 0) 
      return *(data_ + index); 
    else 
      throw std::out_of_range{"Beyond view bounds!"};
  }
private:
  T* data_ = nullptr;
  size_t count_ = 0;
};
