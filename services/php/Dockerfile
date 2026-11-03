# PHP 8.4, matching production since the PHP 8.4 / MariaDB port.
FROM php:8.4-fpm

# mysqli for the app, gd + imagick for thumbnails (Upload::thumb() uses Imagick).
# imagick is pinned so builds are reproducible. Note that `pecl install`
# regenerates its arginfo headers with a script that downloads PHP-Parser from
# GitHub, so building this image needs GitHub access.
RUN apt-get update && apt-get install -y --no-install-recommends \
        libmagickwand-dev \
        libpng-dev \
        libjpeg62-turbo-dev \
        libfreetype6-dev \
    && docker-php-ext-configure gd --with-jpeg --with-freetype \
    && docker-php-ext-install -j"$(nproc)" mysqli gd exif \
    && pecl install imagick-3.8.0 \
    && docker-php-ext-enable imagick \
    && rm -rf /var/lib/apt/lists/* /tmp/pear

# Copy PHP configuration
COPY services/php/php.ini /usr/local/etc/php/php.ini
COPY services/php/www.conf /usr/local/etc/php-fpm.d/www.conf

# Only let ImageMagick decode the formats the site accepts (see the file).
COPY services/php/imagemagick-policy.xml /etc/ImageMagick-7/policy.xml
# Fail the build if the policy didn't take (e.g. ImageMagick's config moved):
# it must refuse an SVG.
RUN php -r 'try { (new Imagick())->readImageBlob("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"1\" height=\"1\"/>"); fwrite(STDERR, "ImageMagick policy not applied\n"); exit(1); } catch (ImagickException $e) {}'

# Set working directory
WORKDIR /var/www/html

# Create upload directories with proper permissions. The pool runs as www-data
# (see www.conf). In development these paths are bind mounts, so the host's
# ownership wins over anything set here and the chown only matters for
# non-mounted deployments.
RUN mkdir -p /var/www/html/offensive/uploads \
             /var/www/html/offensive/zips \
             /var/www/html/offensive/quarantine \
    && chown -R www-data:www-data /var/www/html/offensive

EXPOSE 9000
