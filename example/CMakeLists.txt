add_executable(clock main-clock.cpp)
target_link_libraries(clock PRIVATE cdboost)

add_executable(custom-event-list main-custom-event-list.cpp)
target_link_libraries(custom-event-list PRIVATE cdboost)

add_executable(echobox main-echobox.cpp)
target_link_libraries(echobox PRIVATE cdboost)
