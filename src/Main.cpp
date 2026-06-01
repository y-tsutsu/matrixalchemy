#include "matrixalchemy/App.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>

int main()
{
    try
    {
        matrixalchemy::App app;
        return app.run();
    }
    catch (const std::exception &error)
    {
        std::cerr << "matrixalchemy: " << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
