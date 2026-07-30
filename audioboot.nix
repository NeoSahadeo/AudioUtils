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
    meson setup $out
  '';

  buildPhase = ''
    ninja -C $out
  '';

  installPhase = ''
    ninja -C $out install
  '';
}
