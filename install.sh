#!/bin/bash
cp build/src/ush /usr/bin/ush
sudo setcap cap_setuid,cap_setgid+ep ush
mkdir -p /etc/ush
chmod 1666 /etc/ush
