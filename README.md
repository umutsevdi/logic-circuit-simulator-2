# Logic Circuit Simulator 2

A free and open-source cross-platform Logic Circuit Simulator rewritten in C++. 

> [!WARNING]  
> Currently in progress. Visit the original Logic Circuit Simulator from
> [here](https://github.com/umutsevdi/Logic-Circuit-Simulator).

https://github.com/user-attachments/assets/a770f99d-ed13-40cf-bca7-54b13cea11b8

## Building & Running

### Windows
1. Install build dependencies.
```bat
    vckpg.exe install
```
2. Compile project using Visual Studio.
3. Run the application for the first time. It will initialize the folder
structure and then crash.
4. Copy the `misc/` folder `$HOME/.local/share/LogicCircuitSimulator/misc`.
Rerun the application.

### Linux
1. Install the following dependencies:

|Library|Package Name (Fedora)| Package Name (Debian Based)|
|---------|-------|---|
|curl     |libcurl-devel|libcurl4-openssl-dev|
|glfw     |glfw-devel|libglfw3-dev|
|OpenGL   |mesa-libGL-devel|libgl1-mesa-dev|
|libsecret|libsecret-devel|libsecret-1-dev|

2. CMake Installation
```sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make 
```
3. Run the application for the first time. It will initialize the folder
structure and then crash.
4. Copy the `misc/` folder `$HOME/.local/share/LogicCircuitSimulator/misc`.
Rerun the application.
