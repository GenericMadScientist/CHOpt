# chopt

A command-line tool to work out optimal Star Power paths for Clone Hero songs.

[![Build Status](https://travis-ci.com/GenericMadScientist/chopt.svg?branch=develop)](https://travis-ci.com/GenericMadScientist/chopt)
[![Build Status](https://ci.appveyor.com/api/projects/status/github/GenericMadScientist/chopt?branch=develop&svg=true)](https://ci.appveyor.com/project/GenericMadScientist/chopt)

## Install

Download the latest version from the [Releases page](../../releases). If you're
on something other than Windows, then either compile it yourself or ask me. The
tests do pass on Mac OS and Linux, but I've not tried the program proper on
them. If you do plan to compile chopt yourself, make sure to set ENABLE_LTO to
ON in your CMakeCache.

## Usage

This is a command-line program. An example usage to path Trogdor on Hard and
save the output to trogdor_path.png is

```
> chopt.exe -f trogdor.chart -d hard -o trogdor_path.png
```

The difficulties are lower case (easy/medium/hard/expert) and defaults to
expert if none is provided. The full set of options can be listed by using -h or
--help.

## Limitations

This is version 0.3.2, so there are still some kinks and missing features. The
only supported instrument is five fret guitar, only .chart files are accepted,
and the drawn paths are sufficient to work out what the path is rather than
providing all the details like how much to squeeze or not whammy. The
performance is pretty good for most songs (chopt zips through The Ultimate
Endurance Challenge in a minute on my machine) although some songs cause the
runtime to go up horribly. I aim to address all of these issues in subsequent
versions.

## Dependencies

* [argparse](https://github.com/p-ranav/argparse) 2.1 for argument parsing
* [Catch2](https://github.com/catchorg/Catch2) 2.11.1 for tests
* [CImg](https://cimg.eu) 2.9.0 to produce images
* [libpng](http://libpng.org/pub/png/libpng.html) 1.6.37 to save pngs
* [zlib](https://zlib.net) 1.2.11 is a dependency of libpng

The first three are header-only libraries that are vendored in the chopt repo.
The latter two will need to be provided by anyone compiling chopt for
themselves. They need to be provided to CMake so that
[FindPNG](https://cmake.org/cmake/help/latest/module/FindPNG.html) can find
them.

## Acknowledgements

* FireFox2000000's Moonscraper .chart parser was very helpful for getting an
initial idea of Clone Hero's parsing behaviour.
* Dinoguy1000 and shadoweh helped me make sure chopt runs on other peoples'
machines.
* The CH Score Challenges server for feedback, especially CyclopsDragon,
LightlessWalk, Lucretio, and Taka.

## Contact

If you have any bug reports I would prefer they be reported on the GitHub page,
but feel free to send them to me on Discord at GenericMadScientist#5303.
