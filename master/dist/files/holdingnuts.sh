#!/bin/sh
# HoldingNuts wrapper script 
 
# library directory
LIBDIR=lib
 
# resolve symlink
pth=`readlink $0`
 
# $pth will be empty if our start path wasn't a symlink
if [ $pth ] ; then
        GAMEDIR=`dirname $pth`
else
        GAMEDIR=`dirname $0`
fi
 
# change into game root
cd $GAMEDIR
 
# export library directory; only use LD_LIBRARY_PATH if it is set
if [ -n "$LD_LIBRARY_PATH" ] ; then
    export LD_LIBRARY_PATH="$GAMEDIR/$LIBDIR:$LD_LIBRARY_PATH"
else
    export LD_LIBRARY_PATH="$GAMEDIR/$LIBDIR"
fi
 
./holdingnuts $@
