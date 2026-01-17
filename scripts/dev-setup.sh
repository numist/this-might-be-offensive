#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "Setting up TMBO development environment..."

# Create SSL certificate directories
mkdir -p "$PROJECT_ROOT/services/nginx/certs"
mkdir -p "$PROJECT_ROOT/services/nginx/private"

# Generate self-signed SSL certificates if they don't exist
if [ ! -f "$PROJECT_ROOT/services/nginx/certs/tmbo.pem" ]; then
    echo "Generating self-signed SSL certificates..."
    openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
        -keyout "$PROJECT_ROOT/services/nginx/private/tmbo.key" \
        -out "$PROJECT_ROOT/services/nginx/certs/tmbo.pem" \
        -subj "/C=US/ST=Dev/L=Local/O=TMBO/CN=localhost"
    echo "SSL certificates generated."
else
    echo "SSL certificates already exist, skipping generation."
fi

# Create upload directories if they don't exist
mkdir -p "$PROJECT_ROOT/services/web/src/offensive/uploads"
mkdir -p "$PROJECT_ROOT/services/web/src/offensive/zips"
mkdir -p "$PROJECT_ROOT/services/web/src/offensive/quarantine"

echo ""
echo "Development setup complete!"
echo ""
echo "To start the application, run:"
echo "  docker compose up"
echo ""
echo "To start with cron jobs enabled:"
echo "  docker compose --profile full up"
echo ""
echo "The application will be available at:"
echo "  https://localhost (accept the self-signed certificate warning)"
echo ""
echo "Default login credentials (from populate.sql):"
echo "  Username: admin"
echo "  Password: [nsfw]"
echo ""
