{ pkgs ? import <nixpkgs> {} }:
with pkgs;

stdenv.mkDerivation {
	pname = "audioboot";
	version = "1.1.0";

  src = ./.;

  buildInputs = with pkgs; [
    gcc
    gnumake
    meson
    ninja
  ];
}
