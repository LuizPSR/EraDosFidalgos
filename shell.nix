{ pkgs ? import <nixpkgs> {} }:
with pkgs; mkShell {
  packages = [
    # gdb
    # stb
    sdl3
    sdl3-image
    glew
    # glm
    # (callPackage ./imgui.nix {})
    pkg-config
    cmake
  ];
}
