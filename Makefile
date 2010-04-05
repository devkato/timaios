#========================================
#
# - variables
#
#========================================
CC_DEV = gcc -g -Wall
CC_PROD = gcc -Wall -O3
INCLUDES = -Isrc/core/*.h -Isrc/action/*.h
SRC = src/core/*.c src/action/*.c


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
	$(CC_DEV) -o timaios $(SRC) $(INCLUDES)

product:
	$(CC_PROD) -o timaios -DTM_PRODUCTION $(SRC) $(INCLUDES)

prod:	product


clean:
	rm -f ./timaios ./timaios.pid ./timaios.log

start:
	./timaios

stop:
	pkill timaios

restart:	stop	start

top:
	top -p `cat timaios.pid`

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


#========================================
#
# - send test requsts
#
#========================================
curl:
	curl -v "http://localhost:12345/abc/defg/h?hello=world&heal=theworld"

hello:
	./test/run_and_request_and_kill.sh

req_normal:
	./test/run_and_request_and_kill.sh normal

rn:	req_normal

req_heavy:
	./test/run_and_request_and_kill.sh heavy

rh:	req_heavy
