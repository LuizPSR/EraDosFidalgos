{ pkgs ? import <nixpkgs> {} }:
let
  cross = pkgs.pkgsCross.mingwW64;
in
  with pkgs; mkShell {
    packages = with cross.buildPackages; [
      gcc
      pkg-config
      cmake
      binutils
    ];
#    shellHook = ''
#      export CXX=x86_64-w64-mingw32-g++
#      export CC=x86_64-w64-mingw32-gcc
#    '';
  }
