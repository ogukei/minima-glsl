# minima-glsl
Minimal GLSL Playground for macOS, written in Pure C

![](https://gist.githubusercontent.com/ogukei/421e3accae1bb8768497749a72e0548b/raw/f39451e6e708b02a988e1afdff7bd49dbd53e5c2/minima-glsl.gif)

## Get Started
```
$ make && ./main
```

---

On Mac, sometimes we want a simple graphical application on the command-line, though 
building it with Xcode is a bit overkill. "Cocoa" is the de-facto standard framework to build a GUI application, but also it has a large amount of linkage and runtime overhead to provide convenience.
With this small set of 400~ lines of C code, you don't need neither Xcode nor Cocoa Framework for building. 

## Features
- Enables OpenGL3 shader programming without Xcode or Cocoa
- Supports realtime-rendering
- Supports transparent window background
- Blazing fast application startup
