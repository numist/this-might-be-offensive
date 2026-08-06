-- Drop users.cookie_secret.  The remember cookie is keyed on the user's stored
-- password hash, so nothing reads this column.
--
-- Destructive: deploy the code first.  A commit that still selects the column by
-- name fatals when it is gone, which is the "remember me" box on a password login.
-- Cookie verification degrades quietly instead, since it selects the whole row.
--
-- Not reversible.  Cookies still signed with a per-user secret stop verifying;
-- cookies keyed on the password hash are unaffected.

ALTER TABLE users DROP COLUMN cookie_secret;
