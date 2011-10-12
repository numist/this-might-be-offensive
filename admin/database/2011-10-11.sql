CREATE TABLE `tokens` (
  `tokenid` varchar(64) NOT NULL,
  `userid` int(11) NOT NULL,
  `issued_to` varchar(255) NOT NULL,
  `last_used` timestamp DEFAULT 0 ON UPDATE CURRENT_TIMESTAMP,
  `issue_date` timestamp NOT NULL,
  PRIMARY KEY  (`tokenid`),
  KEY `userid` (`userid`),
  KEY `userid_to_agent` (`userid`,`issued_to`),
  UNIQUE (tokenid)
) ENGINE=InnoDB;