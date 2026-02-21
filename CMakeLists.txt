cmake_minimum_required(VERSION 4.2.3)
project(MiniRayTracer DESCRIPTION "CMake build for a mini ray tracer using Win32 and DX11")


add_executable(${PROJECT_NAME} WIN32)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_include_directories(${PROJECT_NAME} PRIVATE
	Resources/
)

target_sources(${PROJECT_NAME} 
    PRIVATE
	src/main.cxx
	src/Private/Application.cxx
	src/Private/Camera.cxx
	src/Private/Color.cxx
	src/Private/ComputeShaderManager.cxx
	src/Private/D2D1Class.cxx
	src/Private/D3D11Class.cxx
	src/Private/HardwareRenderer.cxx
	src/Private/Hittable.cxx
	src/Private/HittableList.cxx
	src/Private/Interval.cxx
	src/Private/Ray.cxx
	src/Private/SoftwareRenderer.cxx
	src/Private/Sphere.cxx
	src/Private/SubMaterials.cxx
	src/Private/ThreadPool.cxx
	src/Private/Timer.cxx
	src/Private/Vector3D.cxx
	src/Private/VMaterial.cxx

	Resources/Settings.rc
	Resources/app.manifest
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	d3d11.lib dxgi.lib dxguid.lib uuid.lib
	user32.lib d2d1.lib kernel32.lib
)
