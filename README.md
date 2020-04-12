# chopt

A command-line tool to work out optimal Star Power paths for Clone Hero songs.

[![Build Status](https://travis-ci.com/GenericMadScientist/chopt.svg?branch=master)](https://travis-ci.com/GenericMadScientist/chopt)
[![Build Status](https://ci.appveyor.com/api/projects/status/github/GenericMadScientist/chopt?branch=master&svg=true)](https://ci.appveyor.com/project/GenericMadScientist/chopt)

## Install

Download the latest version from the [Releases page](../../releases). If you're
on something other than Windows, then either compile it yourself or ask me. The
tests do pass on Mac OS and Linux, but I've not tried the program proper on
them.

## Usage

This is a command-line program. An example usage to path Trogdor on Hard and
save the output to trogdor_path.bmp is

```
> chopt.exe -f trogdor.chart -d hard -o trogdor_path.bmp
```

The difficulties are lower case (easy/medium/hard/expert) and defaults to
expert if none is provided. The full set of options can be listed by using -h or
--help.

## Limitations

This is version 0.2, so right now the program is quite limited. There is no
support for squeezing or early whammy, the only supported instrument is five
fret guitar, and only .chart files are accepted. The performance is currently
quite poor (GH3 Flames takes 16 minutes on my computer, and the runtime blows up
horribly for more complicated to path songs). I aim to address all of these
issues in subsequent versions.

## Dependencies

* [Catch2](https://github.com/catchorg/Catch2) 2.11.1 for tests
* [CImg](https://cimg.eu/) 2.9.0 to produce images
* [cxxopts](https://github.com/jarro2783/cxxopts) 2.2.0 for argument parsing

All of these are header-only libraries in the libs directory, so there should
be no difficulty getting these set up if you want to build chopt.

## Acknowledgements

* FireFox2000000's Moonscraper .chart parser was very helpful for getting an
initial idea of Clone Hero's parsing behaviour.
* Dinoguy1000 and shadoweh helped me make sure chopt runs on other peoples'
machines.

## Contact

If you have any bug reports I would prefer they be reported on the GitHub page,
but feel free to send them to me on Discord at GenericMadScientist#5303.
