# SecuriKey

A project that uses the Omega2 to provide a means with which users can access their account credentials easily and securely.

## Prerequisites
### Linux / OS X
 * libssh-dev
 * uuid-dev
 * [LEDE build system](https://docs.onion.io/omega2-docs/cross-compiling.html) (For cross-compilation)
### Windows
 * libssh

## Installing

``` bash
# build host source
make [LOGLVL={1,2,3,4}]

# cross compile for omega2
cd src_omega
sh xCompile.sh -buildroot <PATH TO BUILD SYSTEM>
```

``` bash
# specify maximum severity level to log
# LEVEL_ERROR = 1
# LEVEL_WARNING = 2
# LEVEL_INFO = 3
# LEVEL_DEBUG = 4
```