-- Remember-cookie authentication bypass.
--
-- The old remember cookie was derived entirely from public data (userid,
-- username, and a salt that was a literal in the public repository), so anyone
-- could compute a valid cookie for any account.  Cookies are now an HMAC keyed
-- with a random per-user secret plus a pepper from admin/.config.
--
-- Leaving cookie_secret NULL for existing users is deliberate: verification
-- refuses to run without a secret, so this invalidates every remember cookie
-- that has ever been issued.  Users are re-issued one on their next password
-- login with "remember me" checked.
--
-- Run this AFTER deploying the code, and only once admin/.config on the server
-- has remember_pepper, activation_salt, and pwreset_salt set.

ALTER TABLE users ADD COLUMN cookie_secret char(64) DEFAULT NULL;
UPDATE users SET timestamp = timestamp, cookie_secret = NULL;
