This Might Be Offensive
=======================

Getting Started (Docker)
------------------------

1. Install Docker and Docker Compose
2. `git clone git@github.com:numist/this-might-be-offensive.git`
3. `cd this-might-be-offensive`
4. `./scripts/dev-setup.sh`
5. `docker compose up`
6. Point your browser to [`https://localhost/offensive`](https://localhost/offensive/) (accept the self-signed certificate warning)
7. Log in as `admin`/`[nsfw]`

To include cron jobs (for zip file creation, cleanup, and comment indexing):

```bash
docker compose --profile full up
```

The stack matches production: PHP 8.4 and MariaDB 10.11. On first start, with an
empty database volume, the db container loads `admin/database/schema.sql` and
`populate.sql` (see `services/db/init.sh`).

**Upgrading from the old PHP 5.6 / MySQL 8.0 setup:** MariaDB can't start on a
volume MySQL 8.0 created, and the schema is only loaded into an empty volume, so
rebuild and start over with a fresh database. This deletes your local database:

```bash
docker compose exec db mysqldump -uroot -pshortbus tmbo > backup.sql   # optional, to keep data
docker compose down -v
docker compose --profile full up --build
```

**Linux hosts:** PHP-FPM runs as `www-data` (uid 33). `dev-setup.sh` creates
`admin/.config.generated` readable only by you, and the upload directories are
owned by you, so PHP can't read its config ("No configuration file found") or
write uploads. Docker Desktop on macOS hides this. On Linux, run:

```bash
chmod 644 admin/.config.generated
sudo chown -R 33:33 services/web/src/offensive
```

**Services:**

- **nginx** - Web server (ports 80/443)
- **php** - PHP 8.4 FPM for application code
- **db** - MariaDB 10.11 (exposed on port 3307 for local access)
- **redis** - Session and cache storage
- **realtime** - Node.js WebSocket server
- **cron** - Scheduled tasks (only with `--profile full`)

Getting Started (Vagrant - Legacy)
----------------------------------

1. Install vagrant and virtualbox. If you're on a Mac: `brew install Caskroom/cask/vagrant Caskroom/cask/virtualbox`
2. `git clone git@github.com:numist/this-might-be-offensive.git`
3. `cd this-might-be-offensive`
4. `vagrant up`
5. Point your browser to [`https://localhost:8080/offensive`](https://localhost:8080/offensive/)
6. Log in as either `admin`/`[nsfw]` or `asdf`/`[tmbo]`.

The codebase is installed in the vm at `~/sites/tmbo`

Common Gotchas
--------------

Realtime Data: Self-signed certs can pose a problem for realtime, but launching Chrome with `--ignore-certificate-errors` will get it working.

Hotlinking Restrictions: tmbo's content protection is the same in development as it is in production, which means if you browse to your instance by IP, you're not going to see any images. You should either disable the relevant section in the nginx config, or add the ip address you use to the allowed hosts.

Help
----

Problems with the web site are frequently well-documented by error messages emitted by trigger-error. Administrators see this output as part of the rendered page, but it is also recorded to the httpd's logs. On the VM they are located in `~/logs/`.

If you need anything to get running, help can usually be had in #tmbotech on EFnet.

Further Reading
---------------

* `HACKING` — house coding standards, and how the include path and page structure work.
* `DEPLOYING` — how to deploy to a real server, how to get back out again, and the traps that have caused outages. Read it before touching production or the database.
