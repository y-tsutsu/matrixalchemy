#pragma once

#if __has_include(<glad/gl.h>)
#include <glad/gl.h>
#define MATRIXALCHEMY_GLAD2 1
#elif __has_include(<glad/glad.h>)
#include <glad/glad.h>
#define MATRIXALCHEMY_GLAD1 1
#else
#error "GLAD headers were not found."
#endif
