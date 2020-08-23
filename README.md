# chopt

A command-line tool to work out optimal Star Power paths for Clone Hero songs.

[![Build Status](https://travis-ci.com/GenericMadScientist/chopt.svg?branch=develop)](https://travis-ci.com/GenericMadScientist/chopt)
[![Build Status](https://ci.appveyor.com/api/projects/status/github/GenericMadScientist/chopt?branch=develop&svg=true)](https://ci.appveyor.com/project/GenericMadScientist/chopt)

## Install

Download the latest version from the [Releases page](../../releases). If you're
on something other than Windows or Linux, then either compile it yourself or ask
me. The tests do pass on Mac OS, but I've not tried the program proper on it. If
you do plan to compile chopt yourself, make sure to set ENABLE_LTO to ON in your
CMakeCache.

## Usage

This is a command-line program. An example usage to path Trogdor on Hard and
save the output to trogdor_path.png is

```
> chopt.exe -f trogdor.chart -d hard -o trogdor_path.png
```

Only the -f parameter is required, the difficulty defaults to Expert and the
path is by default saved to path.png. The full list of arguments can be found
by passing -h or --help to chopt, or by consulting the table below.

| Arguments             | Action                                                      |
| --------------------- | ----------------------------------------------------------- |
| -h, --help            | List optional arguments                                     |
| -f, --file            | Chart filename                                              |
| -o, --output          | Filename of output image (.bmp or .png)                     |
| -d, --diff            | Difficulty (easy/medium/hard/expert)                        |
| -i, --instrument      | Instrument (guitar/coop/bass/rhythm/keys/ghl/ghlbass/drums) |
| --sqz, --squeeze      | Set squeeze %                                               |
| --ew, --early-whammy  | Set early whammy %                                          |
| --lazy, --lazy-whammy | Set number of ms of whammy lost per sustain                 |
| -b, --blank           | Output a blank image without pathing                        |
| --no-bpms             | Do not draw BPMs                                            |
| --no-solos            | Do not draw solo sections                                   |
| --no-time-sigs        | Do not draw time signatures                                 |

## Dependencies

* [argparse](https://github.com/p-ranav/argparse) 2.1 for argument parsing
* [Catch2](https://github.com/catchorg/Catch2) 2.13.0 for tests
* [CImg](https://cimg.eu) 2.9.1 to produce images
* [libpng](http://libpng.org/pub/png/libpng.html) 1.6.37 to save pngs
* [Qt](https://www.qt.io) 5.15.0 for the GUI
* [zlib](https://zlib.net) 1.2.11 is a dependency of libpng

The first three are header-only libraries that are vendored in the chopt repo.
The latter three will need to be provided by anyone compiling chopt for
themselves, although Qt is only required if the GUI version is being compiled.
libpng and zlib need to be set up so that
[FindPNG](https://cmake.org/cmake/help/latest/module/FindPNG.html) can find
them, and the same is true for Qt (see
[this page](https://cmake.org/cmake/help/latest/manual/cmake-qt.7.html) for
details).

## Acknowledgements

* FireFox2000000's Moonscraper .chart and .mid parsers were very helpful for
getting an initial idea of Clone Hero's parsing behaviour.
* Dinoguy1000 and shadoweh helped me make sure chopt runs on other peoples'
machines.
* Various users for feedback, including CyclopsDragon, LightlessWalk, Lucretio,
RileyTheFox, and Taka.

## Contact

If you have any bug reports I would prefer they be reported on the GitHub page,
but feel free to send them to me on Discord at GenericMadScientist#5303.
