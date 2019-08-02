#!/bin/bash

sudo cp virtual_host.conf /etc/apache2/sites-available/000-default.conf
if ! sudo service apache2 status > /dev/null; then
  sudo service apache2 start
fi

valgrind_id=$(sudo netstat -ltnp | grep ':8080' | grep -oP '([0-9]*)/valgrind.bin' | grep -oP '[0-9]*')
if [ ! -z $valgrind_id ]; then
  kill $valgrind_id
fi

cd ./api
make debug
cd ..

