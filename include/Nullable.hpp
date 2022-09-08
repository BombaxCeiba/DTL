#include "DtlHeader.hpp"
#include <functional>
#include <type_traits>
#include <stdexcept>
#include <tuple>

#ifndef DTL_INCLUDE_NULLABLE_HPP
#define DTL_INCLUDE_NULLABLE_HPP

DTL_NAMESPACE_BEGIN
/**
 * @brief 自C++ 23起 std::aligned_storage 将被弃用，应当使用std::byte作为替代
 *
 * @tparam T
 */
template <class T>
using std_aligned_storage = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

//皆不保证多线程操作安全
/**
 * @brief CNullable<T>的默认删除器，注意重载operator()的参数为T*
 *
 * @tparam T 要删除的类型
 */
template <class T>
struct NullableDefaultDeleter
{
    void operator()(T* p_this)
    {
        p_this->~T();
    }
};
/**
 * @brief 可空对象，允许对象在无默认构造函数的情况下仍然可以执行默认构造，代价是必须在使用对象之前执行Construct方法
 *
 * @tparam T 实际存储的对象
 * @tparam Deleter T的删除器
 */
template <class T, class Deleter = NullableDefaultDeleter<T>>
class Nullable
{
    using StorageType = DTL::std_aligned_storage<T>;
    template <class C, class... Args>
    static void EmplaceAt(C* p_object, Args&&... args)
    {
        ::new (p_object) C(std::forward<Args>(args)...);
    }

public:
    Nullable(Deleter deleter = {})
        : storage_{deleter} {}
    ~Nullable()
    {
        if (has_value_)
        {
            DestroySelf();
        }
    }
    Nullable(const Nullable& other)
        : has_value_{other.has_value_}, storage_{static_cast<Deleter>(other.storage_)}
    {
        if (other)
        {
            EmplaceAt(&storage_.GetUnsafe(), other.storage_.GetUnsafe());
        }
    }
    Nullable& operator=(const Nullable& other)
    {
        this->has_value_ = other.has_value_;
        Deleter& ref_deleter = storage_;
        ref_deleter = static_cast<Deleter>(other.storage_);
        if (other)
        {
            EmplaceAt(&storage_.GetUnsafe(), other.storage_.GetUnsafe());
        }
    }
    Nullable(Nullable&& other) noexcept
        : has_value_{other.has_value_}, storage_{std::move(static_cast<Deleter>(other.storage_))}
    {
        if (other)
        {
            EmplaceAt(&storage_.GetUnsafe(), std::move(other.storage_.GetUnsafe()));
        }
    }
    Nullable& operator=(Nullable&& other) noexcept
    {
        this->has_value_ = other.has_value_;
        Deleter& ref_deleter = storage_;
        ref_deleter = std::move(static_cast<Deleter>(other.storage_));
        if (other)
        {
            EmplaceAt(&storage_.GetUnsafe(), std::move(other.storage_.GetUnsafe()));
        }
    }

    template <class... Args>
    void Construct(Args&&... args)
    {
        if (has_value_)
        {
            DestroySelf();
            has_value_ = false;
        }

        EmplaceAt(&storage_.GetUnsafe(), std::forward<Args>(args)...);

        has_value_ = true;
    }
    const T& GetUnsafe() const noexcept
    {
        return storage_.GetUnsafe();
    }
    T& GetUnsafe() noexcept
    {
        return storage_.GetUnsafe();
    }
    const T& Get() const noexcept
    {
        Check();
        return GetUnsafe();
    }
    T& Get() noexcept
    {
        Check();
        return GetUnsafe();
    }
    bool HasValue() const noexcept
    {
        return has_value_;
    }
    operator bool() const noexcept
    {
        return HasValue();
    }

private:
    void DestroySelf()
    {
        auto&& ref_deleter = static_cast<Deleter&>(storage_);
        ref_deleter(&Get());
    }
    void Check() const
    {
        if (!has_value_)
        {
            throw CallNullObjectError{};
        }
    }

    class CallNullObjectError : public std::runtime_error
    {
    public:
        CallNullObjectError()
            : std::runtime_error{"Value is uninitialized!"} {}
        ~CallNullObjectError() override = default;
    };

    class StorageAndEboDeleter : public Deleter
    {
    private:
        StorageType storage_;

        auto GetStoragePointer() const noexcept
            -> const StorageType*
        {
            return std::addressof(storage_);
        }
        auto GetStoragePointer() noexcept
            -> StorageType*
        {
            return std::addressof(storage_);
        }

    public:
        explicit StorageAndEboDeleter(Deleter deleter)
            : Deleter{deleter} {}
        ~StorageAndEboDeleter() = default;
        StorageAndEboDeleter(const StorageAndEboDeleter&) = delete;
        StorageAndEboDeleter& operator=(const StorageAndEboDeleter&) = delete;

        const T& GetUnsafe() const noexcept
        {
            return *static_cast<const T*>(static_cast<const void*>(GetStoragePointer()));
        }
        T& GetUnsafe() noexcept
        {
            return *static_cast<T*>(static_cast<void*>(GetStoragePointer()));
        }
    };
    bool has_value_{false};
    StorageAndEboDeleter storage_;
};
template <class T, class Deleter = NullableDefaultDeleter<T>>
auto MakeNullableObject(Deleter deleter)
    -> Nullable<T, Deleter>
{
    return {deleter};
}

/**
 * @brief 可延迟构造的对象，用于预先分配内存，在有使用该对象的请求时立即构造此对象，对象必须可默认构造
 *
 * @tparam T 要被应用这一特性的类型
 * @tparam Deleter T的删除器，默认为NullableDefaultDeleter<T>
 */
template <class T, class Deleter = NullableDefaultDeleter<T>>
class LazyConstructable
{
public:
    LazyConstructable() = default;
    ~LazyConstructable() = default;
    T& Get()
    {
        if (content_)
        {
            return content_.GetUnsafe();
        }
        else
        {
            content_.Construct();
            return content_.GetUnsafe();
        }
    }

private:
    Nullable<T, Deleter> content_{};
};

template <class T, class Deleter, class Tuple>
class LazyConstructableWithInitializer;

template <class T, class Deleter, template <class...> class Container, class... InitArgs>
class LazyConstructableWithInitializer<T, Deleter, Container<InitArgs...>>
{
private:
    using ArgsContainer = Container<InitArgs...>;
    using ArgsInitFunction = std::function<ArgsContainer()>;
    constexpr static std::size_t init_args_size = sizeof...(InitArgs);
    template <class Tuple, std::size_t... Indexs>
    void ConstructHelper(Tuple&& args, std::index_sequence<Indexs...>)
    {
        content_.Construct(std::get<Indexs>(args)...);
    }

public:
    LazyConstructableWithInitializer(ArgsInitFunction init_function)
        : init_function_{init_function}
    {
    }
    ~LazyConstructableWithInitializer() = default;
    T& Get()
    {
        if (content_)
        {
            return content_.GetUnsafe();
        }
        else
        {
            auto init_args{std::move(init_function_())};
            ConstructHelper(
                init_args,
                std::make_index_sequence<std::tuple_size<decltype(init_args)>{}>{});
            return content_.GetUnsafe();
        }
    }

private:
    Nullable<T, Deleter> content_{};
    ArgsInitFunction init_function_{};
};

template <class T, class... Args>
using DefaultCLazyConstructableWithInitializer =
    LazyConstructableWithInitializer<T, NullableDefaultDeleter<T>, std::tuple<Args...>>;

DTL_NAMESPACE_END
#endif
