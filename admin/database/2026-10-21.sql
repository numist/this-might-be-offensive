-- Widen the IP columns to fit IPv6 addresses (45 covers IPv4-mapped IPv6), and
-- move offensive_comments off MyISAM.
--
-- The old MySQL wasn't strict, so IPv6 addresses were silently truncated to the
-- column width.  MariaDB is strict by default; the code now asks for the old
-- behavior per session (see openDbConnection()), so this isn't needed for the
-- site to work, but without it addresses keep getting truncated.  Rows that were
-- already truncated stay that way.
--
-- failed_logins.ip was widened on production by hand at some point; this makes
-- databases built from schema.sql match.
--
-- Safe to run before or after deploying the code.
--
-- The users, offensive_uploads and failed_logins changes are in-place metadata
-- changes.  offensive_comments is not: converting it copies the whole table
-- (2.6 GB and 19M rows on production), blocking comment writes until it
-- finishes.  Building its indexes needs more scratch space than a tmpfs /tmp
-- usually has ("Temp file write failure"), so point innodb_tmpdir at real disk
-- first, as a user with the FILE privilege:
--
--   SET GLOBAL innodb_tmpdir = '/var/tmp';

ALTER TABLE users MODIFY ip varchar(45) DEFAULT NULL,
                  MODIFY last_login_ip varchar(45) DEFAULT NULL;
ALTER TABLE offensive_uploads MODIFY ip varchar(45) DEFAULT NULL;
ALTER TABLE failed_logins MODIFY ip varchar(45) NOT NULL DEFAULT '';
ALTER TABLE offensive_comments ENGINE=InnoDB,
                               MODIFY user_ip varchar(45) DEFAULT NULL;
