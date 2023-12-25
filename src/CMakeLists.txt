set(CMAKE_CXX_STANDARD  20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

include_directories(Hiena ${PROJECT_SOURCE_DIR}/include/Hiena)

add_library(Hiena STATIC Hiena.cpp
                         JavaLang.cpp
                         JavaObjectBase.cpp)