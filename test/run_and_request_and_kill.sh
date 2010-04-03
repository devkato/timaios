#!/bin/sh

level=$1
host="localhost"
port="12345"
concurrency=1000

case ${level} in
  "normal" ) ab -n10000 -c${concurrency} "http://${host}:${port}/echo" ;;
  "heavy" ) ab -n100000 -c${concurrency} "http://${host}:${port}/echo" ;;
esac

