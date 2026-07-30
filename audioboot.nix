with import <nixpkgs> {};

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
