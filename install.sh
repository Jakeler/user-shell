#!/bin/bash
install -Dm 755 build/src/ush /usr/bin/ush
setcap cap_setuid,cap_setgid+ep ush
install -dm 1777 /etc/ush

useradd jk -G users,adm
install -Dm 744 -o jk example_config.conf /etc/ush/id.conf

