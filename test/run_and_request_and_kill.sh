#!/bin/sh

host="localhost"
port="12345"

curl -v "http://${host}:${port}/"

ab -n10000 -c1000 "http://${host}:${port}/"
