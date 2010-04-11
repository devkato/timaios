#!/bin/sh

level=$1
host="localhost"
port="12345"
path="/"
concurrency=1000

case ${level} in
  "normal" ) ab -n10000 -c${concurrency} "http://${host}:${port}${path}" ;;
  "heavy" ) ab -n100000 -c${concurrency} "http://${host}:${port}${path}" ;;
  "deadly" ) ab -n1000000 -c${concurrency} "http://${host}:${port}${path}" ;;
esac

