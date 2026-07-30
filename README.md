# Audio Utils

A set of audio utilities.

## Audioboot Carla-Jack

Required:
- pipewire
- pipewire-jack
- pipewire-pulse (pactl)

This is a cli program that will start up a default sink and source. It will then boot up a 
`carla-jack-multi` config file.

```bash
audioboot -a --conf /home/neosahadeo/.audio/multiConf.carxp
```

It will automatically restart Carla if it crashes. Runs on a background thread to consume
practically 0 resources.

### NixOS Install

```nix
let
  audioboot-src = pkgs.fetchFromGitHub {
    owner = "NeoSahadeo";
    repo  = "AudioUtils";
    rev   = "main";
    sha256 = "sha256-2O39hGVDH5POV3eOsKYwa4DJS0Qvle7WAubeiSsG7Dg="; # This will probably change, it will generate the correct one for you
  };

  audioboot = pkgs.callPackage (audioboot-src + "/audioboot.nix") {};
in

  home.packages = with pkgs; [
#...
    audioboot
#...
]

```

### Build from sauce

```bash
meson setup build
cd build
meson compile
```

## Deadsec

Not related to the hacker org, this is a small utility to update pipewire thingies like
samplerate, links, and search nodes.
