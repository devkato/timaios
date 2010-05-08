#========================================
#
# - variables
#
#========================================
CC_DEV = gcc -g -Wall
CC_PROD = gcc -Wall -O3
INCLUDES = -Isrc/core/*.h -Isrc/action/*.h
SRC = src/core/*.c src/action/*.c
#SRC_EXT = src/ext/kvs/*.c src/ext/rdb/*.c
SRC_EXT = src/ext/3rd/http_parser.o

#WALL = -Wextra -Wformat=2 -Wstrict-aliasing=2 -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wfloat-equal -Wpointer-arith -Wswitch-enum -Winline
WALL = 

# LIBS = -lpthread
LIBS =

#========================================
#
# Default target
#
#========================================
default:	build


#========================================
#
# Build(Compile and other tasks to deploy)
#
#========================================
build:	compile

compile:
	$(CC_DEV) $(WALL) -o timaios $(SRC) $(SRC_EXT) $(INCLUDES) $(LIBS)

product:
	$(CC_PROD) -o timaios -DTM_PRODUCTION $(SRC) $(SRC_EXT) $(INCLUDES) $(LIBS)

prod:	product


clean:
	clear && rm -f ./timaios ./timaios.pid ./timaios.log ./core.*

c:	clean

start:
	./timaios

stop:
	pkill timaios

restart:	stop	start

top:
	top -p `cat timaios.pid`

kill:
	pkill timaios

k:	kill

dev:	clean	compile	start

#========================================
#
# - tasks with git repository
#
#========================================
package:	clean
	tar cvfz timaios`date +%Y%m%d`.tgz * --exclude="._*"

commit:
	git commit -a

push:
	git push

p:	push

#========================================
#
# - send test requsts
#
#========================================
curl:
#	curl -v "http://localhost:12345/"
	curl -v "http://localhost:12345/abc/defg/h?hello=world&heal=theworld"

hello:
	./test/run_and_request_and_kill.sh

req_normal:
	./test/run_and_request_and_kill.sh normal

rn:	req_normal

req_heavy:
	./test/run_and_request_and_kill.sh heavy

rh:	req_heavy

req_deadly:
	./test/run_and_request_and_kill.sh deadly

rd:	req_deadly
