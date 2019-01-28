# MentalRay_Shaders
Various shaders designed for Mental Ray.

auxil/ utility files used throughout the code.
  miaux.h               auxiliary functions from the "Writing Mental Ray Shaders book".
  miaux.cpp   
  slh_aux.h             various utility functions, as well as code to compile with newer versions of Visual Studio.
  slh_aux.cpp
  slh_colors.h          functions and operators used to work with miColor.
  slh_vectors.h         functions and operators used to work with miVector.
  
pbrt/core/ a small subsection of the PBRT v3 renderer, modified work with Mental Ray.

pbrt_shaders/ shaders written using PBRT classes.
  slh_pbrt.h            functions designed to simplify interactions with PBRT code.
  slh_pbrt.cpp
  slh_pbrt_glass.cpp    PBRT glass shader.
  slh_pbrt_metal.cpp    PBRT metal shader.
  slh_pbrt_plastic.cpp  PBRT plastic shader - or diffuse dielectric.
  
slh_alphaShade.cpp      shader that returns RGBA = {0,0,0,0}.
slh_dispersion.cpp      dispersion shader, specular dielectric reflection, varying ior per RGB channel.
slh_heightRamp.cpp      returns black to white ramp based on height.
slh_layer.cpp           utility shader - layer multiple shaders.
slh_lightPlate.cpp      flat color, has attributes for color, transparency, intensity and final gather intensity.
slh_mixers.cpp          various utility functions to blend between two attributes. primarily used to blend mia_material
