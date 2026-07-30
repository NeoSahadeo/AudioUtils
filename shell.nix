with import <nixpkgs> {};

mkShell {
  name = "env";
  buildInputs = with pkgs; [
    gcc
    gnumake
    pkg-config
    meson
    ninja
    fish
  ];
  shellHook = 
  ''
  export CLANGD_FLAGS="--query-driver=$(which $CC)"
  ${fish}/bin/fish
  '';
}
