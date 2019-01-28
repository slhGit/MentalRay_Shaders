# MentalRay_Shaders
Various shaders designed for Mental Ray.
.
* [auxil/](./auxil) utility files used throughout the code.
  * [miaux.h](./auxil/miaux.h) - auxiliary functions from the "Writing Mental Ray Shaders book".
  * [miaux.cpp](./auxil/miaux.cpp)   
  * [slh_aux.h](./auxil/slh_aux.h) - various utility functions, as well as code to compile with newer versions of Visual Studio.
  * [slh_aux.cpp](./auxil/slh_aux.cpp)
  * [slh_colors.h](./auxil/slh_colors.h) - functions and operators used to work with miColor.  
  * [slh_vectors.h](./auxil/slh_vectors.h) - functions and operators used to work with miVector.
  
* [pbrt/core](./pbrt/core) a small subsection of the PBRT v3 renderer, modified work with Mental Ray.

* [pbrt_shaders](./pbrt_shaders) shaders written using PBRT classes.
  * [slh_pbrt.h](./pbrt_shaders/slh_pbrt.h) - functions designed to simplify interactions with PBRT code.
  * [slh_pbrt.cpp](./pbrt_shaders/slh_pbrt.cpp)
  * [slh_pbrt_glass.cpp](./pbrt_shaders/slh_pbrt_glass.cpp) - PBRT glass shader.
  * [slh_pbrt_metal.cpp](./pbrt_shaders/slh_pbrt_metal.cpp) - PBRT metal shader.
  * [slh_pbrt_plastic.cpp](./pbrt_shaders/slh_pbrt_plastic.cpp) - PBRT plastic shader.

* [slh_alphaShade.cpp](./slh_alphaShade.cpp) - shader that returns RGBA = {0,0,0,0}.
* [slh_dispersion.cpp](./slh_dispersion.cpp) - dispersion shader, specular dielectric reflection, varying ior per RGB channel.
* [slh_heightRamp.cpp](./slh_heightRamp.cpp) - returns black to white ramp based on height.
* [slh_layer.cpp](./slh_layer.cpp) - utility shader - layer multiple shaders.
* [slh_lightPlate.cpp](./slh_lightPlate.cpp) - flat color, has attributes for color, transparency, intensity and final gather intensity.
* [slh_mixers.cpp](./slh_mixers.cpp) - various utility functions to blend between two attributes. primarily used to blend mia_material
