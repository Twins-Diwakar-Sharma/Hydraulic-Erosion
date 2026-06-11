{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {

  packages = with pkgs; [
    cmake
    gcc
    glfw
    wayland
  ];

  inputsFrom = with pkgs; [
    glfw
    wayland
  ];

  LD_LIBRARY_PATH = "${pkgs.lib.makeLibraryPath [pkgs.wayland pkgs.glfw]}";
}
