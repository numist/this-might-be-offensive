-- MySQL dump 10.11
--
-- Host: localhost    Database: tmbo
-- ------------------------------------------------------
-- Server version	5.0.51a-21-log

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
-- Table structure for table `failed_logins`
--

DROP TABLE IF EXISTS `failed_logins`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `failed_logins` (
  `id` int(11) NOT NULL auto_increment,
  `username` varchar(50) NOT NULL default '',
  `password` varchar(50) NOT NULL default '',
  `ip` varchar(15) NOT NULL default '',
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  PRIMARY KEY  (`id`),
  KEY `ip_timestamp` (`ip`,`timestamp`),
  KEY `username` (`username`,`timestamp`)
) ENGINE=InnoDB AUTO_INCREMENT=32842 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `hall_of_fame`
--

DROP TABLE IF EXISTS `hall_of_fame`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `hall_of_fame` (
  `id` int(11) NOT NULL auto_increment,
  `fileid` int(11) NOT NULL default '0',
  `votes` int(11) default NULL,
  `type` enum('hof','today') NOT NULL default 'hof',
  PRIMARY KEY  (`id`),
  KEY `fileid` (`fileid`)
) ENGINE=InnoDB AUTO_INCREMENT=184 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `ip_history`
--

DROP TABLE IF EXISTS `ip_history`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `ip_history` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) NOT NULL default '0',
  `ip` varchar(15) NOT NULL default '',
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  PRIMARY KEY  (`id`),
  KEY `userid_ip` (`userid`,`ip`)
) ENGINE=InnoDB AUTO_INCREMENT=4059315 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `maxxer_locations`
--

DROP TABLE IF EXISTS `maxxer_locations`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `maxxer_locations` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) NOT NULL default '0',
  `y` float default '0',
  `x` float default '0',
  `mapversion` varchar(10) default NULL,
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `userid_2` (`userid`),
  KEY `userid` (`userid`),
  KEY `mapversion_userid` (`mapversion`,`userid`)
) ENGINE=InnoDB AUTO_INCREMENT=8252 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `merch_buyers`
--

DROP TABLE IF EXISTS `merch_buyers`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
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
) ENGINE=InnoDB AUTO_INCREMENT=503 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `merch_items`
--

DROP TABLE IF EXISTS `merch_items`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `merch_items` (
  `id` int(11) NOT NULL auto_increment,
  `description` varchar(255) NOT NULL default '',
  `price` float NOT NULL default '0',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `merch_order_item_options`
--

DROP TABLE IF EXISTS `merch_order_item_options`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `merch_order_item_options` (
  `id` int(11) NOT NULL auto_increment,
  `order_item_id` int(11) NOT NULL default '0',
  `option_name` varchar(50) NOT NULL default '',
  `option_value` varchar(50) NOT NULL default '',
  PRIMARY KEY  (`id`),
  KEY `order_item_id` (`order_item_id`)
) ENGINE=InnoDB AUTO_INCREMENT=89 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `merch_order_items`
--

DROP TABLE IF EXISTS `merch_order_items`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `merch_order_items` (
  `id` int(11) NOT NULL auto_increment,
  `order_id` int(11) NOT NULL default '0',
  `item_id` int(11) NOT NULL default '0',
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  PRIMARY KEY  (`id`),
  KEY `order_id` (`order_id`)
) ENGINE=InnoDB AUTO_INCREMENT=89 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `merch_orders`
--

DROP TABLE IF EXISTS `merch_orders`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
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
) ENGINE=InnoDB AUTO_INCREMENT=492 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `offensive_comments`
--

DROP TABLE IF EXISTS `offensive_comments`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
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
  KEY `userid_fileid` (`userid`,`fileid`),
  KEY `userid_timestamp` (`userid`,`timestamp`),
  KEY `userid_fileid_commentid` (`id`,`userid`,`fileid`)
) ENGINE=MyISAM AUTO_INCREMENT=276172015 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `offensive_count_cache`
--

DROP TABLE IF EXISTS `offensive_count_cache`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
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
  UNIQUE KEY `threadid_2` (`threadid`),
  KEY `good` (`good`),
  KEY `ca_timestamp` (`timestamp`),
  KEY `ca_comments` (`comments`)
) ENGINE=InnoDB AUTO_INCREMENT=380713 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `offensive_messages`
--

DROP TABLE IF EXISTS `offensive_messages`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `offensive_messages` (
  `id` int(11) NOT NULL auto_increment,
  `to` int(11) NOT NULL default '0',
  `from` int(11) NOT NULL default '0',
  `status` enum('read','unread') NOT NULL default 'read',
  `body` mediumtext NOT NULL,
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP,
  PRIMARY KEY  (`id`),
  KEY `to` (`to`,`from`,`status`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `offensive_squelch`
--

DROP TABLE IF EXISTS `offensive_squelch`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `offensive_squelch` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) NOT NULL default '0',
  `squelched` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `userid` (`userid`,`squelched`)
) ENGINE=InnoDB AUTO_INCREMENT=5585 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `offensive_subscriptions`
--

