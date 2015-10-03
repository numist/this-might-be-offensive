SRCROOT=/home/vagrant/sites/tmbo

# XXX: dev-only
debconf-set-selections <<< 'mysql-server mysql-server/root_password password shortbus'
debconf-set-selections <<< 'mysql-server mysql-server/root_password_again password shortbus'

aptitude remove apache2 libapache2-mod-php5
aptitude -y install \
  apt-xapian-index \
  mysql-client \
  mysql-server \
  nginx \
  php5 \
  php5-fpm \
  php5-imagick \
  php5-mysql \
  redis-server \
  redis-tools

service nginx stop
service php5-fpm stop

mysql --password=shortbus < $SRCROOT/admin/database/dbinit.sql
mysql --password=shortbus tmbo < $SRCROOT/admin/database/schema.sql
mysql --password=shortbus tmbo < $SRCROOT/admin/database/populate.sql

mkdir -p /home/vagrant/logs
chown -R vagrant:www-data /home/vagrant

cp $SRCROOT/admin/configroot/etc/php5/fpm/php.ini /etc/php5/fpm/php.ini
cp $SRCROOT/admin/configroot/etc/php5/fpm/php-fpm.conf /etc/php5/fpm/php-fpm.conf
service php5-fpm start

cp $SRCROOT/admin/configroot/etc/nginx/sites-available/default /etc/nginx/sites-available/default
unlink /etc/nginx/sites-enabled/default
ln -s /etc/nginx/sites-available/default /etc/nginx/sites-enabled/default
openssl req -nodes -x509 -subj "/C=US/ST=OK/L=Okie/O=tmbo/CN=*.thismight.be" -newkey rsa:2048 -keyout /etc/ssl/private/thismight.be.key -out /etc/ssl/certs/thismight.be.pem -days 365
service nginx start
