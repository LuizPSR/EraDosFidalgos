{ pkgs ? import <nixpkgs> {} }:
with pkgs; mkShell {
  packages = [
    cacert
    git
    sdl3
    sdl3-image
    glew
    cmake
    ninja
    gcc
  ];
}
