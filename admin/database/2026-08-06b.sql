-- Drop users.cookie_secret.  The remember cookie is keyed on the user's stored
-- password hash, so nothing reads this column.
--
-- Destructive: deploy the code first.  Any commit that still selects the column
-- fatals on every page that issues or verifies a remember cookie once it is gone.
--
-- Not reversible.  Cookies still signed with a per-user secret stop verifying;
-- cookies keyed on the password hash are unaffected.

ALTER TABLE users DROP COLUMN cookie_secret;
