#include "DtlHeader.hpp"
#include <cstddef>
#include <iostream>
#include <utility>

#ifndef DTL_INCLUDE_STATICSTRING_HPP
#define DTL_INCLUDE_STATICSTRING_HPP

DTL_NAMESPACE_BEGIN

template <std::size_t Index, std::size_t Length>
constexpr char GetChar(const char (&s)[Length]) noexcept
{
    return s[Index < Length ? Index : Length - 1];
}

template <char... Chars>
struct static_string
{
    constexpr static const std::size_t size = sizeof...(Chars) + 1;
    constexpr static const char data[size] = {Chars..., '\0'};
    friend std::ostream& operator<<(std::ostream& os, const static_string& rhs)
    {
        os << rhs.data;
        return os;
    }
};

template <typename String, typename T>
struct make_static_string;
template <typename String, typename T, T... Indexs>
struct make_static_string<String, std::integer_sequence<T, Indexs...>>
{
    using type = static_string<(String{}.data[Indexs])...>;
};

template <char... CharLHS, char... CharRHS>
inline static_string<CharLHS..., CharRHS...> operator+(static_string<CharLHS...>, static_string<CharRHS...>)
{
    return {};
}

template <char... Chars>
constexpr inline bool operator==(static_string<Chars...>, static_string<Chars...>)
{
    return true;
}
template <char... CharLHS, char... CharRHS>
inline bool operator==(static_string<CharLHS...>, static_string<CharRHS...>)
{
    return false;
}

template <char... Chars>
auto AutoEndString(static_string<Chars...>) -> decltype(MakeFixString(static_string<Chars>{}...));
template <char... Previous, char Current, char... After>
auto MakeFixString(static_string<Previous...>, static_string<Current>, static_string<After>...)
    -> decltype(MakeFixString(static_string<Previous..., Current>{}, static_string<After>{}...));
template <char... Previous, char... After>
auto MakeFixString(static_string<Previous...>, static_string<'\0'>, static_string<After>...)
    -> static_string<Previous...>;

#define DTL_MAKE_CHAR_INDEX(n, str) DTL::GetChar<0x##n##0>(str), DTL::GetChar<0x##n##1>(str), \
                                      DTL::GetChar<0x##n##2>(str), DTL::GetChar<0x##n##3>(str), \
                                      DTL::GetChar<0x##n##4>(str), DTL::GetChar<0x##n##5>(str), \
                                      DTL::GetChar<0x##n##6>(str), DTL::GetChar<0x##n##7>(str), \
                                      DTL::GetChar<0x##n##8>(str), DTL::GetChar<0x##n##9>(str), \
                                      DTL::GetChar<0x##n##a>(str), DTL::GetChar<0x##n##b>(str), \
                                      DTL::GetChar<0x##n##c>(str), DTL::GetChar<0x##n##d>(str), \
                                      DTL::GetChar<0x##n##e>(str), DTL::GetChar<0x##n##f>(str)
#define DTL_MAKE_CHAR_INDEX_64_LOW(str) DTL_MAKE_CHAR_INDEX(0, str), DTL_MAKE_CHAR_INDEX(1, str), \
                                          DTL_MAKE_CHAR_INDEX(2, str), DTL_MAKE_CHAR_INDEX(3, str)
#define DTL_MAKE_CHAR_INDEX_64_HEIGHT(str) DTL_MAKE_CHAR_INDEX(4, str), DTL_MAKE_CHAR_INDEX(5, str), \
                                             DTL_MAKE_CHAR_INDEX(6, str), DTL_MAKE_CHAR_INDEX(7, str)
#define DTL_MAKE_CHAR_INDEX_SEQUENCE(str) DTL_MAKE_CHAR_INDEX_64_LOW(str), DTL_MAKE_CHAR_INDEX_64_HEIGHT(str)

#define DTL_MAKE_STATIC_STRING_TYPE(str) decltype(DTL::AutoEndString(DTL::static_string<DTL_MAKE_CHAR_INDEX_SEQUENCE(str)>{}))
#define DTL_MAKE_STATIC_STRING(str) \
    DTL_MAKE_STATIC_STRING_TYPE(str) {}

DTL_NAMESPACE_END
#endif
