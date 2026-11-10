# CLAUDE.md

This Might Be Offensive (tmbo) is a long-lived PHP image and comment community. The `develop` branch is what production serves.

Read these before changing anything substantial:

- `README.md`: local setup (Docker, or legacy Vagrant) and common gotchas.
- `HACKING`: house coding standards, the include path, and page structure.
- `DEPLOYING`: deploy order, migrations, the off switches, and traps that have caused outages. Read it before touching production or the database.

## Stack

PHP 8.4 (FPM), MariaDB 10.11, Redis (sessions and cache, via the bundled Predis client), nginx, and a Node.js Socket.IO service in `realtime/`. Front-end JavaScript is plain scripts with jQuery 1.7.1 and jQuery UI 1.8.17 already loaded on most pages.

## Local development

```sh
./scripts/dev-setup.sh          # safe to rerun; generates admin/.config.generated and certs
docker compose up               # add --profile full for cron jobs
```

Browse to https://localhost/offensive/ and log in as `admin` / `[nsfw]`. MariaDB is exposed on host port 3307 (database, user and password: `tmbo` / `tmbo` / `shortbus`; root password `shortbus`). The schema and `populate.sql` load only into an empty database volume (`services/db/init.sh`).

## Layout

The web root is the repository root.

```
offensive/          main application
  index.php         page router: ?c=<name> renders content/<name>.inc
  api.php           public API
  assets/           shared includes (header.inc, functions.inc, core.inc, classes.inc)
  classes/          domain models (user, upload, comment, token, link, tmbo)
  content/          page controllers
  templates/        HTML templates
  pages/            standalone pages, e.g. pic.php
  ui/               endpoints used by the site's own JavaScript
  js/               front-end scripts
styles/             CSS (theme.css, oldskool.css, grid.css, pic.css, ...)
admin/              admin tools, config loading, database/ (schema.sql and dated migrations)
realtime/           Node.js Socket.IO service
services/           Dockerfiles and config for nginx, php, db, cron, realtime
b/, gis/, contact/  board-style views, map, contact form
```

## Conventions

The full rules are in `HACKING`. The ones that matter most:

- Hard tabs for indentation, spaces for alignment past the last tab. Opening braces share the line with their condition, except a function's opening brace, which gets its own line.
- `.php` files are user-navigable. `.inc` files are includes, mostly in `offensive/assets/`. Pure-PHP files have no whitespace outside the `<?php ?>` block. Don't use short open tags.
- Every `.php` entry point starts with `set_include_path("<relative path to web root>");` then `require_once("offensive/assets/header.inc");`.
- All queries go through `tmbo_query()`. Escape user input with `sqlEscape()` and database output with `htmlEscape()`.
- Report errors with `trigger_error()` and an `E_USER_*` level. Admins see them on the page, and everything goes to the server log.
- `me()` returns the logged-in user.
- No new jQuery code. Write new or rewritten JavaScript with plain DOM APIs.
- Custom HTML attributes use `data-*`.
- Prefer reusing existing CSS (for example `:nth-child` striping) over adding classes.
- Add schema changes as a new dated file in `admin/database/`, and follow the migration order in `DEPLOYING`.

## Gotchas

- `GET offensive/pages/pic.php` moves the viewer's pickup link (`PickupLink::update()`). Never prefetch or prerender pic pages. Preload image URLs instead.
- The sandbox and production share one database (see `DEPLOYING`). Anything destructive you run there hits production data.
- Docker keeps uploads, zips and quarantine under `services/web/src/offensive/` (gitignored) and mounts them into the containers.
