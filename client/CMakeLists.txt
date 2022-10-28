add_executable(chatclient src/chat_client.cpp)
include_directories(include/)
target_link_libraries(chatclient PRIVATE ${Python3_LIBRARIES})
target_include_directories(chatclient PUBLIC ${Python3_INCLUDE_DIRS})