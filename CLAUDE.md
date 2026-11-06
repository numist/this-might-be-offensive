# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

tmbo ("this might be offensive") is a long-lived image-sharing site written in hand-rolled
PHP. There is no framework, no dependency manager for the PHP side, and no build step —
files are served as they sit in the tree.

`HACKING` documents the house coding standards and is authoritative on style.
`DEPLOYING` documents how to deploy, what can be rolled back, and the traps that have
caused outages. Read both before making changes; this file covers what they don't.

## Runtime constraints

The deployed runtime is **PHP 5.5** (Ubuntu Trusty, nginx + php5-fpm). This is the single
most common source of broken changes:

- `hash_equals()` (5.6+) and `random_bytes()` (7+) **do not exist**. `functions.inc` has
  `tmbo_hash_equals()` and `tmbo_random_hex()` standing in for them. Use those.
- The removed `ext/mysql` API is in use throughout — `mysql_query`, `mysql_fetch_assoc`,
  `mysql_real_escape_string`. Do not migrate to PDO/mysqli piecemeal.
- Short open tags (`<?`, `<?=`) are used everywhere and `short_open_tag = On`. To lint a
  file you must pass `-d short_open_tag=On`.

Do not "modernize" incidentally. If a change needs a function, check it exists in 5.5.

## Common commands

There is **no test suite** — `npm test` in `realtime/` is an unimplemented placeholder,
and there is no PHP test harness. Verification is done by exercising the site.

```sh
vagrant up                       # full dev VM; site at https://localhost:8080/offensive
                                 # log in as admin/[nsfw] or asdf/[tmbo]
```

The VM installs the codebase at `~/sites/tmbo` and writes logs to `~/logs/`. Chrome needs
`--ignore-certificate-errors` for the realtime service to work against the self-signed cert.

Syntax-check without a VM:

```sh
docker run --rm -v "$PWD":/app -w /app php:5.6-cli \
  php -d short_open_tag=On -l offensive/assets/logn.inc
```

Schema lives in `admin/database/schema.sql`; migrations are dated files in the same
directory, applied by hand. `admin/database/populate.sql` seeds the two dev accounts.

## Architecture

**Every user-navigable page is a `.php` that opens with two lines**: `set_include_path()`
pointing at the web root, then `require_once("offensive/assets/header.inc")`. Included
files are `.inc`. Because include_path is the web root, every `require_once` in the
codebase is written web-root-relative (`offensive/assets/functions.inc`) regardless of the
including file's location — this is correct and load-bearing, not a bug.

`header.inc` is the entry point for essentially everything: it starts the session,
installs the error handler, forces https, and defines `tmbo_query()` and `me()`.

**Three layers share one set of business rules:**

- `offensive/assets/core.inc` — `core_*` functions. The actual business logic.
- `offensive/api.php` — `api_*` functions. Thin wrappers that validate arguments
  (`argvalidation.inc`), call `core_*`, and serialize via `assets/output/{json,php,plist,xml}.inc`.
  Routing parses `REQUEST_URI` as `api.php/<method>.<rtype>`. Docs are generated from the
  docblocks by `admin/docgen.php`.
- `offensive/content/*.inc` — the web UI. `index.php?c=<name>` includes
  `content/<name>.inc` and calls whichever of six optional hooks it defines, in order:
  `start()`, `title()`, `head()`, `head_post_js()`, `sidebar()`, `body()`. Each is invoked
  only if declared, so a content page implements just the ones it needs.

**`offensive/classes/`** holds the data objects (`User`, `Upload`, `Comment`, `Token`).
They cache aggressively in a class-level static store shared across all instances of the
same row, so constructing the same `User` twice costs one query. Each exposes `api_data()`
— an explicit allowlist of fields safe to serialize. Sensitive columns (`password`,
`cookie_secret`) live in the object's private `$data` and are deliberately absent from
`api_data()`; keep it that way when adding columns.

## Authentication

All of it is in `offensive/assets/logn.inc`. `login()` tries, in order: session →
remember cookie → rate-limit gate (`canLogIn()`) → username/password → API token. Each
returns `true`/`false`/`null`, where `null` means "not applicable, try the next".

The rate-limit gate sits *after* session and cookie auth, so those paths are never
throttled or logged to `failed_logins`.

The remember cookie is `userid.nonce.mac`, an HMAC keyed with a random per-user secret
(`users.cookie_secret`) plus a site pepper from `admin/.config`. It replaced a scheme
derived entirely from public data. Only `issueRememberCookie()` mints one, and only on a
password login with `rememberme` in the request — nothing silently upgrades an old cookie.

Passwords are unsalted `sha1($pw)`. This is a known weakness, not a pattern to copy.

## Secrets

`admin/.config` is an INI file, gitignored, parsed into the global `$config` by
`admin/mysqlConnectionInfo.inc`. It holds the database credentials and three auth secrets:
`remember_pepper`, `activation_salt`, `pwreset_salt`.

Fetch them with `tmbo_secret($key)` from `functions.inc`, which resolves **lazily** —
`api.php` and `activate.php` include `activationFunctions.inc` before
`mysqlConnectionInfo.inc`, so `$config` does not exist at parse time. It fails hard on a
missing or too-short value rather than falling back to a default; keep it that way.

**Never commit a value for any of these.** They were literals in this public repo once,
which made every account's remember cookie forgeable. `admin/configroot/` holds only the
Vagrant dev VM's templates — `vm_setup.sh` generates the dev secrets at provision time and
installs those templates on the dev VM and nowhere else.

## Traps that have caused outages

- **`DELETE FROM tokens` takes the whole site down.** The table mixes user API credentials
  with three internal per-user tokens the code creates lazily then assumes exist forever:
  `" tmbo"` and `" rss"` (leading space) and `"realtime"` (no leading space).
  `Token::getTokenRow()` treats any row count other than one — zero *or* duplicates — as
  "create a replacement" and dereferences the result without checking it. See `DEPLOYING`.
- **Sandbox and production share one database.** They differ in UI, not data. Any
  `UPDATE`/`DELETE`/`ALTER` run from sandbox lands on live data.
- **A 500 means a genuine PHP fatal.** `trigger_error(E_USER_ERROR)` renders
  `index.outoforder.php` with a *200*, so anything returning 500 is a parse error or a call
  to an undefined function. The trace is in `~/logs/thismight.be-error_log`.
- **Errors are only visible to admins.** Non-admins get the kaboom page. Everything goes to
  the log regardless, via `error_log()` in `tmbo_error_handler`.
- **System config is not version controlled.** `/etc/php5/...` and `/etc/nginx/...` are not
  in the repo, so two hosts on identical commits can behave differently. Session cookie
  flags are set in code (`session_set_cookie_params()`) for exactly this reason.
- **Bumping the session cookie name (`TMBOSESS<n>`, in `header.inc`, `logn.inc` and
  `logout.php` — all three must agree) logs the entire site out.** It is the only session
  invalidation available without root on the host. PHP emits `Set-Cookie` only when it
  *creates* a session, so changes to cookie flags do not reach existing sessions any
  other way.
