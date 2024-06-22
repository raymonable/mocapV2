# mocapV2
A program for creating Mocap (Motion Capture) data from SteamVR trackers (but built for Roblox).

## FAQ

### What does this program do?

- Allow you to list your SteamVR trackers
- Capture live tracker data
- Record tracker data at 60 frames per second

... all in one web API!

### What doesn't this program do?

- Variable frames per second
- Changing SteamVR settings / tracker aliases from the API
- Handle inverse kinematics (for solving where each bone should go in an armature)

### What use cases could this program have?

- Roblox (Studio)
- VRChat? (Probably not)

### How do I use it?

See the [documentation](docs) for more details!

## Build instructions

> :exclamation: There are pre-compiled builds in the Releases tab. You may need to install the [Visual Studio Runtime](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170) package.

1. Clone this Git repo
2. Open the solution in Visual Studio 2019+
3. Fix the includes and library paths (would love if this was applied to the actual repository)
4. Build for Release (x64)

## Problems / Unfinished features

- ~~`/trackers/live` has broken JSON (too lazy to fix and re-build)~~
- Includes and additional libraries paths are relative to my user profile (bad)
- No variable framerates
- No HTTPS support
