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

1. Install boost and the required packages for the `mock` target:
    ```bash
    $ sudo apt update
    $ sudo apt install build-essential cmake libboost-dev
    $ sudo apt install libopengl0 libopengl-dev libglfw3 libglfw3-dev libglew2.1 libglew-dev # Dependencies only for mock target
    ```
1. Clone this repository:
    ```bash
    $ git clone https://github.com/LimpSquid/led-cube-engine.git
    ```
1. Change directory to the root of the repository and make a build directory:
    ```bash
    $ cd led-cube-engine
    $ mkdir build
    ```
1. Change directory to the build folder and configure the project:
    ```bash
    $ cd build
    $ cmake ..
    ```
1. If desired change the LED cube engine's target (default is `mock`):
    ```bash
    $ cmake -D LCE_TARGET=<new_target> .
    ```
1. Build source:
    ```bash
    $ cmake --build .
    ```
1. Render an animation:
    ```bash
    $ bin/led-cube-engine render --animation stars
    $ bin/led-cube-engine render --animation lightning '{"color_gradient_start": {"red": 255, "green": 128}}'
    $ bin/led-cube-engine render --animation helix '{"helix_rotation_time_ms":1500,"helix_phase_shift_sin_factor":0.04,"helix_thickness":2,"helix_length":4,"color_gradient_start":{"name":"magenta"},"color_gradient_end":{"name":"cyan"}}'
    ```
1. Finally, check out the [programs](#programs) section for a detailed overview of all commands that are available.

## Build options
Below an overview of all the available build options endpoints that can be accessed by a client that is using the system key UUID.

option|type|description|default
-|-|-|-
`LCE_TARGET`|string|Select the LED cube engine's target graphics device. See the [target dependencies](#target-dependencies) section for more information.|`mock`
`LCE_EVAL_EXPRESSION`|bool|Enable string expressions for JSON number fields. See the [evaluate animation property expressions](#evaluate-animation-property-expressions) section for more information.|`off`

## Programs
The application is subdivided into multiple programs for easy access to core functionalities of the LED cube engine. The following sub-sections will describe each program in detail to get a full overview of all the related capabilities of a program. To list all the available programs, execute the following command:
```bash
$ ./led-cube-engine --help
```

### Library
The `library` program is used to retrieve and list information about the LED cube engine's internal animation library. To list all the available options of the `library` program, execute the following command:
```bash
$ ./led-cube-engine library --help
```

**--list**
> List all the available animation of the library.

**--info \<name>**
> Print information about one or more animations, the `name` parameter must be one of the animations that are listed with the `--list` option. To print information about multiple animations, simply cascade multiple animations, e.g. `--info stars helix`

### Render
The `render` program is maybe one of the most important programs of the LED cube engine. This program is used to directly render animations from the command line interface to the target graphics device; or render multiple animations from a JSON file. To list all the available options of the `render` program, execute the following command:
```bash
$ ./led-cube-engine render --help`
```

**--animation \<name> [properties]**
> Render an animation directly from the command line interface to the LED cube engine's graphics device. The `name` parameter must be one of the animations that are listed with the `--list` option from the `library` program. The optional `properties` parameter must be a JSON object string with valid fields unique to each animation. To list all the (default) property fields of an animation use the `--info` option from the `library` program.
>
> A few usage examples are as follows:
> ```bash
> $ ./led-cube-engine render --animation helix
> $ ./led-cube-engine render --animation lightning '{"color_gradient_start": {"name": "magenta"}}'
> $ ./led-cube-engine render --animation lightning '{"color_gradient_start": {"red": 100, "blue":128}}'
> ```

**-f, --file \<path_to_file>**
> **TODO**

## Target dependencies
Below a table with the supported targets and their respective dependencies.
target|description|package dependency
-|-|-
`mock`|A mock environment to run the animations on your local machine via OpenGL| `libopengl0`, `libopengl-dev`, `libglfw3`, `libglfw3-dev`, `libglew2.1` and `libglew-dev`
`rpi`|A Raspberry PI hooked up to 16 [LED controller](https://github.com/LimpSquid/led-controller) boards|**T.B.D**

## Evaluate animation property expressions
**TODO**
