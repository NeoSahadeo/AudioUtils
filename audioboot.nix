{ pkgs ? import <nixpkgs> {} }:
with pkgs;

stdenv.mkDerivation {
	pname = "audioboot";
	version = "1.1.0";

  src = ./.;

  nativeBuildInputs = [
    meson
    ninja
    pkg-config
  ];

  mesonBuildType = "release";

  configurePhase = ''
    meson setup build --prefix=$out
  '';

  buildPhase = ''
    ninja -C build
  '';

  installPhase = ''
    ninja -C build install
  '';
}
