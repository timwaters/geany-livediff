geany-livediff
==============

GeanyLiveDiff: plugin for geany which diffs file on-the-fly and displays colored markers on editor. 
Currently against git repos and regular files.

## Plugin is unstable
You may build it, make test and post issue. 

### Install

For example (debian)

#### Requirements

* cmake
* libgit2
* Boost
   * Boost libraries:
    * filesystem
    * system

```sh
$ sudo apt-get install libgit2-dev cmake libboost-filesystem-dev libboost-system-dev
```

#### Build

```sh
$ git clone https://github.com/v01d/geany-livediff
$ cd geany-livediff && mkdir build && cd build
$ cmake ../
$ make
$ chmod 644 liblivediff.so
$ cp liblivediff.so /usr/lib/x86_64-linux-gnu/geany/
```

Start geany and enable LiveDiff.
