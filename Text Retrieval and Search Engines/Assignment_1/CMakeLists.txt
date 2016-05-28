cmake_minimum_required(VERSION 3.2.0)

# MeTA only requires C++11, but on all of our supported compilers at least
# C++1y support exists, so we therefore set the standard to 14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(../meta ${CMAKE_CURRENT_BINARY_DIR}/meta EXCLUDE_FROM_ALL)

add_executable(analyze analyze.cpp)
target_link_libraries(analyze meta-index
                              meta-greedy-tagger
                              meta-parser
                              ${CMAKE_THREAD_LIBS_INIT})

add_executable(ranking-experiment ranking-experiment.cpp)
target_link_libraries(ranking-experiment meta-index
                                   meta-ranker
                                   meta-sequence-analyzers
                                   meta-parser-analyzers)

add_executable(relevance-judgements relevance-judgements.cpp)
target_link_libraries(relevance-judgements meta-index
                                           meta-ranker
                                           meta-sequence-analyzers
                                           meta-parser-analyzers)
