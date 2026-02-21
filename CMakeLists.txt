cmake_minimum_required(VERSION 4.2.3)
project(MiniRayTracer DESCRIPTION "CMake build for a mini ray tracer using Win32 and DX11")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



add_executable(${PROJECT_NAME} WIN32)

set_target_properties(${PROJECT_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/Binaries/Output/Debug
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/Binaries/Output/Release
)

target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)

target_include_directories(${PROJECT_NAME} PRIVATE
	Resources/
)

target_compile_options(${PROJECT_NAME} PRIVATE
	#MSVC
	$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/W4 /Od>
	$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/W1 /O2>
	#GCC
	$<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:Debug>>:-Wall -Wextra -O0>
	$<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:Release>>:-w -O3>
	#Clang
	$<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:Debug>>:-Wall -Wextra -O0>
	$<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:Release>>:-w -O3>
)

target_link_options(${PROJECT_NAME} PRIVATE
	 $<$<CXX_COMPILER_ID:GNU>:-municode>
)
add_custom_command(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/RayTraceShader.cso
	COMMAND fxc /T cs_5_0 /E main /Fo ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/RayTraceShader.cso ${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/RayTraceComputeShader.hlsl
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/Shaders/RayTraceComputeShader.hlsl
	COMMENT "Compiling HLSL compute shader"
)

target_sources(${PROJECT_NAME} 
    PRIVATE
	src/main.cpp
	src/Private/Application.cpp
	src/Private/Camera.cpp
	src/Private/Color.cpp
	src/Private/ComputeShaderManager.cpp
	src/Private/D2D1Class.cpp
	src/Private/D3D11Class.cpp
	src/Private/HardwareRenderer.cpp
	src/Private/Hittable.cpp
	src/Private/HittableList.cpp
	src/Private/Interval.cpp
	src/Private/Ray.cpp
	src/Private/SoftwareRenderer.cpp
	src/Private/Sphere.cpp
	src/Private/SubMaterials.cpp
	src/Private/ThreadPool.cpp
	src/Private/Timer.cpp
	src/Private/Vector3D.cpp
	src/Private/VMaterial.cpp

	Shaders/RayTraceShader.cso
	
	Resources/Settings.rc
	Resources/app.manifest
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	d3d11 dxgi dxguid uuid
	d3dcompiler user32 d2d1 kernel32
)
