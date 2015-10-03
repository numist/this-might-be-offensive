SRCROOT=/home/vagrant/sites/tmbo

function config_file {
    cp $SRCROOT/admin/configroot$1 $1
}

# XXX: dev-only
debconf-set-selections <<< 'mysql-server mysql-server/root_password password shortbus'
debconf-set-selections <<< 'mysql-server mysql-server/root_password_again password shortbus'

aptitude remove \
  apache2 \
  libapache2-mod-php5
aptitude -y install \
  apt-xapian-index \
  cpanminus \
  mysql-client \
  mysql-server \
  nginx \
  php5 \
  php5-cli \
  php5-fpm \
  php5-imagick \
  php5-mysql \
  redis-server \
  redis-tools

service nginx stop
service php5-fpm stop

cpanm \
  DBI \
  File::Copy \
  Archive::Zip \
  Image::Size \
  ConfigReader::Simple

mysql --password=shortbus < $SRCROOT/admin/database/dbinit.sql
mysql --password=shortbus tmbo < $SRCROOT/admin/database/schema.sql
mysql --password=shortbus tmbo < $SRCROOT/admin/database/populate.sql

mkdir -p /home/vagrant/logs
chown -R vagrant:www-data /home/vagrant

config_file /etc/cron.d/tmbo

config_file /etc/php5/fpm/php.ini
config_file /etc/php5/fpm/php-fpm.conf
service php5-fpm start

config_file /etc/nginx/sites-available/default
unlink /etc/nginx/sites-enabled/default
ln -s /etc/nginx/sites-available/default /etc/nginx/sites-enabled/default
openssl req -nodes -x509 -subj "/C=US/ST=OK/L=Okie/O=tmbo/CN=*.thismight.be" -newkey rsa:2048 -keyout /etc/ssl/private/thismight.be.key -out /etc/ssl/certs/thismight.be.pem -days 365
service nginx start
