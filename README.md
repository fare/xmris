# Mr Is 4.1.2 in 2026

Mr Is (`xmris`, 1992-1995) by Nathan Sidwell is a remarkably faithful Unix clone of
Mr. Do!, a 1982 game for 8-bit consoles by Kazutoshi Ueda:

  - [Wikipedia page](https://en.wikipedia.org/wiki/Mr._Do!)
  - [Playable SNES version](https://www.retrogames.cz/play_273-SNES.php)
  - [2026 HAMSTER reissue for Nintendo Switch 2 / Nintendo Switch / PlayStation 5 / PlayStation 4 / XBOX Series X|S](https://www.arcadearchives.com/en/title/aca-410/)
  - [Demo video of gameplay](https://www.youtube.com/watch?v=hv2B-p1p8OQ)
  - [Original Z80 arcade ROMs for MAME](https://www.planetemu.net/en/rom/mame-roms-merged/mrdo)

I (Faré Rideau) am amazed at how much patience Nathan had in precisely
re-creating the behavior of the original game, and
I wonder whether he reverse-engineered the code or
just inferred the rules by observation: the game has plenty of fun
corner cases that interact very well with each other,
many not obvious to the beginner (shooting through walls,
dying and gaining a life at the same time, etc.),
that Nathan perfectly reproduced.

2026 revival and SDL port maintained by François-René (Faré) Rideau.

## Building and running Mr Is

Choose a frontend: SDL3 or classic X11.

### Building with SDL3

#### SDL3 Build

For the SDL3 game on Ubuntu, install its build dependencies:

```sh
sudo apt install build-essential pkg-config libsdl3-dev
make -f Makefile.sdl
./xmris-sdl -fullscreen
```

#### SDL3 Test Run
```sh
./xmris-sdl
```

#### SDL3 Install
To install in `/usr/local`, the default, you would:
```sh
sudo make -f Makefile.sdl install install.man
```

Or you can override the installation `PREFIX` as follows,
assuming your `PATH` and `MANPATH` are prepared to deal with it:
```sh
make PREFIX=$HOME/.local -f Makefile.sdl install install.man
```

#### SDL3 Run
```sh
man 6 xmris-sdl
xmris-sdl
```

#### SDL3 on Linux

The SDL frontend builds from the same game, garden, score, and sound sources.
F11 toggles fullscreen. The SDL window centers the garden and scales it in
whole pixels by default. Use `-scale fit` to fill as much of the window as
possible at the original aspect ratio, including in fullscreen.
Use `-scale 2` to request a 2× initial window.
Arrow keys move, Space starts or throws, and K on the title screen opens Options.
Run `./xmris-sdl -help` for the supported options.
The SDL frontend uses the bundled 18-pixel X11 Courier bitmap font and command-line settings.
The X11 game and the xmred garden editor use the existing imake build and X resources.

The Linux executables also contain the digits, alphabet, puzzle and xmred
gardens. Use e.g. `-gardens alphabet` to select one. External garden files
retain priority, including explicitly supplied custom gardens.

### Android game

The debug APK supports ARM64 phones and x86-64 Android emulators. Extract
`xmris-android.apk` from the Android workflow artifact. With USB debugging
enabled and the phone authorized, install and launch from Linux:

```sh
adb devices
adb install -r xmris-android.apk
adb shell am start -n org.xmris.game/org.libsdl.app.SDLActivity
```

On an ARM64 Linux laptop, run the APK on a connected phone and use
`scrcpy` to see and control it on the laptop. On a computer with a running
Android Virtual Device, use `adb -e install -r xmris-android.apk` and
`adb -e shell am start -n org.xmris.game/org.libsdl.app.SDLActivity`.
Use `adb -s SERIAL` if several devices are connected.

Tap **Start**. Swipe anywhere on the game screen to steer. Hold still to
keep moving, change swipe direction to turn, and lift to stop. A near-diagonal swipe forward keeps your direction. A near-diagonal swipe
backward takes the perpendicular turn (moving right, swipe up-left to turn up). A second
swipe takes over even while the first thumb is down. At a screen edge,
rest your other thumb near the center, then lift the first to carry the
current direction over to the new thumb. Tap the Throw strip
to fire with the other thumb. Swiping across or from a button still steers.
Pause has its own row. A fresh steering touch, throw, or keyboard press
resumes play, except for Pause itself. To end the current run by touch, tap Pause, then End game, then Confirm
end game. Cancel keeps the game paused. The configured quit key also ends
the run while paused.
The Options screen has touch controls for Up, Down, Select and Back.
Scores and preferences live in the app's private storage.

To build on an x86-64 Linux SDK host, install JDK 17, Android SDK Platform 35,
Build Tools 35.0.0, NDK 28.2.13676358, SDK CMake 3.22.1, zip, make and a C
compiler. Set ANDROID_HOME and run `bash android/build.sh`. The APK is in
`build/android/`. To update in place, use the same debug signing key.

### SDL joystick controls

Connect a controller before launch or during play. Use the D-pad or left
stick to steer and the bottom face button to start, throw, or select.
Start opens Options on the title screen, pauses during play, and goes back
in a menu. Press the throw button to resume. For an unmapped joystick,
use its first two axes or first hat, first button to throw and second to pause.
These controls are shared by the Android and desktop SDL builds.

### macOS and iOS

On a Mac with Xcode and CMake, generate the sprite headers using the host
compiler, then build the macOS app:

```sh
make -f Makefile.sdl CC=clang bitmaps/sprites
cmake -S apple -B build/mac -G Xcode
cmake --build build/mac --config Release --target xmris
open build/mac/Release/xmris.app
```

For iOS, configure a separate build directory and open its Xcode project:

```sh
cmake -S apple -B build/ios -G Xcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphonesimulator -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0
open build/ios/xmris.xcodeproj
```

Select the xmris scheme and an iPhone simulator, then Run. For a physical
iPhone, configure another directory with `-DCMAKE_OSX_SYSROOT=iphoneos`
and select your signing team in Xcode. iOS uses the same touch controls
as Android. Both Apple targets support SDL controllers.

The **Apple SDL** workflow builds macOS, iOS Simulator, and unsigned iOS
device app bundles. The simulator job installs and launches the game.
These are development artifacts. The device bundle needs provisioning and
signing for installation, and the macOS artifact is not notarized.

### Windows SDL3 game

Extract the Windows zip for your processor and run `xmris.exe` from the
extracted folder. The executable contains SDL and all four garden sets.
Arrow keys move, Space starts or throws, and F11 toggles fullscreen.
Preferences and personal scores are saved in the user's SDL preference
directory (normally `%APPDATA%\xmris`), in `config` and `score`.

To cross-compile on Linux, install CMake, make and a native C compiler for the
sprite generator. Put the bin directory of the UCRT edition of LLVM-MinGW
on PATH and unpack the SDL3 3.4.20 source archive. Set SDL_DIR to that
source directory, for example:

```sh
make -f Makefile.windows ARCH=aarch64 SDL_DIR=/opt/SDL3-3.4.20
make -f Makefile.windows ARCH=x86_64 SDL_DIR=/opt/SDL3-3.4.20
```

Each build lands in `build/windows-$ARCH/`. On ARM64 Linux with native
Wine, run `wine build/windows-aarch64/xmris.exe -scale fit`.

_NB_: On my Linux box using stumpwm, I couldn't get keyboard focus
onto the xmris window unless I started with the following command, and
even then, I had to switch away from the window and back to it to get focus:
```sh
wine explorer /desktop=XmrisPanel,1920x1200 ./xmris.exe -fullscreen -scale auto
```


### Building with X11

#### X11 Prerequisites

On Ubuntu, I needed to install `xmkmf` from xutils, the Athena widgets,
and the standard 75dpi bitmap fonts.
The X11 game uses XRender for scaling and SDL2 for sound:
```
sudo apt install build-essential xutils-dev libxaw7-dev xfonts-75dpi libxrender-dev libsdl2-dev
```

If you want the fonts to work in the same X11 session, you might additionally need to:
```
xset fp+ /usr/share/fonts/X11/75dpi
xset fp rehash
```

The rpmfusion docs suggest you also need to install the 100dpi bitmap fonts
(debian package `xfonts-100dpi`), but in my experience they are not necessary to run the game.

#### X11 Build
```sh
xmkmf
make
```

#### X11 Test Run
```sh
./xmris
```

#### X11 Install
```sh
make install
make install.man
```

#### X11 Run
```sh
man 6 xmris
xmris
```

### Personal gardens

Both games look for named gardens in `$XDG_DATA_HOME/xmris`, or
`~/.local/share/xmris` when that variable is unset, empty, or relative.
For example, save `mygarden.gdn` there and run `xmris-sdl -gardens mygarden`.
A file in the current directory (or beside the including garden) takes
precedence, followed by the personal directory and then the frontend's
resource locations.

### Game options

Press **F1** at the title screen in either frontend.
The original **K** shortcut also works until rebound.
The menus always use the arrow keys, Enter, and Esc.
You can toggle sound effects and background music independently,
choose Easy, Hard, or Classic diamond behavior,
or open Controls to review your bindings.
Select an action and press Enter to change just that key.
Restore defaults resets the working copy.
Save and return applies your edits, while Esc discards them.
Esc returns to the title.
Changes are saved under `$XDG_CONFIG_HOME/xmris/config`,
which defaults to `~/.config/xmris/config`.
Saved controls override X key resources,
including bindings supplied with `-xrm`.
For each action without a valid saved binding,
X11 uses its usual resource lookup and built-in default.
For other preferences, command-line settings and explicit X resources
take precedence at startup.
The `-nosound` switch disables the audio device for the whole session.

### Linux score files

Both frontends keep personal scores in `~/.xmris.score`. For a shared
leaderboard with a setgid installation, the administrator must pre-create
`/var/games/xmris.score` as a regular file writable by the game group.
Keep the containing directory under administrator control. The game opens
the shared file before dropping privileges and can use personal scores
when the shared file is unavailable.

## Features added 2026

In 2026, I used AIs to add these features that were missing in 1992.

### Display: Scaling and Centering

The game now automatically uses the largest whole-number pixel scale
that fits its window, and stays centered as you resize.
You can use an X resource `Xmris.scale: 3` or `Xmris.scale: auto`
to confirm or override the default (`auto`).
You can use the command-line option `-scale 2` to lock it at 2×,
or `-scale auto`, to override the default from the X resource.
This feature was not needed in 1992, when a 436×539 window
(including 1 pixel border each side) was actually bigger
than most people's displays used to be.

The SDL variant can also scale to the whole screen, though
I personally prefer an integer scale.

The game will also center its display within the window if the user
or window manager oversizes it. This was also not a problem in 1992,
but it is one for me using stumpwm in 2026.

### Sound Effects

The original X version lacked the classic Mr Do! sound
(do re mi fa sol la si do) as you eat cherries,
as well as the melodies that accompanied the game and various events.
I suppose sound was not generally available on Unix in 1992.
I added game audio, and it is enabled by default.
Set `Xmris.sound: false` in your X resources for quiet play,
or run `xmris -nosound`.
Use `-sound` to enable audio for a single session.
For effects and fanfares without the looping soundtrack,
use `-nomusic` or `Xmris.music: false`.

The chase music speeds up by 50% when the nearest enemy's distance falls to
80 arcade pixels or less, and slows down again at 113 or more (16 pixels per
cell). Between those thresholds it keeps its tempo. xmris measures tunnel
distance for ordinary monsters and direct separation for digging monsters.

### Game Logic

`Xmris.diamondDifficulty` controls the chance that an eligible split apple
reveals a diamond. The default, `easy`, gives a 1 in 512 chance. Use `hard`
for 1 in 2048. These settings reproduce the two possible arcade settings
of Mr. Do!, and both allow another diamond after collecting one.
The historical `classic` setting of 1992 xmris gives a 1 in 256 chance
but allows only one collection per game.

In the arcade, the ball starts nine pixels ahead of the player, so a throw
can clear a thin wall. xmris supports the same trick, but checks how far the
hedge has been dug and can refuse a shot the arcade would launch.

I also restored some behavior from the arcade game: there is a .4 second
grace period after eating a cherry to continue a streak, and the last
cherry in the streak is 500 points total, not 550 (with 500 plus regular 50).

Apart from that, it looks like Nathan had very precisely re-created the game.

## Provenance

This repository was initialized with the contents of
[the xmris 4.0.5 tarball (2008) from sourceforge](https://sourceforge.net/projects/xmris/files/xmris/4.05/),
plus [patches from rpmfusion](https://github.com/rpmfusion/xmris/)
by Hans de Goede <hdegoede@redhat.com> (2010-2011) (also include a rpm spec and icons).
Some historic tarballs and USENET posts were used to record earlier history.

## Troubleshooting

### Missing the 75dpi X fonts
```
X Error of failed request:  BadName (named color or font does not exist)
  Major opcode of failed request:  45 (X_OpenFont)
  Serial number of failed request:  10
  Current serial number in output stream:  11
Maybe we need to install additional X packages on the server side?
```

Solution: install and configure the good old fonts.

## TODO

If you want to make this game better, have an AI add the following features:
* [ ] Porting to other platforms beside X11.
* [ ] Running in a TEE from a deterministic build so as to maintain
      a cheat-resistant global high score chart
* [ ] Have an AI play the game and beat it.

## Game Strategy

Do you understand what triggers the letters to go out?
What are the (somewhat) harder letters to collect?

_Challenge_: Can you collect all the EXTRA letters right in garden 1
without losing a life? It isn't easy, but it's possible!
