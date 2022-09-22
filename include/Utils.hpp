#include "DtlHeader.hpp"
#include <cstddef>
#include <memory>
#include <utility>

#ifndef DTL_INCLUDE_UTILS_HPP
#define DTL_INCLUDE_UTILS_HPP

DTL_NAMESPACE_BEGIN

/**
 * @brief 调用指针指向的对象的对应类型的析构函数
 *
 * @tparam T 传入的移除了指针后的类型
 * @param p_memory 指向要执行析构函数的对象的指针
 */
template <class T>
void DestroyAt(T* p_memory)
{
    p_memory->~T();
}

template <class T, class... Args>
void EmplaceAt(T* p_memory, Args&&... args)
{
    ::new (p_memory) T(std::forward<Args>(args)...);
}

template <class T>
void DestroyRef(T& ref_memory)
{
    auto p_memory = std::addressof(ref_memory);
    DTL::DestroyAt(p_memory);
}

template <class T>
void EmplaceRef(T& ref_memory)
{
    auto p_memory = std::addressof(ref_memory);
    DTL::EmplaceAt(p_memory);
}

/**
 * @brief 编译期获取数组长度
 *
 * @tparam T 数组元素类型
 * @tparam N 编译期推断的数组长度
 * @return constexpr std::size_t 编译期推断的数组长度
 */
template <typename T, std::size_t N>
constexpr std::size_t GetArrayLength(const T (&)[N]) noexcept
{
    return N;
}

template <class T, T FAILED_VALUE>
struct ReturnValueVerifier
{
private:
    bool value_{false};

public:
    ReturnValueVerifier(T value)
    {
        if (value != FAILED_VALUE)
        {
            value_ = true;
        }
    }
    template <class Cleaner>
    ReturnValueVerifier(T value, Cleaner&& cleaner)
    {
        if (value != FAILED_VALUE)
        {
            value_ = true;
        }
        else
        {
            cleaner();
        }
    }

    DTL_EXPLICIT_FALSE operator bool() const noexcept
    {
        return value_;
    }

    static bool Verify(T value) noexcept
    {
        if (value != FAILED_VALUE)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    template <class Cleaner>
    static bool Verify(T value, Cleaner&& cleaner) noexcept(noexcept(cleaner))
    {
        if (value != FAILED_VALUE)
        {
            return true;
        }
        else
        {
            cleaner();
            return false;
        }
    }
};

template <typename T>
class ReversionWrapper
{
public:
    T& raw_data;
};

template <typename T>
auto begin(ReversionWrapper<T> rw) noexcept(noexcept(rw.raw_data.rbegin()))
    -> decltype(rw.raw_data.rbegin())
{
    return rw.raw_data.rbegin();
}

template <typename T>
auto end(ReversionWrapper<T> rw) noexcept(noexcept(rw.raw_data.rend()))
    -> decltype(rw.raw_data.rend())
{
    return rw.raw_data.rend();
}

template <typename T>
ReversionWrapper<T> reverse(T&& raw_data)
{
    return {raw_data};
}

DTL_NAMESPACE_END
#endif
