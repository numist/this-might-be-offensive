FROM devilbox/php-fpm:5.6-prod

# Stretch is EOL, so its signing keys have expired and the keyring shipped in this
# 2017-era base image predates the keys now used to sign the archive. The archive
# is still validly signed -- apt reports EXPKEYSIG/NO_PUBKEY, not a bad signature.
#
# Install the current debian-archive-keyring, pinned by SHA-256 and fetched over
# HTTPS, so apt has a working trust anchor. That is what lets the install below
# run without --allow-unauthenticated: signature verification stays ON, and a
# tampered mirror fails the build rather than silently shipping a modified package.
#
# The .deb is verified by hash before dpkg touches it, so the download itself does
# not need to be trusted. To update, bump both ARGs together.
ARG KEYRING_URL=https://archive.debian.org/debian/pool/main/d/debian-archive-keyring/debian-archive-keyring_2021.1.1+deb11u1_all.deb
ARG KEYRING_SHA256=28ca7749ab7978f3c571732c3aa1c56e3ad1d5db3c915293763d4f6cb8fcce89
RUN set -eux; \
    curl -fsSL -o /tmp/keyring.deb "$KEYRING_URL"; \
    echo "${KEYRING_SHA256}  /tmp/keyring.deb" | sha256sum -c -; \
    dpkg -i /tmp/keyring.deb; \
    rm -f /tmp/keyring.deb

# Use archived Debian repos for Stretch (EOL).
# Check-Valid-Until is disabled because the archive's Release files are years past
# their expiry date; this relaxes freshness only, not signature verification.
RUN echo 'deb http://archive.debian.org/debian stretch main' > /etc/apt/sources.list && \
    echo 'deb http://archive.debian.org/debian-security stretch/updates main' >> /etc/apt/sources.list && \
    echo 'Acquire::Check-Valid-Until "false";' > /etc/apt/apt.conf.d/99no-check-valid-until

# Install cron, Perl, and dependencies. Deliberately no --allow-unauthenticated:
# if this ever starts failing verification, that is a signal worth stopping for.
RUN apt-get update && apt-get install -y \
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
