# Interstate3D

**Interstate3D** is a small, modular C++ 3D application/engine focused on vehicle demos, physics, and rendering experiments. The project uses modern C++ (C++20), OpenGL (with optional Vulkan), Bullet physics, Assimp for asset import, and a lightweight stb image loader.

**Quick Highlights**
- **Language:** C++ (C++20)
- **Build:** CMake (>= 3.10)
- **Render:** OpenGL (Vulkan optional)
- **Physics:** Bullet
- **Assets:** Assimp + stb
- **Tests:** GoogleTest (integrated via CMake FetchContent)

**Project Layout**
- **Source:** [src](src/)
- **Shaders:** [shaders](shaders/)
- **Assets:** [assets](assets/)
- **Build files:** [CMakeLists.txt](CMakeLists.txt)
- **Demos / scripts:** demo_showcase.cpp, vehicle_3d_demo.py

**Requirements**
- **CMake:** 3.10 or newer
- **C++ compiler:** A C++20-capable compiler (MSVC, clang, or GCC)
- **Libraries:** OpenGL, GLFW (pkg-config glfw3), Bullet, Assimp, GLM (headers)
- **Python (optional):** For the Python preview scripts (`vehicle_preview.py` / `vehicle_3d_demo.py`) — Python 3.x

Notes: On systems without Vulkan installed, Vulkan support is detected but optional. The project includes a small `stb` static library under `external/stb`.

**Build (recommended, cross-platform)**
1. Create a build directory and configure with CMake.

Windows + Visual Studio (example):

```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

Linux / macOS (example):

```bash
mkdir -p build
cd build
cmake ..
cmake --build . -- -j$(nproc)
```

After a successful build the executable will be created in the build output (typically `Interstate3D` or `Interstate3D.exe`). CMake copies shader files into the build `shaders/` directory and the `assets/` directory into the build tree, so the executable can find them at runtime.

**Running**
- **Native executable:** Run the produced executable from the `build` directory so relative shader/asset paths resolve correctly:

```bash
./Interstate3D
# or on Windows
Interstate3D.exe
```

- **Python demos:** Run the preview/demo Python scripts directly. They may require additional Python packages depending on the environment.

```bash
python vehicle_preview.py
python vehicle_3d_demo.py
```

**Tests**
- Tests are located under [tests](tests/) and use GoogleTest. From the `build` directory you can run:

```bash
ctest --output-on-failure
```

or run the test executable directly (see the test CMake target name in the `tests` CMakeLists).

**Key Files & Notes**
- Build configuration: [CMakeLists.txt](CMakeLists.txt)
- Example demos: `demo_showcase.cpp`, `demo_showcase` project files
- Rendering backends: [src/rendering](src/rendering/)
- Physics and vehicle code: [src/physics](src/physics/), [src/game](src/game/)

**Development Tips**
- Ensure your system has the native development packages for GLFW, Bullet, and Assimp (platform package manager or build from source).
- If CMake cannot find GLM, point `CMAKE_INCLUDE_PATH` or install GLM system-wide.
- Enable verbose CMake output to troubleshoot missing pkg-config modules: `cmake .. -D CMAKE_VERBOSE_MAKEFILE=ON`.

**Contributing**
- Feel free to open issues or submit pull requests. Provide a short description, steps to reproduce (if applicable), and which platform/compiler you used.

**License**
- No license file is included. Add a `LICENSE` if you want to make the licensing explicit.

---

If you want, I can also:
- Add a minimal `README` badge list (build status, dependencies).
- Add a `CONTRIBUTING.md` or example CI workflow for automated builds and tests.
