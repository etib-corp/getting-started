# Application assets

Everything in this directory is copied next to the executable on every build,
keeping its sub-directories. Load a file by its path relative to this
directory:

```cpp
meshBuilder.makeMesh(parent, "models/chair.obj", "textures/wood.png");
imageBuilder.makeImage(parent, "images/photo.png");
```

The engine's own files (`shaders/`, `fonts/`, `textures/default_texture.png`)
are not stored here: the build copies them from the pinned libraries. A file
placed here under the same path replaces the library's one.
