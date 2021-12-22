# Getting started
This project uses CMake to build the executable. Make sure you have CMake version 3.13 or higher installed. To build and run the executable follow these steps:

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
    $ cmake -D LCE_TARGET=<new_target>
    ```
1. Build source:
    ```bash
    $ cmake --build .
    ```
1. Run executable
    ```bash
    $ bin/led-cube-engine
    ```

# Target dependencies
Below a table with the supported targets and their respective dependencies.
target|description|dependencies
-|-|-
`mock`|A mock environment to run the animations on your local machine via OpenGL|`OpenGL`, `GLEW` and `GLFW`
`rpi`|A Raspberry PI hooked up to 16 [LED controller](https://github.com/LimpSquid/led-controller) boards|**T.B.D**
