START TRANSACTION;

DROP TABLE IF EXISTS `temp_prefs`;
CREATE TABLE `temp_prefs` (
  `id` int(11) NOT NULL auto_increment,
  `userid` varchar(50) NOT NULL default '',
  `prefname` varchar(50) NOT NULL default '',
  `value` text NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `userid_id` (`userid`,`id`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

INSERT INTO temp_prefs(userid, prefname, value) SELECT userid, names.value AS prefname, val.value AS value FROM user_preferences, preference_names_values names, preference_names_values val WHERE nameid=names.id AND val.id=valueid;

UPDATE temp_prefs SET prefname = 'hide_nsfw' WHERE prefname = 'hide nsfw';
UPDATE temp_prefs SET prefname = 'hide_tmbo' WHERE prefname = 'hide tmbo';

ALTER TABLE user_preferences RENAME TO old_user_preferences;
ALTER TABLE preference_names_values RENAME TO old_preference_names_values;

ALTER TABLE temp_prefs RENAME TO user_preferences;

COMMIT;