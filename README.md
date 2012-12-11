This Might Be Offensive
=======================

Getting Started with a Virtual Machine
--------------------------------------

The easiest way to get started is to download the development VM, which can be found at http://thismight.be/~thismightbe/vm.tgz. It was made in Fusion 4, but will work on a recent Workstation or ESX.

All user-level passwords are set to `[tmbo]`. This includes the web site's unprivileged account (asdf) and the machine user thismightbe.
All root-level passwords are set to `[nsfw]`. This includes the web site's admin account (admin) and the SSL certificate.

The codebase can be found in ~thismightbe/this-might-be-offensive as a read-only clone of this repo.

Rolling Your Own
----------------

If you want to build your own server, it won't take more than an hour or few, and you'll need the following packages:

* redis-server
* mysql-server
* nginx-full
* php5-fpm
* php5-mysql
* imagemagick
* perl
* xapian
* node.js (if available, very easy to build from source)

You'll want to look at the example nginx config file in admin. You'll also want to install the following perl modules from CPAN:

* DBI
* File::Copy
* Archive::Zip
* Image::Size
* ConfigReader::Simple

Point your webroot at the repository and create a new file in the `admin` directory named `.config` - this is where you will store your
database credentials.

The file follows .ini syntax, and should look something like this:

    [tmbo]
    database_host = "localhost"
    database_user = "bob"
    database_pass = "my_password_is_hard"
    database_name = "name_of_database"

You'll also want to put the following lines in your crontab:

    THEMAXX=/home/thismightbe/sites/tmbo
    MAILTO=thismightbe@numist.net
    
    # m     h       dom     mon     dow     cmd
    5 0 * * * $THEMAXX/offensive/deleteOldFiles.pl 7 zips > /dev/null
    2 0 * * * $THEMAXX/offensive/zipYesterday.pl zips
    3 1 * * * $THEMAXX/offensive/deleteOldFiles.pl 2 quarantine > /dev/null
    0 * * * * /usr/bin/nice /usr/bin/php5 $THEMAXX/admin/commentIndexer.php

Realtime Data
-------------------------

The realtime data system uses a redis and node.js to push data to connected clients.  The server is located in /realtime.  First install the dependencies with "npm install .", then start it with "node app.js".  There is also an example init script in /admin.


Content Protection Configuration
-------------------------

tmbo's content protection is the same in development as it is in production, which means if you browse to your instance by IP, you're not going to see any images. You should either disable the relevant section in the nginx config, or add the ip address you use to the allowed hosts. 

Developing using GitHub
-----------------------

If you want to fork the project to contribute changes via github:

    cd ~thismightbe
    rm this-might-be-offensive
    git clone git://github.com/USERNAME/this-might-be-offensive.git
    su -c "chgrp -R www-data ~thismightbe/this-might-be-offensive" -
    chmod -R g+w this-might-be-offensive

You'll need to build a replacement `admin/.config` file as detailed above.

Help!
-----

Problems with the web site are frequently well-documented by error messages emitted by trigger-error. Administrators see this output as part of the rendered page, but it is also recorded to the httpd's logs. On the VM they are located in `~thismightbe/logs/`.

If you need anything to get running, help can usually be had in #tmbotech on EFnet.
