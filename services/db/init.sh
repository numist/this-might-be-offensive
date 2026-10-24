#!/bin/bash
# Runs once, when the db container starts on an empty data volume.
#
# Only schema.sql and populate.sql: the other files in admin/database are
# migrations that schema.sql already includes, plus dbinit.sql (the image
# creates the database and user itself) and sanitize.sql, which must never run
# against a dev database by accident.
set -e
mariadb -uroot -p"$MARIADB_ROOT_PASSWORD" "$MARIADB_DATABASE" < /tmbo-database/schema.sql
mariadb -uroot -p"$MARIADB_ROOT_PASSWORD" "$MARIADB_DATABASE" < /tmbo-database/populate.sql
