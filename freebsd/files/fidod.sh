#!/bin/sh

if ! PREFIX=$(expr $0 : "\(/.*\)/etc/rc\.d/$(basename $0)\$"); then
    echo "$0: Cannot determine the PREFIX" >&2
    exit 1
fi



case $1 in
start)
    if [ -x ${PREFIX}/sbin/fidod ]; then
        ${PREFIX}/sbin/fidod  &&
        echo -n ' fidod'
    fi
    ;;

stop)
    kill `head /var/run/fidod.pid` && echo -n ' fidod'
    ;;

restart)
    killall -HUP drwebd && echo -n ' drwebd'
    ;;

*)
    echo "usage: `basename $0` {start|stop|restart}" >&2
    exit 64
    ;;
esac

