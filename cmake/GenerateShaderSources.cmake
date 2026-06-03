file(READ "${VERTEX_SHADER}" COLOR_VERTEX_SHADER)
file(READ "${FRAGMENT_SHADER}" COLOR_FRAGMENT_SHADER)

set(GENERATED_SHADER_SOURCES "#pragma once\n\n#include <string_view>\n\nnamespace matrixalchemy::render::shader_sources\n{\n\ninline constexpr std::string_view colorVertex = R\"malc(${COLOR_VERTEX_SHADER})malc\";\ninline constexpr std::string_view colorFragment = R\"malc(${COLOR_FRAGMENT_SHADER})malc\";\n\n} // namespace matrixalchemy::render::shader_sources\n")

get_filename_component(OUTPUT_DIR "${OUTPUT_FILE}" DIRECTORY)
file(MAKE_DIRECTORY "${OUTPUT_DIR}")
file(WRITE "${OUTPUT_FILE}" "${GENERATED_SHADER_SOURCES}")
