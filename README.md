# Poker
Plans to be a functional poker website

# Dependencies
Apache2
G++
Boost
OpenSSL
eidheim's Simple-Web-Server

# Setup
First, we install the dependencies
```
./get_dependencies.sh
```

Now, we link the git directory to apache2's host location.
```
ln -s . /var/www/html
```

Inside of `etc/apache2/apache2.conf`, add
```
ProxyPass /api http://localhost:8080/api
```
at the end of the file. This will make apache2 route api requests to the C++ server internally.

Also, inside of the `<Directory /var/www></Directory>` tags, add
```
Order allow,deny
Deny from all
<FilesMatch "\.(html|ico|css|js)">
  Allow from all
</FilesMatch>
```
This will only allow apache to serve files needed to render the webpage, so that users cannot view the rest of the git repository.

# Run
To run the server, simple execute `./run_server.sh`
