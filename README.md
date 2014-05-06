surface2volume
==============

[![Build Status](https://travis-ci.org/thorbenk/surface2volume.svg?branch=master)](https://travis-ci.org/thorbenk/surface2volume)

`surface2volume` takes a set of object meshes (in `.obj` format) as input
and renders the entire scene into a voxel grid. Each object is assigned
its own unique label.

Preferrably, the object meshes should be watertight. However, sometimes
some problematic areas may be non-manifold. In order to still produce 
a reasonable segmentation, the algorithm proceeds as follows:

- Build a BVH tree for each object's triangles.  
  The BVH tree used is [Fast-BVH](https://github.com/brandonpelfrey/Fast-BVH).
- For each voxel in the (x,y) plane, shoot a ray in the `z` direction.
  If it intersects a mesh, change current label color and mark as inside.
  If the mesh is left again, change label color to _background_ until
  hitting the next mesh (or leaving the volume)
- For robustness, repeat this process by shooting rays in `x` and `y`
  direction as well.
- For each voxel, take the majority vote on the voxel's label assignment
  from the `x`, `y` and `z` rays.

