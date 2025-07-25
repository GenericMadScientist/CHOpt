# CHOpt

A program to generate optimal Star Power paths for Clone Hero.

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/3e82e23473fc48779a486d9099d52e21)](https://app.codacy.com/gh/GenericMadScientist/CHOpt/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)

## Install

Download the latest version from the [Releases page](../../releases). If you're
on Windows, you will need to have installed the latest
[Visual Studio Redistributable](https://aka.ms/vs/17/release/vc_redist.x64.exe).

## Usage

If you are unfamiliar with the intricacies of reading paths, read
[this guide](misc/How-to-read-paths.md).

CHOpt has two versions, a command-line version and a graphical version. The
graphical version has the same options as the command-line version and is
self-explanatory enough. As for the command-line version, an example usage to
path Trogdor on Hard and save the output to trogdor_path.png is

```bat
> CHOpt.exe -f trogdor.chart -d hard -o trogdor_path.png
```

Only the -f parameter is required, the difficulty defaults to Expert and the
path is by default saved to path.png. The full list of arguments can be found
by passing -h or --help to CHOpt, or by consulting the table below.

| Arguments               | Action                                                                        |
| ----------------------- | ----------------------------------------------------------------------------- |
| -h, --help              | List optional arguments                                                       |
| -f, --file              | Chart filename                                                                |
| -o, --output            | Filename of output image (.bmp or .png)                                       |
| -d, --diff              | Difficulty (easy/medium/hard/expert)                                          |
| -i, --instrument        | Instrument (guitar/coop/bass/rhythm/keys/ghl/ghlbass/drums/proguitar/probass) |
| --sqz, --squeeze        | Set squeeze %                                                                 |
| --ew, --early-whammy    | Set early whammy %                                                            |
| --lazy, --lazy-whammy   | Set number of ms of whammy lost per sustain                                   |
| --delay, --whammy-delay | Amount of ms after each activation before whammy can be obtained              |
| --lag, --video-lag      | Video calibration, in ms                                                      |
| -s, --speed             | Set speed the song is played at                                               |
| -l, --lefty-flip        | Draw with lefty flip                                                          |
| --no-double-kick        | Disable 2x kick (drums only)                                                  |
| --no-kick               | Disable non-2x kicks (drums only)                                             |
| --no-pro-drums          | Disable pro drums (drums only)                                                |
| --enable-dynamics       | Enables double points from ghost and accent notes (drums only)                |
| --engine                | Choose the engine (ch/fnf/gh1/rb/rb3)                                         |
| -p, --precision-mode    | Enable precision mode (CH only)                                               |
| -b, --blank             | Output a blank image without pathing                                          |
| --no-image              | Do not create an image                                                        |
| --no-bpms               | Do not draw BPMs                                                              |
| --no-solos              | Do not draw solo sections                                                     |
| --no-time-sigs          | Do not draw time signatures                                                   |
| --act-opacity           | Set opacity of activations in images                                          |

If you would like to conveniently run CHOpt on a setlist and you happen to be
on Windows, I made a PowerShell script that I've put [here](misc/setlist.ps1).
Change the four variables then run the script. The simplest way to do that is
probably to open the folder the script is in, double click the top bar and type
in cmd then enter to open a command prompt in that folder, then run the command

```bat
> powershell -ExecutionPolicy Bypass -File setlist.ps1
```

## Dependencies

* [Boost](https://www.boost.org) for Boost.Locale and Boost.Test
* [CImg](https://cimg.eu) to produce images
* [libpng](http://libpng.org/pub/png/libpng.html) to save pngs
* [Qt 6](https://www.qt.io) for the GUI and various utility code for both
versions
* [SightRead](https://github.com/GenericMadScientist/SightRead) for parsing
charts and midis
* [zlib](https://zlib.net) is a dependency of libpng

CImg is vendored in the repo. Boost, libpng, Qt, and zlib will need to be
provided by anyone compiling CHOpt for themselves. Boost is only required for
tests. libpng and zlib need to be set up so that
[FindPNG](https://cmake.org/cmake/help/latest/module/FindPNG.html) can find
them, and the same is true for Boost and Qt (see
[this](https://cmake.org/cmake/help/latest/module/FindBoost.html) and
[this](https://cmake.org/cmake/help/latest/manual/cmake-qt.7.html) page for
details). SightRead is included a git submodule.

## Acknowledgements

* FireFox2000000's Moonscraper .chart and .mid parsers were very helpful for
getting an initial idea of Clone Hero's parsing behaviour.
* Dinoguy1000 and shadoweh helped me make sure CHOpt runs on other peoples'
machines.
* Various users for feedback and testing, including 3-UP, AddyMills, ArchWK,
Bromik, CyclopsDragon, DNelson, GHNerd, GiometriQ, Haggis, Jdsmitty1, Joel,
Jpetersen5, Jrh, JUANPGP, Kyleruth, LightlessWalk, Littlejth, Lucretio,
NicoBrenChan, RandomDays, RileyTheFox, Taka, Tposejank, Venxm and Zantor.

## Contact

If you have any bug reports I would prefer they be reported on the GitHub page,
but feel free to send them to me on Discord at genericmadscientist.
