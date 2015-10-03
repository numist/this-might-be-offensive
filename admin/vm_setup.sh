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
  cpanminus \
  mysql-client \
  mysql-server \
  nginx \
  nodejs-legacy \
  npm \
  php5 \
  php5-cli \
  php5-fpm \
  php5-imagick \
  php5-mysql \
  redis-server \
  redis-tools

# TODO: test redis
# TODO: set up realtime
# TODO: comment indexer

service nginx stop
service php5-fpm stop

mkdir -p /home/vagrant/logs $SRCROOT/offensive/{zips,uploads,quarantine}
chown -R www-data:www-data /home/vagrant/logs

cpanm --notest \
  DBI \
  File::Copy \
  Archive::Zip \
  Image::Size \
  ConfigReader::Simple

npm install -g \
  iniparser \
  mysql \
  redis \
  socket.io

# XXX: dev-only
# TODO: idempotence
openssl req -nodes -x509 -subj "/C=US/ST=OK/L=Okie/O=tmbo/CN=*.localhost" -newkey rsa:2048 -keyout /etc/ssl/private/thismight.be.key -out /etc/ssl/certs/thismight.be.pem -days 365

redis-cli FLUSHALL
mysql --password=shortbus < $SRCROOT/admin/database/dbinit.sql
mysql --password=shortbus tmbo < $SRCROOT/admin/database/schema.sql
mysql --password=shortbus tmbo < $SRCROOT/admin/database/populate.sql

config_file /etc/cron.d/tmbo
config_file /etc/init.d/tmbo-realtime
update-rc.d tmbo-realtime defaults 98 02
service tmbo-realtime start

config_file /etc/php5/fpm/php.ini
config_file /etc/php5/fpm/php-fpm.conf
service php5-fpm start

config_file /etc/nginx/sites-available/default
unlink /etc/nginx/sites-enabled/default
ln -s /etc/nginx/sites-available/default /etc/nginx/sites-enabled/default
service nginx start
