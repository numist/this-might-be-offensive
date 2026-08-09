FROM devilbox/php-fpm:5.6-prod

# The devilbox image already includes most extensions including imagick
# No additional apt-get needed

# Copy PHP configuration - devilbox uses /usr/local/etc paths
COPY services/php/php.ini /usr/local/etc/php/php.ini
COPY services/php/www.conf /usr/local/etc/php-fpm.d/www.conf

# Set working directory
WORKDIR /var/www/html

# Create upload directories with proper permissions.
#
# devilbox is correct here, even though www.conf says user/group = www-data:
# php-fpm.conf re-declares [www] user/group AFTER its include of php-fpm.d/*.conf
# ("the following settings overwrite any includes again"), so the pool always runs
# as devilbox (uid 1000). Verified with ps: workers run as devilbox.
#
# In development these paths are bind mounts, so the host's ownership wins over
# anything set here and the chown only matters for non-mounted deployments.
RUN mkdir -p /var/www/html/offensive/uploads \
             /var/www/html/offensive/zips \
             /var/www/html/offensive/quarantine \
    && chown -R devilbox:devilbox /var/www/html/offensive

EXPOSE 9000

# Use the default entrypoint and command from devilbox image
# The devilbox image runs php-fpm automatically via its entrypoint
