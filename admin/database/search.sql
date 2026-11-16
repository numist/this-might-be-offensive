-- Comment search index, in its own PostgreSQL database (not MariaDB).
--
-- MariaDB's offensive_comments stays the source of truth.  This holds only
-- what search needs: the comment's id, its author (so search can leave out
-- squelched users before paging) and its words.  Search returns ids and the
-- site loads the comments themselves from MariaDB.
--
-- admin/commentSearchSync.php runs this on every start, so it has to stay
-- safe to run again.  Everything here can be rebuilt from MariaDB with
-- `php admin/commentSearchSync.php <webroot> --rebuild`, so this database
-- needs no backups.

CREATE TABLE IF NOT EXISTS comments (
	id integer PRIMARY KEY,
	userid integer NOT NULL,
	tsv tsvector NOT NULL
);

-- fastupdate off: a search never has to read a pending list that the sync
-- hasn't merged yet.  New comments arrive slowly enough to index directly.
CREATE INDEX IF NOT EXISTS comments_tsv ON comments USING gin (tsv) WITH (fastupdate = off);

-- Newest-first searches either walk the comments backwards checking each one
-- (fast for common words) or look the words up in the index and sort (fast
-- for rare ones).  The planner picks by how many comments it thinks match,
-- and with the default statistics it thinks a rare word it has no figures
-- for matches thousands, walks, and takes a second or more.  Tracking more
-- words fixes that.  ANALYZE then samples 3M comments, about 10 seconds.
DO $$
BEGIN
	IF (SELECT attstattarget FROM pg_attribute
	    WHERE attrelid = 'comments'::regclass AND attname = 'tsv') IS DISTINCT FROM 10000 THEN
		ALTER TABLE comments ALTER tsv SET STATISTICS 10000;
	END IF;
END $$;

-- The highest offensive_comments.id the sync has read.  Vote-only comments
-- have no text and aren't stored, so max(comments.id) can't stand in for it.
CREATE TABLE IF NOT EXISTS sync_state (
	one boolean PRIMARY KEY DEFAULT true CHECK (one),
	last_id integer NOT NULL
);
INSERT INTO sync_state (last_id) VALUES (0) ON CONFLICT DO NOTHING;
