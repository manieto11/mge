{ pkgs ? import <nixpkgs> {} }:

let
  crossPkgs = pkgs.pkgsCross.mingwW64;
in
crossPkgs.stdenv.mkDerivation {
  pname = "mge-windows";
  version = "0.1.0";

  src = ./.;
  
  nativeBuildInputs = with crossPkgs; [
    pkg-config
  ];

  buildInputs = with crossPkgs; [
    glfw
    vulkan-headers
    vulkan-loader
    # vulkan-validation-layers 
  ];

  makeFlags = [
    "BIN_EXT=.exe"
    "INS_DIR=$(out)" 
    "LIB_EXT=.dll"
  ];
}