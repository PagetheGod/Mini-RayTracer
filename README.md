# Mini Ray Tracer (C++)

This project is a personal **C++ ray tracing and rendering experiment** based on the book
*Ray Tracing in One Weekend* by Dr. Peter Shirley:

https://raytracing.github.io/books/RayTracingInOneWeekend.html

However, this project does differ from the original implementations in multiple aspects.

--

## What's Different?

- **Render Target**
  - Renders to a GUI window insted of a ppm image. This change was made because of personal preferences and my desire to practice using the WIN32 API.
- **Render Logic**
  - Uses multi-threaded rendering over the single-threaded rendering presented in the book's codes. This is done for performance, and to practice multi-threaded programming.
- **Code Structure** 
  - Removed the circular dependencies that are introduced by the book's codes.
---

## Features Implemented

- **Application Framework**
  - Basic GUI and window creation using WIN32
  - Frame presentation using Direct2D.

- **Ray Tracing & Rendering**
  - Implemented core ray tracing logic based on the book’s implementation. 
  - Integrated rendering logic into a standalone application rather than a console-only renderer.

- **Multithreading & Performance**
  - Added multithreaded rendering using a thread pool. (Thread pool implementation is based on https://github.com/progschj/ThreadPool/blob/master/ThreadPool.h. Credits to the author)
  - Reduced render time by up to **80%** compared to the single-threaded implementation in the book.

---

## Technical Focus

- C++ systems programming
- Ray tracing fundamentals and software rendering
- Multithreading and thread synchronization.
- Performance optimization
- Integrating rendering logic with OS GUI APIs

---

## Project Status

This project is **ongoing** and primarily intended as a learning and experimentation platform.

Planned future work includes:
- Further refactoring and cleanup of rendering architecture
- Experimenting with hardware rendering paths using DX11
- Potentially moving to modern APIs such as Vulkan and DX12
