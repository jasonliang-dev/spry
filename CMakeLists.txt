cmake_minimum_required(VERSION 3.12...3.27)
project(Spry LANGUAGES C CXX)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DDEBUG)

set(BOX2D
  src/deps/box2d/collision/b2_broad_phase.cpp
  src/deps/box2d/collision/b2_chain_shape.cpp
  src/deps/box2d/collision/b2_circle_shape.cpp
  src/deps/box2d/collision/b2_collide_circle.cpp
  src/deps/box2d/collision/b2_collide_edge.cpp
  src/deps/box2d/collision/b2_collide_polygon.cpp
  src/deps/box2d/collision/b2_collision.cpp
  src/deps/box2d/collision/b2_distance.cpp
  src/deps/box2d/collision/b2_dynamic_tree.cpp
  src/deps/box2d/collision/b2_edge_shape.cpp
  src/deps/box2d/collision/b2_polygon_shape.cpp
  src/deps/box2d/collision/b2_time_of_impact.cpp
  src/deps/box2d/common/b2_block_allocator.cpp
  src/deps/box2d/common/b2_draw.cpp
  src/deps/box2d/common/b2_math.cpp
  src/deps/box2d/common/b2_settings.cpp
  src/deps/box2d/common/b2_stack_allocator.cpp
  src/deps/box2d/common/b2_timer.cpp
  src/deps/box2d/dynamics/b2_body.cpp
  src/deps/box2d/dynamics/b2_chain_circle_contact.cpp
  src/deps/box2d/dynamics/b2_chain_polygon_contact.cpp
  src/deps/box2d/dynamics/b2_circle_contact.cpp
  src/deps/box2d/dynamics/b2_contact.cpp
  src/deps/box2d/dynamics/b2_contact_manager.cpp
  src/deps/box2d/dynamics/b2_contact_solver.cpp
  src/deps/box2d/dynamics/b2_distance_joint.cpp
  src/deps/box2d/dynamics/b2_edge_circle_contact.cpp
  src/deps/box2d/dynamics/b2_edge_polygon_contact.cpp
  src/deps/box2d/dynamics/b2_fixture.cpp
  src/deps/box2d/dynamics/b2_friction_joint.cpp
  src/deps/box2d/dynamics/b2_gear_joint.cpp
  src/deps/box2d/dynamics/b2_island.cpp
  src/deps/box2d/dynamics/b2_joint.cpp
  src/deps/box2d/dynamics/b2_motor_joint.cpp
  src/deps/box2d/dynamics/b2_mouse_joint.cpp
  src/deps/box2d/dynamics/b2_polygon_circle_contact.cpp
  src/deps/box2d/dynamics/b2_polygon_contact.cpp
  src/deps/box2d/dynamics/b2_prismatic_joint.cpp
  src/deps/box2d/dynamics/b2_pulley_joint.cpp
  src/deps/box2d/dynamics/b2_revolute_joint.cpp
  src/deps/box2d/dynamics/b2_weld_joint.cpp
  src/deps/box2d/dynamics/b2_wheel_joint.cpp
  src/deps/box2d/dynamics/b2_world.cpp
  src/deps/box2d/dynamics/b2_world_callbacks.cpp
  src/deps/box2d/rope/b2_rope.cpp
)

set(LUASOCKET
  src/deps/luasocket/auxiliar.c
  src/deps/luasocket/buffer.c
  src/deps/luasocket/compat.c
  src/deps/luasocket/except.c
  src/deps/luasocket/inet.c
  src/deps/luasocket/io.c
  src/deps/luasocket/luasocket.c
  src/deps/luasocket/mime.c
  src/deps/luasocket/options.c
  src/deps/luasocket/select.c
  src/deps/luasocket/tcp.c
  src/deps/luasocket/timeout.c
  src/deps/luasocket/udp.c
)

if (WIN32)
  set(LUASOCKET
    ${LUASOCKET}
    src/deps/luasocket/wsocket.c
  )
endif()

if (UNIX)
  set(LUASOCKET
    ${LUASOCKET}
    src/deps/luasocket/serial.c
    src/deps/luasocket/unix.c
    src/deps/luasocket/unixdgram.c
    src/deps/luasocket/unixstream.c
    src/deps/luasocket/usocket.c
  )
endif()

file(GLOB SOURCES CONFIGURE_DEPENDS
  src/*.cpp
  src/*.h
)

add_executable(${PROJECT_NAME}
  ${SOURCES}
  ${BOX2D}
  ${LUASOCKET}
)

target_include_directories(${PROJECT_NAME} PRIVATE
  src/deps/box2d
  src/deps/lua
)