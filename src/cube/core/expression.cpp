#ifdef EVAL_EXPRESSION
#include <cube/core/expression.hpp>
#include <cube/specs.hpp>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <3rdparty/exprtk/exprtk.hpp>
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

namespace
{

template<typename T>
T do_eval(std::string const & str)
{
    using symbol_table_t = exprtk::symbol_table<T>;
    using expression_t = exprtk::expression<T>;
    using parser_t = exprtk::parser<T>;

    symbol_table_t symbol_table;
    symbol_table.add_constant("cube_size_1d", static_cast<T>(cube::cube_size_1d));
    symbol_table.add_constant("cube_size_2d", static_cast<T>(cube::cube_size_2d));
    symbol_table.add_constant("cube_size_3d", static_cast<T>(cube::cube_size_3d));
    symbol_table.add_constant("cube_size_3d", static_cast<T>(cube::cube_size_3d));
    symbol_table.add_constant("cube_size_3d", static_cast<T>(cube::cube_size_3d));
    symbol_table.add_constant("cube_axis_min_value", static_cast<T>(cube::cube_axis_min_value));
    symbol_table.add_constant("cube_axis_max_value", static_cast<T>(cube::cube_axis_max_value));
    symbol_table.add_constants();

    expression_t expression;
    expression.register_symbol_table(symbol_table);

    parser_t parser;
    if (!parser.compile(str, expression))
        throw std::runtime_error("Expression compilation error: " + parser.error());
    return expression.value();
}

} // End of namespace

namespace cube::core
{

float evalf(std::string const & str)
{
    return do_eval<float>(str);
}

double evald(std::string const & str)
{
    return do_eval<double>(str);
}

long double evalld(std::string const & str)
{
    return do_eval<long double>(str);
}

} // End of namespace
#endif
