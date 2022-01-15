# LED Cube Engine
<p align="center">
    <img src="resources/demo.gif" alt="OpenGL Mock Demo"/>
</p>

## Getting started
Make sure you have the following packages installed on your system:
- Compiler of your choice (GCC and Clang are tested and supported);
- Boost;
- CMake (version 3.13 or higher);

To build and run the `mock` target follow the steps below:

1. Install GCC, cmake, boost and the required packages for the `mock` target:
    ```bash
    $ sudo apt update
    $ sudo apt install build-essential cmake libboost-dev
    $ sudo apt install libopengl0 libopengl-dev libglfw3 libglfw3-dev libglew2.1 libglew-dev # Dependencies only for mock target
    ```
1. Clone this repository:
    ```bash
    $ git clone https://github.com/LimpSquid/led-cube-engine.git
    ```
1. Change directory to the root of the repository and make a build folder:
    ```bash
    $ cd led-cube-engine
    $ mkdir build
    ```
1. Change directory to the build folder and configure the project:
    ```bash
    $ cd build
    $ cmake ..
    ```
1. If desired, change the LED cube engine's target (default is `mock`):
    ```bash
    $ cmake -D LCE_TARGET=<new_target> .
    ```
1. Build the source:
    ```bash
    $ cmake --build .
    ```
1. Render an animation:
    ```bash
    $ bin/led-cube-engine render --animation stars
    $ bin/led-cube-engine render --animation lightning '{"color_gradient_start": {"red": 255, "green": 128}, "color_gradient_end": {"color": "random"}}'
    $ bin/led-cube-engine render --animation helix '{"helix_rotation_time_ms":1500,"helix_phase_shift_sin_factor":0.04,"helix_thickness":2,"helix_length":4,"color_gradient_start":{"color":"magenta"},"color_gradient_end":{"color":"#24e6ebff"}}'
    ```
