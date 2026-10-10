<h1 align="center">LuciferClient</h1>

<p align="center">
  LuciferClient is a custom launcher for Minecraft that allows you to easily manage multiple installations of Minecraft at once.<br />
  <br />This is a <b>fork</b> of <a href="https://prismlauncher.org">Prism Launcher</a> and is <b>not</b> endorsed by or affiliated with the Prism Launcher project.
</p>

## Installation

- Builds are published on the [Releases](https://github.com/luciferstudios/LuciferClientv1/releases) page.
- The latest build status can be found in the [GitHub Actions](https://github.com/luciferstudios/LuciferClientv1/actions) tab.

### Development Builds

Development builds are not intended for most users. There may be bugs and other instabilities. You have been warned.

Development builds are produced by [GitHub Actions](https://github.com/luciferstudios/LuciferClientv1/actions) for **Linux**, **Windows** and **macOS**. They include debug information, so their file sizes are relatively large.

## Community & Support

Found a bug or want to suggest a feature? [Open an issue](https://github.com/luciferstudios/LuciferClientv1/issues).

Please do **not** report LuciferClient bugs to the Prism Launcher project.

## Building

LuciferClient builds the same way as Prism Launcher (C++23, Qt 6, CMake). Clone the repository **with submodules**:

```sh
git clone --recursive https://github.com/luciferstudios/LuciferClientv1.git
# or, in an existing checkout:
git submodule update --init --recursive
```

Then follow Prism Launcher's [build instructions](https://prismlauncher.org/wiki/development/build-instructions). The CMake presets in [CMakePresets.json](CMakePresets.json) cover Linux, macOS and Windows (MSVC / MinGW).

The resulting binary is called `luciferclient`. Its data is stored in a `LuciferClient` folder, separate from any Prism Launcher installation.

### Branding

All branding (name, app ID, binary name, URLs) is configured in:

- [`program_info/CMakeLists.txt`](program_info/CMakeLists.txt): display name, app ID, environment variable prefix, copyright
- [`CMakeLists.txt`](CMakeLists.txt): binary name, updater repository, bug tracker, news/help/community URLs and API keys

## API keys and third-party terms

This fork still inherits some service configuration from Prism Launcher in [CMakeLists.txt](CMakeLists.txt) (Microsoft login client ID, CurseForge API key, Imgur client ID, metadata server, news/wiki URLs). Before distributing builds, replace the API keys with your own or set them to empty strings (`""`) to disable the related features.

Note that if you build this software without removing the provided API keys you are accepting the following terms and conditions:

- [Microsoft Identity Platform Terms of Use](https://docs.microsoft.com/en-us/legal/microsoft-identity-platform/terms-of-use)
- [CurseForge 3rd Party API Terms and Conditions](https://support.curseforge.com/en/support/solutions/articles/9000207405-curse-forge-3rd-party-api-terms-and-conditions)

If you are packaging LuciferClient for a distribution, set `Launcher_BUILD_PLATFORM` to a slug representing your distribution.

## Credits

LuciferClient is built on the work of the [Prism Launcher](https://github.com/PrismLauncher/PrismLauncher), [PolyMC](https://github.com/PolyMC/PolyMC) and [MultiMC](https://github.com/MultiMC/Launcher) contributors.

## License [![License](https://img.shields.io/github/license/luciferstudios/LuciferClientv1?label=License&logo=gnu&color=C4282D)](LICENSE)

All launcher code is available under the GPL-3.0-only license. See [LICENSE](LICENSE) and [COPYING.md](COPYING.md).

The logo and related assets are under the CC BY-SA 4.0 license (see [program_info/LICENSE](program_info/LICENSE)).
