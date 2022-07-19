#pragma once

#include <memory>
#include <functional>

namespace cube::core
{

class engine_context;
class animation;
class graphics_device;

namespace detail
{

class animation_session
{
public:
    animation_session();
    ~animation_session();

    void set(std::shared_ptr<cube::core::animation> animation);
    cube::core::animation & operator*();
    operator bool() const;

private:
    animation_session(animation_session &) = delete;
    animation_session(animation_session &&) = delete;

    std::shared_ptr<cube::core::animation> animation_;
};

} // End of namespace

class basic_engine
{
public:
    using predicate_t = std::function<bool(void)>;

    basic_engine(engine_context & context);

    engine_context & context();
    void run();
    void run_while(predicate_t predicate);
    void stop();

private:
    basic_engine(basic_engine &) = delete;
    basic_engine(basic_engine &&) = delete;

    virtual void poll_one(bool stopping) = 0;

    template<typename ... F>
    void do_run(F ... extra);

    engine_context & context_;
    bool stopping_;
};

template<typename T>
struct graphics_device_factory
{
    using type = T;

    std::unique_ptr<type> operator()(engine_context & context) const
    {
        return std::make_unique<type>(context);
    }
};

class render_engine :
    public basic_engine
{
public:
    template<typename T>
    render_engine(engine_context & context, graphics_device_factory<T> factory) :
        basic_engine(context),
        device_(factory(context))
    { }

    void load(std::shared_ptr<animation> animation);

private:
    render_engine(render_engine &) = delete;
    render_engine(render_engine &&) = delete;

    void poll_one(bool stopping) override;

    detail::animation_session animation_session_;
    std::unique_ptr<graphics_device> device_;
};

class poll_engine :
    public basic_engine
{
public:
    poll_engine(engine_context & context);

private:
    poll_engine(poll_engine &) = delete;
    poll_engine(poll_engine &&) = delete;

    void poll_one(bool stopping) override;
};

} // End of namespace
