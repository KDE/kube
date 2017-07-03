cmake_minimum_required(VERSION 3.0)

cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0063 NEW)

project(kube VERSION 0.4)

enable_testing()

add_subdirectory(framework)
add_subdirectory(components)
add_subdirectory(accounts)
add_subdirectory(icons)
add_subdirectory(applications)

add_subdirectory(tests)
