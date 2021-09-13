
# Welcome to XEphem!

XEphem is an interactive astronomy program for all UNIX platforms,
written and maintained by Elwood Downey over more than thirty years
1990–2021 and now generously released under the MIT License.

![Sky view](/GUI/xephem/help/png/sky-view.png?raw=true)

There are more screenshots at bottom of this README.

## Documentation

* [Home Page](https://xephem.github.io/XEphem/Site/xephem.html)
* [Reference Manual](https://xephem.github.io/XEphem/GUI/xephem/help/xephem.html)
* [Changelog](https://xephem.github.io/XEphem/Site/changes.html)

## Resources

* [GitHub repository](https://github.com/XEphem/XEphem)
* [GitHub Discussions](https://github.com/XEphem/XEphem/discussions)
* [Mailing list](https://groups.io/g/xephem)
* [Star catalogs](https://github.com/XEphem/Catalogs)

## Installing XEphem

There are XEphem packages available for several operating systems:

* [Gentoo](https://packages.gentoo.org/packages/sci-astronomy/xephem)
* [openSUSE](https://build.opensuse.org/package/show/Application:Geo/xephem)
* [Slack](https://slackbuilds.org/repository/13.37/academic/xephem/)

Or you can try compiling from source code.  XEphem should compile fine
on modern Linux systems, though perhaps not as well for folks on macOS.
The source includes an `INSTALL` file with instructions.  There are two
ways to get the source:

* Download a recent [release](https://github.com/XEphem/XEphem/releases)
* Checkout the [git repository](https://github.com/XEphem/XEphem)

If you want to get it running quickly from the source code, the
repository includes the code for three Docker containers that are each
capable of compiling and running XEphem.  The commands to launch them
when sitting at the repository root are:

    $ ./Containers/docker-ubuntu-14.04/run
    $ ./Containers/docker-ubuntu-20.04/run
    $ ./Containers/docker-ubuntu-21.04/run

Once one of the containers has launched, use the shell command history:
press up-arrow to ask for the previous command — it will be the correct
command to compile and run XEphem.

The repository also includes a `.tito` directory and `xephem.spec` file
for building an XEphem RPM package.

## More Screenshots

![Sun view](/GUI/xephem/help/png/sun-view.png?raw=true)

![Solar system view](/GUI/xephem/help/png/solsys.png?raw=true)

![Saturn and its moons](/GUI/xephem/help/png/saturn.png?raw=true)

![Binary star view](/GUI/xephem/help/png/sky-binary.png?raw=true)

![Moon view](/GUI/xephem/help/png/moon.png?raw=true)

![Earth view](/GUI/xephem/help/png/earth.png?raw=true)
