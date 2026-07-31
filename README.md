# Magicfour-Remake
The 3D remake of '[Magicfour](https://github.com/firstin0907/Magicfour)', a magic action role-playing game.

## Introduction
'Magicfour-Remake' is a game project developed with C++17 and DirectX 11.

## How To Build
### Dependencies
You may need the following dependencies:
- **Windows SDK**: Required for Direct3D rendering (installed with Visual Studio).
- **vcpkg**: Used to install required libraries.

This project uses vcpkg manifest mode via the included `vcpkg.json` file.
The following libraries are declared in the manifest:
- **[DirectXTex](https://github.com/microsoft/DirectXTex)**: Used for reading texture files.
- **[DirectXTK](https://github.com/microsoft/DirectXTK)**: DirectX Toolkit, required for audio playback.
- **[rapidxml](https://rapidxml.sourceforge.net/index.htm)**: XML parser, used for reading [resources.xml](https://github.com/firstin0907/Magicfour-Remake/blob/main/Magicfour%20Remake/data/resources.xml) file.

### Setup
1. Install Visual Studio 2022 (or later) with the Desktop development with C++ workload. Ensure the Windows SDK is installed.
2. Install vcpkg and add the vcpkg installation directory to your `PATH` and `VCPKG_ROOT` environment variable.
3. Ensure Visual Studio is integrated with vcpkg:
```bash
vcpkg integrate install
```
4. Open the `Magicfour Remake.sln` solution file in Visual Studio 2022 or later and build the project.

## How To Play
The objective of the game is to survive as long as possible while collecting magical gems.  
You can cast skills depending on the last gem collected. You lose the last gem when a monster hits the magician. Without any gem, the game is over if a monster hits the magician.

### Controls
- **Z**: Cast a skill using the last gem the magician obtained.
- **X**: Drop the last skill the magician acquired.
- **Up Arrow**: Jump. You can perform a double jump by pressing up arrow again before landing.
- **Down Arrow**: Move down if the magician is not on the lowest ground.
- **Left/Right Arrow**: Walk or run. Re-press the left or right arrow key right after stopping walking will make the magician run.

## How was it made?
This section describes the core architecture and implementation details of this project.

### Game Object
The game object interface, `IGameObject` class, represents the interactable game object in the game, such as charaters, monster and skill objects.
The main game logic class, `ApplicationClass`, holds all game object data as pointers of `IGameObject` and calls its methods iterating over them in every frame.

Each game object derived `IGameObject` implements the following methods:
- `void FrameMove(...)`: Updates the object's position for one frame.
- `bool Frame(...)`: Processes the game logic for one frame and returns whether the object is still alive.
  If it returns `false`, `ApplicationClass` removes the object from the game object list.
- `rect_t GetGlobalRange()`: Returns the object's global bounding range.
  `ApplicationClass` uses this to perform collision checks between objects.
- `bool IsCollidable()`: Returns whether the object can currently collide.
  For example, a character becomes temporarily non-collidable after taking damage to prevent continuous collision damage.
- `void Draw(...)`: Renders the object and its additional visual effects. Only pushes render commands to the render queues, not makes actual drawcall.

`FrameMove(...)` is called for all objects first, followed by collision detection.
After that, `Frame(...)` is executed to process each object's logic.

Most game objects also inherit from the `Stateful<...>` class, which manages their internal state.
This state is used consistently across the system, such as for collision checks and determining animation poses.

### Resource Manager
All resources such as 3D models, sounds, and bitmaps are managed in a unified way by the Resource Manager.
The Resource Manager reads the `data/resources.xml` file, which contains metadata for each resource in XML format.

Each resource element has a unique string-based `name` property.
Using this name, resources can be easily referenced, added, or modified in the game logic without recompiling the source code.

### Smooth Animation
The character animations are based on `.bvh` format animation file.
Because the character pose is determined based on the `state_` field of the character instance,
the character animation can be seemed very flickery when its state swtiches.

To address this issue, the final pose is computed by interpolating between the pose of the previous state and the current state.

In `AnimatedObjectClass`, transformation matrices are decomposed into components.
The rotation components are interpolated using spherical linear interpolation (SLERP), and the final transformation matrix is reconstructed by composing the interpolated components.

### Rendering

Rendering logic is encapsulated in shader-specific classes, each responsible for its own rendering behavior.
Every shader maintains its own render queue and performs batched rendering.

During the frame update, the `Draw(...)` method of each `IGameObject` pushes its render commands to the appropriate shader by calling `PushRenderQueue(...)`.

After all game objects have been processed, `ApplicationClass::Render()` invokes `RenderShader(...)` on each shader instance.
Each shader then submits the collected vertex data to the DirectX rendering context and issues the corresponding draw calls.

Internally, render commands are grouped by shared models to reduce buffer updates and minimize state change overhead.
