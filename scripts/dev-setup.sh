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

# Generate the runtime config with fresh auth secrets.
#
# admin/.config.docker is committed and holds only non-secret service settings.
# The auth secrets -- remember_pepper, activation_salt, pwreset_salt -- sign the
# remember cookie, activation links, and password reset links, so a value that
# lives in the repository is a value an attacker already has. They are generated
# per checkout into admin/.config.generated, which is gitignored, and the
# containers mount that file as admin/.config.
#
# Regenerating invalidates outstanding remember cookies and activation/reset
# links, so an existing file is left alone. Delete it to get new secrets.
CONFIG_GENERATED="$PROJECT_ROOT/admin/.config.generated"
if [ ! -f "$CONFIG_GENERATED" ]; then
    echo "Generating auth secrets..."
    # umask so the secrets are not world-readable even briefly.
    (
        umask 077
        cat "$PROJECT_ROOT/admin/.config.docker" > "$CONFIG_GENERATED"
        for secret in remember_pepper activation_salt pwreset_salt; do
            echo "$secret = \"$(openssl rand -hex 32)\"" >> "$CONFIG_GENERATED"
        done
    )
    echo "Auth secrets generated."
else
    echo "Auth secrets already exist, skipping generation."
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
