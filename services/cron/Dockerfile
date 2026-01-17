FROM devilbox/php-fpm:5.6-prod

# Use archived Debian repos for Stretch (EOL)
RUN echo 'deb http://archive.debian.org/debian stretch main' > /etc/apt/sources.list && \
    echo 'deb http://archive.debian.org/debian-security stretch/updates main' >> /etc/apt/sources.list && \
    echo 'Acquire::Check-Valid-Until "false";' > /etc/apt/apt.conf.d/99no-check-valid-until

# Install cron, Perl, and dependencies
RUN apt-get update && apt-get install -y --allow-unauthenticated \
    cron \
    perl \
    libdbi-perl \
    libarchive-zip-perl \
    libimage-size-perl \
    cpanminus \
    zip \
    && cpanm --notest ConfigReader::Simple \
    && rm -rf /var/lib/apt/lists/*

# Copy crontab
COPY services/cron/crontab /etc/cron.d/tmbo
RUN chmod 0644 /etc/cron.d/tmbo && crontab /etc/cron.d/tmbo

# Create log file for cron output
RUN touch /var/log/cron.log

WORKDIR /var/www/html

# Run cron in the foreground
CMD ["cron", "-f"]
