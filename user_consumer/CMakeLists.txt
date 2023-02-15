cmake_minimum_required(VERSION 3.23)
project(user_consumer)

set(CMAKE_CXX_STANDARD 14)

add_executable(user_consumer main.cpp single_user_single_consumer.hpp multi_user_single_producer.hpp single_user_multi_producers.hpp multi_users_multi_producers.hpp)
