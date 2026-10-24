# Same PHP as the php service, so admin/commentIndexer.php runs on 8.4.
FROM php:8.4-cli

RUN docker-php-ext-install mysqli

# Install cron, Perl, and dependencies.
RUN apt-get update && apt-get install -y \
    cron \
    perl \
    libdbi-perl \
    libarchive-zip-perl \
    libimage-size-perl \
    cpanminus \
    zip \
    && cpanm --notest ConfigReader::Simple \
    && rm -rf /var/lib/apt/lists/* \
    && ln -s /usr/local/bin/php /usr/bin/php

# Copy crontab
COPY services/cron/crontab /etc/cron.d/tmbo
RUN chmod 0644 /etc/cron.d/tmbo && crontab /etc/cron.d/tmbo

# Create log file for cron output
RUN touch /var/log/cron.log

WORKDIR /var/www/html

# Run cron in the foreground
CMD ["cron", "-f"]
