#
# デフォルトターゲット
#
default:	build

#
# ビルドプロセス
#
build:	compile

#
# コンパイル
#
compile:
	gcc -g -Wall -O2 -o timaios src/core/*.c -Isrc/core/*.h


#
# - 生成物を削除
#
clean:
	rm -f ./timaios

start:
	./timaios

stop:
	pkill timaios

restart:	stop	start


hello:
	./test/run_and_request_and_kill.sh

package:	clean
	tar cvfz timaios`date +%Y%m%d`.tgz * --exclude="._*"

commit:
	git commit
push:
	git push
