
# XEphem Docker container

This directory offers a `Dockerfile` plus a small shell script `run`.
Run the shell script to build and launch a Ubuntu 14.04 container with
all the libraries XEphem needs:

    ./container/run

Then, at the container’s prompt (which looks like `root@...:/src#`),
you can build XEphem with:

    make -C /src/GUI/xephem

And run it with:

    /src/GUI/xephem/xephem

If you press up-arrow at the bash prompt right after the container
launches, you will find this pair of commands already in the shell
history so you can simply press Enter to run them.
