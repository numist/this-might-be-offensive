# Same PHP as the php service. The search-sync service uses this image too.
FROM php:8.4-cli

RUN apt-get update && apt-get install -y --no-install-recommends libpq-dev \
    && docker-php-ext-install mysqli pdo_pgsql \
    && rm -rf /var/lib/apt/lists/*

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
