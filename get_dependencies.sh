#!/bin/sh
sudo apt-get install apache2
a2enmod proxy
a2enmod proxy_http

sudo apt-get install g++
sudo apt-get install libboost-all-dev
sudo apt-get install libssl-dev
git clone https://gitlab.com/eidheim/Simple-Web-Server.git ./api/http
cd ./api/http
git checkout 684b9aa62a18f72150986899a3b9f31679ca620a
cd ../../
