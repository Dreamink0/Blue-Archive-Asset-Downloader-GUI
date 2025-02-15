cmake_minimum_required(VERSION 3.30)
project(Blue_Archive_Asset_Downloader_GUI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(ext/ftxui)

add_executable(Blue_Archive_Asset_Downloader_GUI main.cpp)

target_link_libraries(Blue_Archive_Asset_Downloader_GUI PRIVATE ftxui::screen ftxui::dom ftxui::component)
