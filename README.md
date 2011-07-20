realXtend Tundra 2.0
====================

Tundra is a scriptable 3D internet application development platform. It is aimed primarily for application developers, as a platform for creating networked 3D worlds with customized content.

Tundra is licensed under [Apache 2.0] and based on [Qt] and [Ogre3D].

Getting Started
---------------

Tundra uses a traditional server-client architecture for networking. After installing you will find the `Tundra` executable from the install directory, run `Tundra --help` for available commands.

This executable can be configured to run a set of C++ and JavaScript plugins. You can make your own config or use the ones that we ship. Here is some examples:

    Tundra --config viewer.xml
    Tundra --config plugins.xml --server --port 6565 --protocol udp
    Tundra --config server-headless.xml --server --headless

The Tundra server mode is used for standalone-mode editing and viewing Tundra documents. To host a 3D scene, run Tundra in dedicated mode using the --headless command line option. The Tundra viewer mode is the client that is used to connect to a server.

Compiling from sources
----------------------

Tundra source code is available at the [realXtend github repository]. This repository hosts various branches for new and old viewers from the realXtend team, so be sure to checkout `tundra2` branch after cloning.
 
Tundra uses [CMake] as its build system and depends on various other open source projects. See more from `doc/dependencies.txt`.

### Windows

For windows we support Visual Studio 2008 and 2010 build environments. Here are the quick steps for VC2008 after you have cloned the git repo. Same steps apply to VC2010, just with different batch scripts.

1.  Install [CMake]. (>= 2.8 is recommended)
2.  Run `win_update_deps_vs2008.bat` to acquire the prebuilt dependencies.
3.  Go through the `CMakeBuildConfigTemplate.txt` if you want to customise the build.
4.  Run `win_cmake_vs2008.bat`, this will generate a .sln solution file.
5.  Open the solution file with Visual Studio 2008 and build.

See also `doc/build-windows.txt`

### Linux

See `doc/build-linux.txt` for more details. Also check out the linux distro spesific build scripts in `/tools`.

Documentation
-------------

More information about Tundra can be found online at http://www.realxtend.org/doxygen/.

Contact Information
-------------------

You can find our developers from IRC `#realxtend-dev @ freenode`. Also check out the [user-oriented mailing list](http://groups.google.com/group/realxtend) and the [developer-oriented mailing list](http://groups.google.com/group/realxtend-dev).

Releases
--------

New releases are announced on the mailing lists and at the [realXtend blog]. They are uploaded to our google code project site, that we use for hosting downloads. http://code.google.com/p/realxtend-naali/downloads/list

[Qt]:          http://qt.nokia.com/                            "Qt homepage"
[Ogre3D]:      http://www.ogre3d.org/                          "Ogre3D homepage"
[Apache 2.0]:  http://www.apache.org/licenses/LICENSE-2.0.txt  "Apache 2.0 license"
[CMake]:       http://www.cmake.org/                           "CMake homepage"
[realXtend blog]: http://www.realxtend.org                     "realXtend blog"
[realXtend github repository]: https://github.com/realXtend/naali/tree/tundra2 "realXtend Tundra repository"
