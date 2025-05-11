# Fluid Simulation Sandbox

This project is a partial 2D realtime particle-based fluid simulation sandbox implemented in C++. It simulates viscoelastic fluids based on the principles from the paper [Particle-based viscoelastic fluid simulation by Simon Clavet, Philippe Beaudoin, and Pierre Poulin](https://dl.acm.org/doi/10.1145/1073368.1073400).

## Features

*   Real-time 2D fluid simulation.
*   Viscoelastic fluid properties (springs between particles).
*   Interaction with rigid objects.
*   Adjustable simulation parameters.

## Building and Running

### Prerequisites

*   A C++ compiler supporting C++23.
*   CMake (version 3.28 or higher).

### Build Steps

1.  **Clone the repository and navigate to the project folder:**
    ```bash
    git clone https://gitlab.mff.cuni.cz/teaching/nprg041/2024-25/svoboda-cze-1220/prasilfi.git
    cd prasilfi/project
    ```

2.  **Install SFML dependencies:**
    ```
    sudo apt update
    sudo apt install \
    libxrandr-dev \
    libxcursor-dev \
    libxi-dev \
    libudev-dev \
    libfreetype-dev \
    libflac-dev \
    libvorbis-dev \
    libgl1-mesa-dev \
    libegl1-mesa-dev \
    libfreetype-dev
    ```

2.  **Configure the project:**
    ```
    cmake -B build -D CMAKE_BUILD_TYPE=Release
    ```

3.  **Build the project:**
    ```
    cmake --build build
    ```

### Running the Simulation
To run the simulation simply run:

```
./build/bin/fluid_simulation_sandbox
```

## Controls

The simulation can be controlled via mouse and keyboard. Control hins should be displayed on the right side of the window. You can adjust basically any simulation parameter from inside the window, to do so simply press the corresponding key combination.

Some parameters are pretty self explanatory, some are a little magic, you can read what their change usually does here (but some combinations are inherently unstable):

*   **`Simulation Speed`**: Overall speed multiplier for the simulation (If the FPS is too low, this parameter might be higher than the simulation speed actually is).
*   **`Gravity X/Y`**: Components of the gravitational force applied to particles.
*   **`Edge Bounciness`**: Coefficient of restitution when particles hit window boundaries (0 = no bounce, 1 = perfect bounce).
*   **`Interaction Radius`**: The maximum distance at which particles influence each other. This defines the neighborhood for density calculations, viscosity, and spring creation. A little magic, but mostly changes the distance of individual fluid particles (higher value = particles are more distant).
*   **`Rest Density`**: The target density the simulation tries to maintain for the fluid, also a bit magic (higher value = particles are closer).
*   **`Stiffness`**: Very magic, it makes the low/high density forces stronger, making the particles more strictly distanced (higher value = the fluid is less compressible and 'splashy').
*   **`Near Stiffness`**: Very magic, same as stiffness but for closer particle distances (higher value = the fluid is less compressible and 'splashy').
*   **`Linear Viscosit`**: Controls the fluid's resistance to flow, proportional to the relative velocity of neighboring particles (higher value = thicker fluid).
*   **`Quadratic Viscosity`**: Similar to linear viscosity, but the force is proportional to the square of the relative velocity. This can help dampen fast motions more strongly (higher value = thicker fluid).
*   **`Plasticity`**: Part of the viscoelastic model. Allows the fluid to exhibit plastic behavior (higher value = harder to deform the 'fluid').
*   **`Yield Ratio`**: Defines the elastic limit for springs (higher values = harder to 'permanently' deform the 'fluid').
*   **`Spring Stiffness`**: Controls the strength of the temporary springs formed between particles (harder to deform the 'fluid').
*   **`Control Radius`**: The radius around the mouse cursor used for adding/removing particles.
*   **`Particle Spawn Rate`**: Controls the number of particles spawned per time.
*   **`Object Radius`**: Default radius for newly created objects.
*   **`Object Mass`**: Default mass for newly created objects.
*   **`Base Particle Size`**: The visual size of particles when they are under no stress.
*   **`Particle Stress Size Multiplier`**: Influences how much a particle's visual size increases based on the stress it experiences.
*   **`Base Particle Color`**: The base color of particles.
*   **`Particle Stress Color Multiplier`**: Influences how much a particle's color shifts based on the stress it experiences.
