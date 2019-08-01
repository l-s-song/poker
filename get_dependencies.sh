#!/bin/sh

# Download Eidheim's Simple Web Server
if [ ! -d "./api/http" ]; then
	git clone https://gitlab.com/eidheim/Simple-Web-Server.git ./api/http
	cd ./api/http
	git checkout 684b9aa62a18f72150986899a3b9f31679ca620a
	cd ../../
fi

# Check if apt is installed
if ! apt -v; then
	echo "Apt is not installed; this script is dependent on apt"
	echo "On your linux distribution, please install apache2, g++, libboost, and libssl"
	exit
fi

# Update apt repositories
sudo apt update

# Install apache2, and activate needed modules
sudo apt install apache2
sudo a2enmod proxy
sudo a2enmod proxy_http

# Install C++ Dependencies
sudo apt install g++
sudo apt install libboost-all-dev
sudo apt install libssl-dev

# Install valgrind and net-tools
sudo apt install valgrind
sudo apt install net-tools

