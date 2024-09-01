######################################################################################
# CMake directives
######################################################################################

# Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)


######################################################################################
# Project declaration and options
######################################################################################

# Notre projet est étiqueté Sega_Dumper
project(Sega_Dumper)

# Crée des variables avec les fichiers à compiler
set(SRCS
    main.c
    )

######################################################################################
# CMake
######################################################################################
    
# Notre exécutable
add_executable(MD_Dumper ${SRCS})


######################################################################################
# Dependencies and Definitions
######################################################################################

# On recherche la présence de la lib externe

find_package(PkgConfig REQUIRED)

if (INCLUDE_EXTRA_CMAKELISTS)
	pkg_search_module(LIBUSB1 REQUIRED libusb-1.0)
	include_directories(SYSTEM ${LIBUSB1_INCLUDE_DIRS})

 	pkg_search_module(SDL2 REQUIRED sdl2)
	include_directories(SYSTEM ${SDL2_INCLUDE_DIRS})

	pkg_search_module(SDL2_IMAGE REQUIRED SDL2_image)
	include_directories(SYSTEM ${SDL2_IMAGE_INCLUDE_DIRS})

 	find_package(Libusb1 QUIET)
else ()
	find_path(LIBUSB_INCLUDE_DIR NAMES libusb.h PATH_SUFFIXES "include" "libusb" "libusb-1.0")
	find_library(LIBUSB_LIBRARY NAMES usb PATH_SUFFIXES "lib" "lib32" "lib64")
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "./cmake")

if (LIBUSB1_FOUND)
	message("libusb found")
	set(_includes ${LIBUSB1_INCLUDE_DIRS})
	set(_libs ${LIBUSB1_LIBRARIES})
	target_include_directories(MD_Dumper PUBLIC ${LIBUSB1_INCLUDE_DIRS}})
	target_link_libraries (MD_Dumper ${LIBUSB1_LIBRARIES})
	include_directories(MD_Dumper ${LIBUSB1_INCLUDE_DIRS})
else ()
	# Sinon, nous affichons un message d'erreur
	message("libusb not found !")
endif ()

# Add SDL2 library
find_package(SDL2 REQUIRED)
target_link_libraries(MD_Dumper SDL2::Main)

# Add SDL2_image library
find_package(SDL2_image REQUIRED)
target_link_libraries(MD_Dumper SDL2::Image)
