SRCROOT=/home/vagrant/sites/tmbo

# System configuration files are backed up before the new file is copied into place
function system_config_file {
  if [ -f "$1" -a ! -f "$1.backup" ]; then
    echo "Backing up $1 -> $1.backup"
    cp "$1" "$1.backup"
  fi

  echo "Installing $1"
  [ -f "$SRCROOT/admin/configroot$1" ] || ( echo "Missing file $SRCROOT/admin/configroot$1"; exit 1 )
  cp "$SRCROOT/admin/configroot$1" "$1"
}

# Site configuration files are replaced if they already exist
function site_config_file {
  echo "Installing $1"
  [ -f "$1" ] && rm -r "$1"
  [ -f "$SRCROOT/admin/configroot$1" ] || ( echo "Missing file $SRCROOT/admin/configroot$1"; exit 1 )
  cp "$SRCROOT/admin/configroot$1" "$1"
}

function data_dir {
  [ -d $1 ] && rm -r $1
  echo "Setting up $1"
  mkdir -p $1
  chown -R www-data:www-data $1
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

# TODO: comment indexer

service nginx stop
service php5-fpm stop

data_dir /home/vagrant/logs
data_dir $SRCROOT/offensive/zips
data_dir $SRCROOT/offensive/uploads
data_dir $SRCROOT/offensive/quarantine

echo "Installing periodic dependencies"
cpanm --notest \
  DBI \
  File::Copy \
  Archive::Zip \
  Image::Size \
  ConfigReader::Simple

echo "Installing realtime dependencies"
npm install -g \
  cookies \
  iniparser \
  mysql \
  redis \
  socket.io \
2> /dev/null

# XXX: dev-only
echo "Generating SSL certificate"
openssl req -nodes -x509 -subj "/C=US/ST=OK/L=Okie/O=tmbo/CN=*.localhost" -newkey rsa:2048 -keyout /etc/ssl/private/thismight.be.key -out /etc/ssl/certs/thismight.be.pem -days 365 2> /dev/null

redis-cli FLUSHALL
mysql --password=shortbus < $SRCROOT/admin/database/dbinit.sql
mysql --password=shortbus tmbo < $SRCROOT/admin/database/schema.sql
mysql --password=shortbus tmbo < $SRCROOT/admin/database/populate.sql
site_config_file /home/vagrant/sites/tmbo/admin/.config

system_config_file /etc/php5/cli/php.ini
site_config_file /etc/cron.d/tmbo

site_config_file /etc/init.d/tmbo-realtime
update-rc.d tmbo-realtime defaults 98 02
service tmbo-realtime start

system_config_file /etc/php5/fpm/php.ini
system_config_file /etc/php5/fpm/pool.d/www.conf
service php5-fpm start

system_config_file /etc/nginx/sites-available/default
unlink /etc/nginx/sites-enabled/default
ln -s /etc/nginx/sites-available/default /etc/nginx/sites-enabled/default
service nginx start
