# LuciferClient Nix Packaging

## Installing a development release (flake)

LuciferClient is not packaged in nixpkgs. You can build and install it straight from this repository's flake.

### Installing the package directly

After adding `github:luciferstudios/LuciferClientv1` to your flake inputs, you can access the flake's `packages` output.

Example:

```nix
{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

    luciferclient = {
      url = "github:luciferstudios/LuciferClientv1";

      # Optional: Override the nixpkgs input of luciferclient to use the same revision as the rest of your flake
      # Note that this may break the reproducibility mentioned above, and you might not be able to access the binary cache
      #
      # inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    { nixpkgs, luciferclient, ... }:
    {
      nixosConfigurations.foo = nixpkgs.lib.nixosSystem {
        modules = [
          ./configuration.nix

          (
            { pkgs, ... }:
            {
              environment.systemPackages = [ luciferclient.packages.${pkgs.system}.luciferclient ];
            }
          )
        ];
      };
    };
}
```

### Using the overlay

Alternatively, if you don't want to use our `packages` output, you can add our overlay to your nixpkgs instance.
This will ensure LuciferClient is built with your system's packages.

> [!NOTE]
> Depending on what revision of nixpkgs your system uses, this may result in binaries that differ from the above `packages` output.

Example:

```nix
{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

    luciferclient = {
      url = "github:luciferstudios/LuciferClientv1";

      # Optional: Override the nixpkgs input of luciferclient to use the same revision as the rest of your flake
      # Note that this may break the reproducibility mentioned above, and you might not be able to access the binary cache
      #
      # inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    { nixpkgs, luciferclient, ... }:
    {
      nixosConfigurations.foo = nixpkgs.lib.nixosSystem {
        modules = [
          ./configuration.nix

          (
            { pkgs, ... }:
            {
              nixpkgs.overlays = [ luciferclient.overlays.default ];

              environment.systemPackages = [ pkgs.luciferclient ];
            }
          )
        ];
      };
    };
}
```

### Installing the package ad-hoc (`nix shell`, `nix run`, etc.)

You can simply call the default package of this flake.

Example:

```shell
nix run github:luciferstudios/LuciferClientv1

nix shell github:luciferstudios/LuciferClientv1

nix profile install github:luciferstudios/LuciferClientv1
```

## Installing a development release (without flakes)

### Installing the package directly (`fetchTarball`)

We use flake-compat to allow using this Flake on a system that doesn't use flakes.

Example:

```nix
{ pkgs, ... }:
{
  environment.systemPackages = [
    (import (
      builtins.fetchTarball "https://github.com/luciferstudios/LuciferClientv1/archive/develop.tar.gz"
    )).packages.${pkgs.system}.luciferclient
  ];
}
```

### Using the overlay (`fetchTarball`)

Alternatively, if you don't want to use our `packages` output, you can add our overlay to your instance of nixpkgs.
This results in LuciferClient using your system's libraries

Example:

```nix
{ pkgs, ... }:
{
  nixpkgs.overlays = [
    (import (
      builtins.fetchTarball "https://github.com/luciferstudios/LuciferClientv1/archive/develop.tar.gz"
    )).overlays.default
  ];

  environment.systemPackages = [ pkgs.luciferclient ];
}
```

### Installing the package ad-hoc (`nix-env`)

You can add this repository as a channel and install its packages that way.

Example:

```shell
nix-channel --add https://github.com/luciferstudios/LuciferClientv1/archive/develop.tar.gz luciferclient

nix-channel --update luciferclient

nix-env -iA luciferclient.luciferclient
```

## Package variants

Both Nixpkgs and this repository offer the following packages:

- `luciferclient` - The preferred build, wrapped with everything necessary to run the launcher and Minecraft
- `luciferclient-unwrapped` - A minimal build that allows for advanced customization of the launcher's runtime environment

### Customizing wrapped packages

The wrapped package (`luciferclient`) offers some build parameters to further customize the launcher's environment.

The following parameters can be overridden:

- `additionalLibs` (default: `[ ]`) Additional libraries that will be added to `LD_LIBRARY_PATH`
- `additionalPrograms` (default: `[ ]`) Additional libraries that will be added to `PATH`
- `controllerSupport` (default: `isLinux`) Turn on/off support for controllers on Linux (macOS will always have this)
- `gamemodeSupport` (default: `isLinux`) Turn on/off support for [Feral GameMode](https://github.com/FeralInteractive/gamemode) on Linux
- `jdks` (default: `[ jdk21 jdk17 jdk8 ]`) Java runtimes added to `LUCIFERCLIENT_JAVA_PATHS` variable
- `msaClientID` (default: `null`, requires full rebuild!) Client ID used for Microsoft Authentication
- `textToSpeechSupport` (default: `isLinux`) Turn on/off support for text-to-speech on Linux (macOS will always have this)
