## 🚨 WIP | Read-Only Disclaimer 🚨
> **Warning**
> At the moment, this project is in **"look, but don't touch"** mode. Attempting to clone and build this repository right now is guaranteed to trigger an existential crisis in your compiler due to the absence of external private dependencies `snassert` and `cstrs` on GitHub. Building is strictly reserved for the chosen one (me).

# WIP | snvoxeng

Custom high-performance Vulkan-based voxel execution and rendering engine built from scratch.

![Current Render View](docs/media/render_record.gif)
*Current state: Analytic Ray Tracing with Perfect Specular Reflections, Procedural Atmospheric Skybox & Tonemapping (`2/pi * atan(x)` with gamma correction) implemented via Vulkan Compute pipeline.*

---

## Roadmap & Current Progress

The up-to-date and detailed project roadmap, including all design specifics, is always available on our Discord server:  
📢 **[Join our Discord](https://discord.gg/9HsRDdFBFV)**

### 📍 Current Stage: **Milestone 3: Analytical Ray Marching (Math Foundations)**
* [x] Passing camera parameters to the compute shader via Push Constants
* [x] Iterative Ray Tracing loop with multi-bounce reflections
* [x] Custom analytical intersection testing for Spheres and AABBs (Cubes)
* [x] Dynamic atmospheric skybox model (Rayleigh/Mie scattering & sun disc simulation)
* [x] Custom Tonemapping and Gamma Correction Pipeline
* [/] Integrating VMA into the project
* [ ] Passing camera parameters to the compute shader via UBO.
* [ ] **Milestone Result:** A smooth 3D sphere rendered on screen that can be fully navigated with a free camera.

---

## 🛠️ Next Steps
* Finalize VMA pipeline integration for buffers/images.
* Implement basic abstractions and tools for UBO management.
