UPDATE offensive_uploads SET nsfw = 1, timestamp = timestamp WHERE filename LIKE "%[nsfw]%";
UPDATE offensive_uploads SET tmbo = 1, timestamp = timestamp WHERE filename LIKE "%[tmbo]%";