DROP TABLE IF EXISTS `offensive_subscriptions`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `offensive_subscriptions` (
  `userid` int(11) NOT NULL default '0',
  `fileid` int(11) NOT NULL default '0',
  `commentid` int(11) default NULL,
  KEY `u_f_c` (`userid`,`fileid`,`commentid`),
  KEY `sub_fileid` (`fileid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `offensive_uploads`
--

DROP TABLE IF EXISTS `offensive_uploads`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `offensive_uploads` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) NOT NULL default '0',
  `filename` varchar(255) NOT NULL default '',
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `ip` varchar(16) default NULL,
  `nsfw` tinyint(4) NOT NULL,
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
  KEY `status_type_id` (`status`,`type`,`id`),
  KEY `t_t_id` (`type`,`timestamp`,`id`,`status`)
) ENGINE=InnoDB AUTO_INCREMENT=266080 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `old_preference_names_values`
--

DROP TABLE IF EXISTS `old_preference_names_values`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `old_preference_names_values` (
  `id` int(11) NOT NULL auto_increment,
  `value` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `old_user_preferences`
--

DROP TABLE IF EXISTS `old_user_preferences`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `old_user_preferences` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) NOT NULL default '0',
  `nameid` int(11) NOT NULL default '0',
  `valueid` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `userid_nameid` (`userid`,`nameid`)
) ENGINE=InnoDB AUTO_INCREMENT=36983 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `referrals`
--

DROP TABLE IF EXISTS `referrals`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `referrals` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) NOT NULL default '0',
  `referral_code` varchar(32) NOT NULL default '',
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `email` varchar(50) NOT NULL default '',
  PRIMARY KEY  (`id`),
  KEY `userid` (`userid`)
) ENGINE=InnoDB AUTO_INCREMENT=5232 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `user_notes`
--

DROP TABLE IF EXISTS `user_notes`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `user_notes` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) NOT NULL default '0',
  `notes` mediumtext NOT NULL,
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `user_preferences`
--

DROP TABLE IF EXISTS `user_preferences`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `user_preferences` (
  `id` int(11) NOT NULL auto_increment,
  `userid` varchar(50) NOT NULL default '',
  `prefname` varchar(50) NOT NULL default '',
  `value` text NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `userid_id` (`userid`,`id`)
) ENGINE=InnoDB AUTO_INCREMENT=32020 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `user_profile_attrs`
--

DROP TABLE IF EXISTS `user_profile_attrs`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `user_profile_attrs` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `user_profiles`
--

DROP TABLE IF EXISTS `user_profiles`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `user_profiles` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) NOT NULL default '0',
  `attr_name` varchar(255) NOT NULL default '',
  `attr_value` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `users`
--

DROP TABLE IF EXISTS `users`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `users` (
  `userid` int(11) NOT NULL auto_increment,
  `password` char(40) default NULL,
  `email` varchar(60) default NULL,
  `username` varchar(50) default NULL,
  `created` datetime NOT NULL default '0000-00-00 00:00:00',
  `account_status` set('admin','normal','locked','awaiting activation','memoriam') NOT NULL default 'awaiting activation',
  `ip` varchar(16) default NULL,
  `last_login_ip` varchar(15) default NULL,
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `referred_by` int(11) default NULL,
  PRIMARY KEY  (`userid`),
  UNIQUE KEY `username` (`username`),
  KEY `last_login_ip` (`last_login_ip`),
  KEY `referred_by` (`referred_by`),
  KEY `id_status` (`userid`,`account_status`),
  KEY `account_status` (`account_status`)
) ENGINE=InnoDB AUTO_INCREMENT=6431 DEFAULT CHARSET=latin1 PACK_KEYS=1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `vote_count_for_export`
--

DROP TABLE IF EXISTS `vote_count_for_export`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `vote_count_for_export` (
  `id` int(11) NOT NULL auto_increment,
  `fileid` int(11) NOT NULL default '0',
  `vote` varchar(15) NOT NULL default '',
  `count` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `fileid` (`fileid`)
) ENGINE=InnoDB AUTO_INCREMENT=399262 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `vote_stats`
--

DROP TABLE IF EXISTS `vote_stats`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `vote_stats` (
  `id` int(11) NOT NULL auto_increment,
  `userid` int(11) default NULL,
  `value` int(11) NOT NULL default '0',
  `type` varchar(50) NOT NULL default '',
  `timestamp` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  PRIMARY KEY  (`id`),
  KEY `userid` (`userid`),
  KEY `userid_to_type` (`userid`,`type`),
  KEY `type_to_val` (`type`,`value`)
) ENGINE=InnoDB AUTO_INCREMENT=9805 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2009-07-11  7:52:14
