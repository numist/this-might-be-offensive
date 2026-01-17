FROM nginx:1.24-alpine

# Remove default config
RUN rm /etc/nginx/conf.d/default.conf

# Copy nginx configuration
COPY services/nginx/nginx.conf /etc/nginx/nginx.conf
COPY services/nginx/tmbo.conf /etc/nginx/conf.d/tmbo.conf

# Create SSL directories (certs will be mounted)
RUN mkdir -p /etc/ssl/certs /etc/ssl/private

# Create log directory
RUN mkdir -p /var/log/nginx

EXPOSE 80 443

CMD ["nginx", "-g", "daemon off;"]
