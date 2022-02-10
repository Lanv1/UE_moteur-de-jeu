#!/bin/sh
bindir=$(pwd)
cd /home/spon/M1/S2/hai819i-moteur/TP2/TP2/
export 

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r  " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /home/spon/M1/S2/hai819i-moteur/TP2/build/TP2 
	else
		"/home/spon/M1/S2/hai819i-moteur/TP2/build/TP2"  
	fi
else
	"/home/spon/M1/S2/hai819i-moteur/TP2/build/TP2"  
fi
