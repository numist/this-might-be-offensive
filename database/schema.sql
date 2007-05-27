-- MySQL dump 10.10
--
-- Host: mysql.rocketsheep.com    Database: themaxx
-- ------------------------------------------------------
-- Server version	4.1.16-standard-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `offensive_subscriptions`
--

DROP TABLE IF EXISTS `offensive_subscriptions`;
CREATE TABLE `offensive_subscriptions` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) NOT NULL default '0',
  `fileid` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `userid` (`userid`,`fileid`),
  KEY `fileid_index___added_by_dreamhost` (`fileid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `hall_of_fame`
--

DROP TABLE IF EXISTS `hall_of_fame`;
CREATE TABLE `hall_of_fame` (
  `id` int(11) NOT NULL auto_increment,
  `fileid` int(11) NOT NULL default '0',
  `votes` int(11) default NULL,
  `type` enum('hof','today') NOT NULL default 'hof',
  PRIMARY KEY  (`id`),
  KEY `fileid` (`fileid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `merch_order_item_options`
--

DROP TABLE IF EXISTS `merch_order_item_options`;
CREATE TABLE `merch_order_item_options` (
  `id` int(11) NOT NULL auto_increment,
  `order_item_id` int(11) NOT NULL default '0',
  `option_name` varchar(50) NOT NULL default '',
  `option_value` varchar(50) NOT NULL default '',
  PRIMARY KEY  (`id`),
  KEY `order_item_id` (`order_item_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `merch_buyers`
--

DROP TABLE IF EXISTS `merch_buyers`;
CREATE TABLE `merch_buyers` (
  `id` int(11) NOT NULL auto_increment,
  `first_name` varchar(64) NOT NULL default '',
  `last_name` varchar(64) NOT NULL default '',
  `address_name` varchar(128) default NULL,
  `street` varchar(200) NOT NULL default '',
  `city` varchar(40) NOT NULL default '',
  `state` varchar(40) NOT NULL default '',
  `zip` varchar(20) NOT NULL default '',
  `country` varchar(64) NOT NULL default '',
  `email` varchar(127) NOT NULL default '',
  `notify_version` varchar(10) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `offensive_count_cache`
--

DROP TABLE IF EXISTS `offensive_count_cache`;
CREATE TABLE `offensive_count_cache` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `threadid` int(10) unsigned NOT NULL default '0',
  `good` int(10) unsigned NOT NULL default '0',
  `bad` int(10) unsigned NOT NULL default '0',
  `tmbo` int(10) unsigned NOT NULL default '0',
  `comments` int(10) unsigned NOT NULL default '0',
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `repost` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `threadid_2` (`threadid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `merch_order_items`
--

DROP TABLE IF EXISTS `merch_order_items`;
CREATE TABLE `merch_order_items` (
  `id` int(11) NOT NULL auto_increment,
  `order_id` int(11) NOT NULL default '0',
  `item_id` int(11) NOT NULL default '0',
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  PRIMARY KEY  (`id`),
  KEY `order_id` (`order_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `vote_stats`
--

DROP TABLE IF EXISTS `vote_stats`;
CREATE TABLE `vote_stats` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) default NULL,
  `value` int(11) NOT NULL default '0',
  `type` varchar(50) NOT NULL default '',
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  PRIMARY KEY  (`id`),
  KEY `userid` (`userid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `maxxer_locations`
--

DROP TABLE IF EXISTS `maxxer_locations`;
CREATE TABLE `maxxer_locations` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) NOT NULL default '0',
  `x` float NOT NULL default '0',
  `y` float NOT NULL default '0',
  `mapversion` varchar(10) default NULL,
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `userid_2` (`userid`),
  KEY `userid` (`userid`),
  KEY `mapversion_userid` (`mapversion`,`userid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `user_profiles`
--

DROP TABLE IF EXISTS `user_profiles`;
CREATE TABLE `user_profiles` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) NOT NULL default '0',
  `attr_name` varchar(255) NOT NULL default '',
  `attr_value` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `merch_items`
--

DROP TABLE IF EXISTS `merch_items`;
CREATE TABLE `merch_items` (
  `id` int(11) NOT NULL auto_increment,
  `description` varchar(255) NOT NULL default '',
  `price` float NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `user_preferences`
--

DROP TABLE IF EXISTS `user_preferences`;
CREATE TABLE `user_preferences` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) NOT NULL default '0',
  `nameid` int(11) NOT NULL default '0',
  `valueid` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `userid_nameid` (`userid`,`nameid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `offensive_comments`
--

DROP TABLE IF EXISTS `offensive_comments`;
CREATE TABLE `offensive_comments` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) NOT NULL default '0',
  `fileid` int(11) NOT NULL default '0',
  `comment` text NOT NULL,
  `vote` enum('this is good','this is bad') default NULL,
  `offensive` smallint(6) default NULL,
  `repost` smallint(6) default NULL,
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `user_ip` varchar(20) default NULL,
  PRIMARY KEY  (`id`),
  KEY `fileid` (`fileid`),
  KEY `vote` (`vote`),
  KEY `userid` (`userid`),
  KEY `userid_fileid` (`userid`,`fileid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `referrals`
--

DROP TABLE IF EXISTS `referrals`;
CREATE TABLE `referrals` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) NOT NULL default '0',
  `referral_code` varchar(32) NOT NULL default '',
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `email` varchar(50) NOT NULL default '',
  PRIMARY KEY  (`id`),
  KEY `userid` (`userid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `merch_orders`
--

DROP TABLE IF EXISTS `merch_orders`;
CREATE TABLE `merch_orders` (
  `id` int(11) NOT NULL auto_increment,
  `status` enum('pending','shipped') NOT NULL default 'pending',
  `transaction_id` varchar(50) NOT NULL default '',
  `amount` float NOT NULL default '0',
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `buyer_id` int(11) NOT NULL default '0',
  `payment_status` varchar(50) default NULL,
  PRIMARY KEY  (`id`),
  KEY `status` (`status`,`transaction_id`),
  KEY `buyer_id` (`buyer_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `user_notes`
--

DROP TABLE IF EXISTS `user_notes`;
CREATE TABLE `user_notes` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) NOT NULL default '0',
  `notes` mediumtext NOT NULL,
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `offensive_uploads`
--

DROP TABLE IF EXISTS `offensive_uploads`;
CREATE TABLE `offensive_uploads` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) NOT NULL default '0',
  `filename` varchar(255) NOT NULL default '',
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `ip` varchar(16) default NULL,
  `nsfw` tinyint(4) default NULL,
  `hash` varchar(32) default NULL,
  `tmbo` tinyint(4) NOT NULL default '0',
  `type` enum('image','topic','audio','video','avatar') NOT NULL default 'image',
  `status` enum('normal','pending','expired') NOT NULL default 'normal',
  PRIMARY KEY  (`id`),
  KEY `timestamp` (`timestamp`),
  KEY `filename` (`filename`),
  KEY `userid` (`userid`),
  KEY `hash` (`hash`),
  KEY `type` (`type`),
  KEY `status` (`status`),
  KEY `type_userid` (`type`,`userid`),
  KEY `status_type_id` (`status`,`type`,`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `preference_names_values`
--

DROP TABLE IF EXISTS `preference_names_values`;
CREATE TABLE `preference_names_values` (
  `id` int(11) NOT NULL auto_increment,
  `value` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `offensive_bookmarks`
--

DROP TABLE IF EXISTS `offensive_bookmarks`;
CREATE TABLE `offensive_bookmarks` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) NOT NULL default '0',
  `fileid` int(11) NOT NULL default '0',
  `type` enum('auto','manual') NOT NULL default 'auto',
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `commentid` int(11) default NULL,
  PRIMARY KEY  (`id`),
  KEY `userid` (`userid`,`fileid`),
  KEY `fileid` (`fileid`),
  KEY `commentid` (`commentid`),
  KEY `userid_fileid` (`userid`,`fileid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `offensive_squelch`
--

DROP TABLE IF EXISTS `offensive_squelch`;
CREATE TABLE `offensive_squelch` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) NOT NULL default '0',
  `squelched` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `userid` (`userid`,`squelched`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `offensive_messages`
--

DROP TABLE IF EXISTS `offensive_messages`;
CREATE TABLE `offensive_messages` (
  `id` int(11) NOT NULL auto_increment,
  `to` int(11) NOT NULL default '0',
  `from` int(11) NOT NULL default '0',
  `status` enum('read','unread') NOT NULL default 'read',
  `body` mediumtext NOT NULL,
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP,
  PRIMARY KEY  (`id`),
  KEY `to` (`to`,`from`,`status`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `failed_logins`
--

DROP TABLE IF EXISTS `failed_logins`;
CREATE TABLE `failed_logins` (
  `id` int(11) NOT NULL auto_increment,
  `username` varchar(50) NOT NULL default '',
  `password` varchar(50) NOT NULL default '',
  `ip` varchar(15) NOT NULL default '',
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  PRIMARY KEY  (`id`),
  KEY `ip_timestamp` (`ip`,`timestamp`),
  KEY `username` (`username`,`timestamp`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `users`
--

DROP TABLE IF EXISTS `users`;
CREATE TABLE `users` (
  `userid` int(11) NOT NULL auto_increment,
  `password` varchar(20) default NULL,
  `email` varchar(60) default NULL,
  `username` varchar(50) default NULL,
  `created` datetime NOT NULL default '0000-00-00 00:00:00',
  `account_status` set('admin','normal','locked','awaiting activation') NOT NULL default 'awaiting activation',
  `ip` varchar(16) default NULL,
  `last_login_ip` varchar(15) default NULL,
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `referred_by` int(11) default NULL,
  PRIMARY KEY  (`userid`),
  UNIQUE KEY `username` (`username`),
  KEY `last_login_ip` (`last_login_ip`),
  KEY `referred_by` (`referred_by`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Table structure for table `user_profile_attrs`
--

DROP TABLE IF EXISTS `user_profile_attrs`;
CREATE TABLE `user_profile_attrs` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `ip_history`
--

DROP TABLE IF EXISTS `ip_history`;
CREATE TABLE `ip_history` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) NOT NULL default '0',
  `ip` varchar(15) NOT NULL default '',
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Table structure for table `vote_count_for_export`
--

DROP TABLE IF EXISTS `vote_count_for_export`;
CREATE TABLE `vote_count_for_export` (
  `id` int(11) NOT NULL auto_increment,
  `fileid` int(11) NOT NULL default '0',
  `vote` varchar(15) NOT NULL default '',
  `count` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `fileid` (`fileid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

