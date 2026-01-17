FROM devilbox/php-fpm:5.6-prod

# The devilbox image already includes most extensions including imagick
# No additional apt-get needed

# Copy PHP configuration - devilbox uses /usr/local/etc paths
COPY services/php/php.ini /usr/local/etc/php/php.ini
COPY services/php/www.conf /usr/local/etc/php-fpm.d/www.conf

# Set working directory
WORKDIR /var/www/html

# Create upload directories with proper permissions
RUN mkdir -p /var/www/html/offensive/uploads \
             /var/www/html/offensive/zips \
             /var/www/html/offensive/quarantine \
    && chown -R devilbox:devilbox /var/www/html/offensive

EXPOSE 9000

# Use the default entrypoint and command from devilbox image
# The devilbox image runs php-fpm automatically via its entrypoint
