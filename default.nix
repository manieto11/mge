with import <nixpkgs> {};

pkgs.stdenv.mkDerivation {
  pname = "mge";
  version = "0.1.0";

  src = ./.;

  nativeBuildInputs = with pkgs; [
    pkg-config
  ];

  buildInputs = with pkgs; [
    glfw
    raylib
    vulkan-headers
    vulkan-loader
    vulkan-validation-layers
  ];

  installFlags = [ "INS_DIR=$(out)" ];
}