cmake_minimum_required(VERSION 3.0)
project(TapTools)


message(STATUS "Updating Git Submodules")
execute_process(
	COMMAND				git submodule update --init --recursive
	WORKING_DIRECTORY	"${PROJECT_SOURCE_DIR}"
)


add_subdirectory(source/tap.fourpole~)

