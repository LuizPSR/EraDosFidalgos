{ pkgs ? import <nixpkgs> {} }:
with pkgs; mkShell {
  packages = [
    cacert
    git
    glew
    cmake
    ninja
    gcc
    pkgconf
    alsa-lib
    libGL
    xorg.libXtst
    xorg.libX11
    xorg.libxcb
    xorg.libXScrnSaver
    xorg.libXcursor
    xorg.libXext
    xorg.libXfixes
    xorg.libXi
    xorg.libXrandr
  ];
}
