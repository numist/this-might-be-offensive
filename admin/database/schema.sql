-- MySQL dump 10.13  Distrib 5.1.49, for debian-linux-gnu (x86_64)
--
-- Host: localhost    Database: tmbo
-- ------------------------------------------------------
-- Server version	5.1.49-3

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
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `failed_logins` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(50) NOT NULL DEFAULT '',
  `password` varchar(50) NOT NULL DEFAULT '',
  `ip` varchar(15) NOT NULL DEFAULT '',
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `ip_timestamp` (`ip`,`timestamp`),
  KEY `username` (`username`,`timestamp`)
) ENGINE=InnoDB AUTO_INCREMENT=32846 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `ip_history`
--

DROP TABLE IF EXISTS `ip_history`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ip_history` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) NOT NULL DEFAULT '0',
  `ip` varchar(15) NOT NULL DEFAULT '',
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `userid_ip` (`userid`,`ip`)
) ENGINE=InnoDB AUTO_INCREMENT=4059355 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `maxxer_locations`
--

DROP TABLE IF EXISTS `maxxer_locations`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `maxxer_locations` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) NOT NULL DEFAULT '0',
  `y` float DEFAULT '0',
  `x` float DEFAULT '0',
  `mapversion` varchar(10) DEFAULT NULL,
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `userid_2` (`userid`),
  KEY `userid` (`userid`),
  KEY `mapversion_userid` (`mapversion`,`userid`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `merch_buyers`
--

DROP TABLE IF EXISTS `merch_buyers`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `merch_buyers` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `first_name` varchar(64) NOT NULL DEFAULT '',
  `last_name` varchar(64) NOT NULL DEFAULT '',
  `address_name` varchar(128) DEFAULT NULL,
  `street` varchar(200) NOT NULL DEFAULT '',
  `city` varchar(40) NOT NULL DEFAULT '',
  `state` varchar(40) NOT NULL DEFAULT '',
  `zip` varchar(20) NOT NULL DEFAULT '',
  `country` varchar(64) NOT NULL DEFAULT '',
  `email` varchar(127) NOT NULL DEFAULT '',
  `notify_version` varchar(10) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `merch_items`
--

DROP TABLE IF EXISTS `merch_items`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `merch_items` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `description` varchar(255) NOT NULL DEFAULT '',
  `price` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `merch_order_item_options`
--

DROP TABLE IF EXISTS `merch_order_item_options`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `merch_order_item_options` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `order_item_id` int(11) NOT NULL DEFAULT '0',
  `option_name` varchar(50) NOT NULL DEFAULT '',
  `option_value` varchar(50) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  KEY `order_item_id` (`order_item_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `merch_order_items`
--

DROP TABLE IF EXISTS `merch_order_items`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `merch_order_items` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `order_id` int(11) NOT NULL DEFAULT '0',
  `item_id` int(11) NOT NULL DEFAULT '0',
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `order_id` (`order_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `merch_orders`
--

DROP TABLE IF EXISTS `merch_orders`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `merch_orders` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `status` enum('pending','shipped') NOT NULL DEFAULT 'pending',
  `transaction_id` varchar(50) NOT NULL DEFAULT '',
  `amount` float NOT NULL DEFAULT '0',
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `buyer_id` int(11) NOT NULL DEFAULT '0',
  `payment_status` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `status` (`status`,`transaction_id`),
  KEY `buyer_id` (`buyer_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `offensive_comments`
--

DROP TABLE IF EXISTS `offensive_comments`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `offensive_comments` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) NOT NULL DEFAULT '0',
  `fileid` int(11) NOT NULL DEFAULT '0',
  `comment` text NOT NULL,
  `vote` enum('this is good','this is bad') DEFAULT NULL,
  `offensive` smallint(6) DEFAULT NULL,
  `repost` smallint(6) DEFAULT NULL,
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `user_ip` varchar(20) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `fileid` (`fileid`),
  KEY `vote` (`vote`),
  KEY `userid` (`userid`),
  KEY `userid_fileid` (`userid`,`fileid`),
  KEY `userid_timestamp` (`userid`,`timestamp`),
  KEY `userid_fileid_commentid` (`id`,`userid`,`fileid`)
) ENGINE=MyISAM AUTO_INCREMENT=276172016 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `offensive_count_cache`
--

