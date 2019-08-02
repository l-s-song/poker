# Poker
Plans to be a functional poker website

# Dependencies

- Apache2
- g++
- Boost
- OpenSSL
- Eidheim's Simple-Web-Server
- Valgrind

Valgrind is optional if you adjust `./api/makefile` and `./run_server.sh` accordingly.

# Setup
First, we install the dependencies
```
./get_dependencies.sh
```

Now, we link the git directory to apache2's host location.
```
sudo rm -r /var/www/html
sudo ln -s $(pwd) /var/www/html
```

This is presuming you're in the git directory, otherwise replace `$(pwd)` with the absolute location of the git directory. Note that moving the git directory will break this link.

Make sure that your server is only accepting requests to port 80, and not port 8080.

# Run
To run the server, simply execute `./run_server.sh`. Note that this will overwrite `/etc/apache2/sites-available/000-default.conf`.

