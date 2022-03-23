#pragma once

#include <memory>
#include <functional>
#include <tuple>
#include <optional>

namespace cube::core
{

template<typename F>
class composite_function
{
public:
    static auto decompose(F f)
    {
        return make_shared(std::move(f), function_signature(&F::operator()))->decompose();
    }

private:
    template<typename ... A>
    struct function_signature
    {
        template<typename R, typename C>
        constexpr function_signature(R(C::*)(A...) const)
        { }
    };

    template<typename ... A>
    struct impl :
        std::enable_shared_from_this<impl<A ...>>
    {
        template<typename T>
        using normalized_t = typename std::remove_cv_t<std::remove_reference_t<T>>;
        template<typename T>
        using normalized_optional_t = std::optional<normalized_t<T>>;

        template<std::size_t I, typename T>
        struct closure
        {
            std::shared_ptr<impl> i;

            void operator()(T && value)
            {
                if (i) {
                    std::get<I>(i->args) = std::move(value);
                    i->arg_set();
                    i.reset();
                }
            }
        };

        impl(F && f) :
            function(std::move(f))
        { }

        template<std::size_t ... I>
        std::tuple<std::function<void(A)> ...> decompose(std::index_sequence<I ...>)
        {
            return std::tuple(closure<I, A>{this->shared_from_this()} ...);
        }

        std::tuple<std::function<void(A)> ...> decompose()
        {
            return decompose(std::make_index_sequence<sizeof ... (A)>{});
        }

        template<std::size_t ... I>
        void aggregate_and_forward(std::index_sequence<I ...>)
        {
            function(*std::get<I>(args) ...);
        }

        void arg_set()
        {
            if (++n_args == sizeof ... (A))
                aggregate_and_forward(std::make_index_sequence<sizeof ... (A)>{});
        }

        F function;
        std::tuple<normalized_optional_t<A> ...> args;
        std::size_t n_args{0};
    };

    template<typename ... A>
    static std::shared_ptr<impl<A ...>> make_shared(F && f, function_signature<A ...>)
    {
        return std::make_shared<impl<A ...>>(std::move(f));
    }
};

template<typename F>
auto decompose(F f)
{
    return composite_function<F>::decompose(std::move(f));
}

} // End of namespace
