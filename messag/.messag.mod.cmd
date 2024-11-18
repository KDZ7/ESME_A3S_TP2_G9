cmd_/home/kdz7/TP2/messag/messag.mod := printf '%s\n'   messag.o | awk '!x[$$0]++ { print("/home/kdz7/TP2/messag/"$$0) }' > /home/kdz7/TP2/messag/messag.mod