DROP TABLE IF EXISTS `offensive_count_cache`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `offensive_count_cache` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `threadid` int(10) unsigned NOT NULL DEFAULT '0',
  `good` int(10) unsigned NOT NULL DEFAULT '0',
  `bad` int(10) unsigned NOT NULL DEFAULT '0',
  `tmbo` int(10) unsigned NOT NULL DEFAULT '0',
  `comments` int(10) unsigned NOT NULL DEFAULT '0',
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `repost` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `threadid_2` (`threadid`),
  KEY `good` (`good`),
  KEY `ca_timestamp` (`timestamp`),
  KEY `ca_comments` (`comments`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `offensive_squelch`
--

DROP TABLE IF EXISTS `offensive_squelch`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `offensive_squelch` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) NOT NULL DEFAULT '0',
  `squelched` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `userid` (`userid`,`squelched`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `offensive_subscriptions`
--

DROP TABLE IF EXISTS `offensive_subscriptions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `offensive_subscriptions` (
  `userid` int(11) NOT NULL DEFAULT '0',
  `fileid` int(11) NOT NULL DEFAULT '0',
  `commentid` int(11) DEFAULT NULL,
  KEY `u_f_c` (`userid`,`fileid`,`commentid`),
  KEY `sub_fileid` (`fileid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `offensive_uploads`
--

DROP TABLE IF EXISTS `offensive_uploads`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `offensive_uploads` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) NOT NULL DEFAULT '0',
  `filename` varchar(255) NOT NULL DEFAULT '',
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `ip` varchar(16) DEFAULT NULL,
  `nsfw` tinyint(4) NOT NULL,
  `hash` varchar(32) DEFAULT NULL,
  `tmbo` tinyint(4) NOT NULL DEFAULT '0',
  `type` enum('image','topic','audio','video','avatar') NOT NULL DEFAULT 'image',
  `status` enum('normal','pending','expired') NOT NULL DEFAULT 'normal',
  PRIMARY KEY (`id`),
  KEY `timestamp` (`timestamp`),
  KEY `filename` (`filename`),
  KEY `userid` (`userid`),
  KEY `hash` (`hash`),
  KEY `type` (`type`),
  KEY `status` (`status`),
  KEY `type_userid` (`type`,`userid`),
  KEY `status_type_id` (`status`,`type`,`id`),
  KEY `t_t_id` (`type`,`timestamp`,`id`,`status`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `referrals`
--

DROP TABLE IF EXISTS `referrals`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `referrals` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `userid` int(11) NOT NULL DEFAULT '0',
  `referral_code` varchar(32) NOT NULL DEFAULT '',
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `email` varchar(50) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  KEY `userid` (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `tokens`
--

DROP TABLE IF EXISTS `tokens`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `tokens` (
  `tokenid` varchar(64) NOT NULL,
  `userid` int(11) NOT NULL,
  `issued_to` varchar(255) NOT NULL,
  `last_used` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00' ON UPDATE CURRENT_TIMESTAMP,
  `issue_date` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`tokenid`),
  UNIQUE KEY `tokenid` (`tokenid`),
  KEY `userid` (`userid`),
  KEY `userid_to_agent` (`userid`,`issued_to`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `users`
--

DROP TABLE IF EXISTS `users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `users` (
  `userid` int(11) NOT NULL AUTO_INCREMENT,
  `password` char(40) DEFAULT NULL,
  `email` varchar(60) DEFAULT NULL,
  `username` varchar(50) DEFAULT NULL,
  `created` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `account_status` set('admin','normal','locked','awaiting activation','memoriam') NOT NULL DEFAULT 'awaiting activation',
  `ip` varchar(16) DEFAULT NULL,
  `last_login_ip` varchar(15) DEFAULT NULL,
  `timestamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `referred_by` int(11) DEFAULT NULL,
  PRIMARY KEY (`userid`),
  UNIQUE KEY `username` (`username`),
  KEY `last_login_ip` (`last_login_ip`),
  KEY `referred_by` (`referred_by`),
  KEY `id_status` (`userid`,`account_status`),
  KEY `account_status` (`account_status`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=latin1 PACK_KEYS=1;
/*!40101 SET character_set_client = @saved_cs_client */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2011-10-08  3:41:18
