#!/bin/sh
if ! sudo service apache2 status; then
  sudo service apache2 start
fi
cd ./api
make debug
cd ..
