#pragma once

namespace cube::programs
{

// Signals
void sigint_library();
void sigint_render();

// Main entry
int main_library(int ac, char const * const av[]);
int main_render(int ac, char const * const av[]);

}
