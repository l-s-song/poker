#!/bin/sh
sudo apt-get install libboost-all-dev
sudo apt-get install libssl-dev
git clone https://gitlab.com/eidheim/Simple-Web-Server.git http
cd http
git checkout 684b9aa62a18f72150986899a3b9f31679ca620a