1. Finally, see the [programs](#programs) section for a detailed overview of all commands that are available.

## Build options
Below an overview of all the available build options:

option|type|description|default
-|-|-|-
`LCE_TARGET`|string|Select the LED cube engine's target graphics device. See the [*target dependencies*](#target-dependencies) section for more information.|`mock`
`LCE_EVAL_EXPRESSIONS`|bool|Enable string expressions for JSON number fields. See the [*evaluate animation property expressions*](#evaluate-animation-property-expressions) section for more information. **Note: enabling this option will significantly increase the compile time when you build the application for the first time.**|`off`

## Programs
The application is divided into multiple programs for easy access to core functionalities of the LED cube engine. The following sub-sections will describe each program in detail to get a overview of all the capabilities of a program. To list all the available programs, execute the following command:
```bash
$ ./led-cube-engine --help
```

### Library
The `library` program is used to list information about the LED cube engine's animation library. To list all the available options of the `library` program, execute the following command:
```bash
$ ./led-cube-engine library --help
```

**--list**
> List all the available animations of the library.

**--info \<name>**
> Print information about one or more animations, the `name` parameter must be one of the animations that are listed with the `--list` option. To print information about multiple animations, simply cascade the animation names, e.g. `--info stars helix`

**--dump-properties \<name> \<properties>**
> Load an animation with the specified properties and dump the resulting properties to the command line interface. The `name` parameter must be one of the animations that are listed with the `--list` option. The `properties` parameter must be a JSON object string with fields that are overriding the default properties of the animation. When the build option `LCE_EVAL_EXPRESSIONS` is used, any expression is also evaluated.

### Render
This program is used to render animations from the command line interface to the target graphics device; or render multiple animations from a JSON file. To list all the available options of the `render` program, execute the following command:
```bash
$ ./led-cube-engine render --help
```

**--animation \<name> [properties]**
> Render an animation from the command line interface to the LED cube engine's graphics device. The `name` parameter must be one of the animations that are listed with the `--list` option from the `library` program. The optional `properties` parameter must be a valid JSON object string with fields unique for the animation passed to the `name` parameter. To list all the default properties of an animation use the `--info` option from the `library` program.
>
> A few usage examples are as follows:
> ```bash
> $ ./led-cube-engine render --animation helix
> $ ./led-cube-engine render --animation lightning '{"color_gradient_start": {"color": "magenta"}, "color_gradient_end": {"color": "#24e6ebff"}}'
> $ ./led-cube-engine render --animation lightning '{"color_gradient_start": {"red": 100, "blue":128}, "color_gradient_end": {"color": "random"}}'
> ```

**-f, --file \<path_to_file>**
> Render one or more animations from a JSON file to the LED cube engine's graphics device. The `path_to_file` parameter must refer to a file that contains a valid JSON array with one or more animations. To render animations from multiple files simply cascade the paths, e.g. `--file /path/to/first_file /path/to/second_file`. The animations are played in a sequential fashion, by changing the order of the JSON array elements, or by changing the order of the filepaths, you can change the order in which animations are rendered by the engine.
>
> The format of the JSON array containing the animation definitions is as follows:
> ```JSON
> [
>   {
>     "animation": "name"
>   },
>   {
>     "animation": "name",
>     "properties": {
>       "string": "foo",
>       "int": 10,
>       "int_expression": "random(cube_axis_min_value, cube_axis_max_value)",
>       "double": 123.456,
>       "double_expression": "sin(0.5 * pi * random(0.0, 1.0)) * cube_size_1d"
>     }
>   }
> ]
> ```
>
> The `animation` field is required, it specifies the name of the animation which must be one of the animations that are listed with the `--list` option from the `library` program. If it does not exist an error will be given.
>
> The `properties` field is optional and, if provided, must be a valid JSON object string with fields unique for the animation passed to the `animation` field. Note that the example property fields `int_expression` and `double_expression` are only allowed when the build option `LCE_EVAL_EXPRESSIONS` is enabled. If the build option is not enabled, and a string value was given to a number field (e.g. `int`, `double`), an error will be returned. If a property field is missing, the default values will be used. It is allowed to provide additional fields with a key that is not matching any of the animation's properties, those fields will simply be ignored.

## Creating a new animation
Naturally you want to create your own animations. The process of adding an animation to the LED cube engine is a straightforward process. All the animations can be be found in the directory `<repo_root>/src/cube/gfx/animations`. To add a new one, simply create a new header and source file and use the templates that are provided down below:

```c++
#pragma once

#include <cube/gfx/configurable_animation.hpp>

namespace cube::gfx::animations
{

class my_animation :
    public configurable_animation
{
public:
    my_animation(core::engine_context & context);

private:
    // Optional, properties that are to be configured via JSON
    PROPERTY_ENUM
    (
        my_int_property,
        my_double_property,
    )

    virtual void start() override; // Optional, called before the animation is started
    virtual void paint(core::graphics_device & device) override; // Required, paint a single animation frame
    virtual void stop() override; // Optional, called after the animation is finished
    virtual nlohmann::json properties_to_json() const override; // Optional, implement if we have atleast one property
    virtual std::vector<property_pair_t> properties_from_json(nlohmann::json const & json) const override; // Optional, implement if we have atleast one property

    // Optional, use this if you want to render at the specified scene frame rate
    core::animation_scene scene_;
};

} // End of namespace
```

```c++
#include <cube/gfx/animations/my_animation.hpp>
#include <cube/gfx/library.hpp>
#include <cube/core/painter.hpp>

using namespace cube::gfx;
using namespace cube::core;

namespace
{

// Adds the animation to library with name "my_animation".
// Optionally, you can pass an alternative name to the
// publisher's constructor.
animation_publisher<animations::my_animation> const publisher;

// Some default animation property values
constexpr int default_int_property = 12345;
constexpr double default_double_property = 3.14;

} // End of namespace

namespace cube::gfx::animations
{

my_animation::my_animation(engine_context & context) :
    configurable_animation(context),
    scene_(*this, [this](auto elapsed) { /* Do some additional processing each scene frame */ })
{ }

void my_animation::start()
{
    // Do some additional processing before the animation starts
    // ...
    // ...

    // For example read the int property:
    auto const int_value = read_property(my_int_property, default_int_property);

    scene_.start();
}

void my_animation::paint(graphics_device & device)
{
    painter p(device);
    p.wipe_canvas();

    // Draw something interesting with the use of the painter
    // ...
    // ...
}

void my_animation::stop()
{
    // Do some additional processing after the animation has finished
    // ...
    // ...

    scene_.stop();
}

nlohmann::json my_animation::properties_to_json() const
{
    return {
        to_json(my_int_property, default_int_property),
        to_json(my_double_property, default_double_property),
    };
}

std::vector<my_animation::property_pair_t> my_animation::properties_from_json(nlohmann::json const & json) const
{
    return {
        from_json(json, my_int_property, default_int_property),
        from_json(json, my_double_property, default_double_property),
    };
}

} // End of namespace
```

If you've succesfully provisioned your animation, you can simply re-compile the application and execute the following commands:
```bash
$ ./led-cube-engine library --list # To see if your animation has been added to the library
$ ./led-cube-engine library --info my_animation # To see the exposed properties in JSON
$ ./led-cube-engine render --animation my_animation # Finally, render it
```

## Evaluate animation property expressions
With the build option `LCE_EVAL_EXPRESSIONS` enabled it is possible to further customize an animation. Usually an animation contains one or more number properties. For example, the `helix` animation has a decimal property `helix_length` to change the length of the helix. By default this is a hardcoded value, which you can set to some different decimal, for example by providing `"helix_length": 2.0`. This will modify the `helix` animation in a way that the helix is of exactly two full rotations in length.

To make this property more interesting we can use an expression which assigns some random number to `helix_length`:
```JSON
"helix_length": "random(0.5, 4.0)"
```

With this expression, whenever the animation is first loaded, a random number from the interval `[0.5, 4.0]` is assigned to `helix_length`. We can even go a step further and improve this expression given some constants that are available during the expression evaluation:
```JSON
"helix_length": "random(0.5, cube_size_1d / 4)"
```
With this expression the LED cube's size is also taken into consideration. For example, when the cube has a size of `16` a random number will be picked from the interval `[0.5, 4.0]`. If the cube has a size of `32` a random number will be picked from the interval `[0.5, 8.0]`.

See the following [docs](https://www.partow.net/programming/exprtk/index.html) for all the available mathematical expressions. The sections below will describe the available constants and custom functions.

### Constants
constant|value
-|-
`cube_size_1d`|number of voxels from a single side of the cube
`cube_size_2d`|same as `cube_size_1d * cube_size_1d`
`cube_size_3d`|same as `cube_size_1d * cube_size_1d * cube_size_1d`
`cube_axis_min_value`|`0`
`cube_axis_max_value`|same as `cube_size_1d - 1`

### Custom functions
function|description|example usage
-|-|-
`random(min, max)`|pick a random number from the range `[min, max]`|`random(0.0, 1.0)`, `random(-1.0, 3.25 * pi)`

## Target dependencies
Below a table with the supported targets and their respective dependencies.
target|description|package dependency
-|-|-
`mock`|A mock environment to run the animations on your local machine via OpenGL| `libopengl0`, `libopengl-dev`, `libglfw3`, `libglfw3-dev`, `libglew2.1` and `libglew-dev`
`rpi`|A Raspberry PI hooked up to 16 [LED controller](https://github.com/LimpSquid/led-controller) boards|**T.B.D**
