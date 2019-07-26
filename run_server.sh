#!/bin/sh
if ! sudo service apache2 status > /dev/null; then
  sudo service apache2 start
fi
valgrind_id=$(sudo netstat -ltnp | grep ':8080' | grep -oP '([0-9]*)/valgrind.bin' | grep -oP '[0-9]*')
kill $valgrind_id
cd ./api
make debug
cd ..

