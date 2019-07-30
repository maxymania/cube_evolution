#
#   Copyright 2019 Simon Schmidt
#
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0)

#include_directories( ./inc )
include_directories( ./enet/include )
include_directories( ./guf/inc )


file(GLOB ENET
	enet/*.c
	)

file(GLOB GUF
	guf/*.cpp
	)

file(GLOB CUBE_CLIENT
	src/client.cpp
	src/clientextras.cpp
	src/clientgame.cpp
	src/clients2c.cpp
	src/command.cpp
	src/console.cpp
	src/editing.cpp
	src/entities.cpp
	src/main.cpp
	src/menus.cpp
	src/monster.cpp
	src/physics.cpp
	src/rendercubes.cpp
	src/renderextras.cpp
	src/rendergl.cpp
	src/rendermd2.cpp
	src/renderparticles.cpp
	src/rendertext.cpp
	src/rndmap.cpp
	src/savegamedemo.cpp
	src/server.cpp
	src/serverbrowser.cpp
	src/serverms.cpp
	src/serverutil.cpp
	src/sound.cpp
	src/tools.cpp
	src/weapon.cpp
	src/world.cpp
	src/worldio.cpp
	src/worldlight.cpp
	src/worldocull.cpp
	src/worldrender.cpp
	)

file(GLOB CUBE_SERVER
	src/standalone.cpp
	src/serverms.cpp
	src/server.cpp
	src/tools.cpp
	)

add_executable(client ${ENET} ${GUF} ${CUBE_CLIENT})

target_link_libraries(client SDL2)
target_link_libraries(client SDL2_image)
target_link_libraries(client SDL2_mixer)
#target_link_libraries(client GL)
target_link_libraries(client EGL)
target_link_libraries(client GLESv2)
target_link_libraries(client z)

add_executable(server ${ENET} ${CUBE_SERVER})


