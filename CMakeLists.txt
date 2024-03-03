cmake_minimum_required (VERSION 3.8)

project ("xui")

add_executable (xui "src/main.cpp" "src/xui.cpp" "src/gdi_implement.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET xui PROPERTY CXX_STANDARD 20)
endif()
