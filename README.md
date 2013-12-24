geany-livediff
==============

GeanyLiveDiff: plugin for geany which diffs file on-the-fly and displays colored markers on editor. 
Currently against git repos and regular files.

![livediff.png](https://raw.github.com/v01d/geany-livediff/master/livediff.png)

### Install

For example (debian)

#### Requirements

* cmake
* libgit2
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
$ sudo make install
```

Start geany and enable LiveDiff.