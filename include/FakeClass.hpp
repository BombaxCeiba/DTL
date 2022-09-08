#include "DtlHeader.hpp"
#include "Utils.hpp"

#ifndef DTL_INCLUDE_FAKECLASS_HPP
#define DTL_INCLUDE_FAKECLASS_HPP

DTL_NAMESPACE_BEGIN

/**
 * @brief 析构StaticVariableWrapper包装对象前默认执行的函数，实际上无操作
 *
 * @tparam T
 */
template <class T>
class DefaultFakeClassDtor
{
public:
    void operator()(T*){};
};
/**
 * @brief 设计上用于静态变量包装类，用于自定义变量默认初始化后行为和析构前行为
 *
 * @tparam T 要被包装的类型
 * @tparam DTOR 自定义执行析构函数前的行为
 */
template <class T, class DTOR = DefaultFakeClassDtor<T>>
class FakeClass : private DTOR
{
private:
    T m_content;

public:
    /**
     * @brief 构造一个StaticVariableWrapper
     *
     * @tparam CTOR 自定义变量默认初始化后的函数类型
     * @param ctor 自定义变量默认初始化后的行为，传入变量的指针作为参数
     * @param dtor 自定义变量执行析构函数前的行为，传入变量的指针作为参数
     */
    template <class CTOR>
    FakeClass(CTOR ctor, DTOR dtor = {})
        : DTOR{dtor}
    {
        ctor(std::addressof(m_content));
    }
    ~FakeClass()
    {
        (*static_cast<DTOR*>(this))(std::addressof(m_content));
    }
    T& Get() noexcept
    {
        return m_content;
    }
    const T& Get() const noexcept
    {
        return m_content;
    }
};
/**
 * @brief 生成静态变量包装类的函数
 *
 * @tparam T 要被包装的类型
 * @tparam CTOR 自定义变量默认初始化后的函数类型
 * @tparam DTOR 自定义变量执行析构函数前的函数类型
 * @param ctor 自定义变量默认初始化后的行为，传入变量的指针作为参数
 * @param dtor 自定义变量执行析构函数前的行为，传入变量的指针作为参数
 * @return CStaticVariableWrapper<T, DTOR> 包装后的变量，已经初始化
 */
template <class T, class CTOR, class DTOR = DefaultFakeClassDtor<T>>
auto MakeFakeClass(CTOR ctor, DTOR dtor = {})
    -> FakeClass<T, DTOR>
{
    return {ctor, dtor};
}

DTL_NAMESPACE_END
#endif
