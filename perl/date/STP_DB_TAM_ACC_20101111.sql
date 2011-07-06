-- MySQL dump 10.9
--
-- Host: localhost    Database: dqmsDB
-- ------------------------------------------------------
-- Server version	4.1.20

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `TB_MACCESSC`
--

DROP TABLE IF EXISTS `TB_MACCESSC`;
CREATE TABLE `TB_MACCESSC` (
  `BRANCHID` char(3) NOT NULL default '',
  `PCFIP` varchar(15) default NULL,
  `PCFBIP` int(10) unsigned NOT NULL default '0',
  `PCFTYPE` int(2) unsigned NOT NULL default '0',
  `BSCID` char(2) NOT NULL default '',
  `BTSID` varchar(4) NOT NULL default '',
  `SYSID` char(2) NOT NULL default '',
  `BRANCHNAME` varchar(20) default NULL,
  `PCFNAME` varchar(20) default NULL,
  `PCFTYPENAME` varchar(20) default NULL,
  `BSCIDNAME` varchar(20) default NULL,
  `BTSIDNAME` varchar(20) default NULL,
  `BSMSC` varchar(12) default NULL,
  `UPDATEDATE` varchar(14) default NULL,
  `SYSIDNAME` varchar(20) default NULL,
  PRIMARY KEY  (`BRANCHID`,`PCFBIP`,`PCFTYPE`,`BSCID`,`BTSID`,`SYSID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `TB_MACCESSC`
--


/*!40000 ALTER TABLE `TB_MACCESSC` DISABLE KEYS */;
LOCK TABLES `TB_MACCESSC` WRITE;
INSERT INTO `TB_MACCESSC` VALUES ('3','10.160.27.196',178265028,4,'11','0','6','','','','','','','0',''),('3','10.160.27.199',178265031,4,'11','0','6','','','','','','','0',''),('3','10.160.28.9',178265097,4,'0','15','26','','','','','','','0',''),('3','10.160.27.197',178265029,4,'11','0','6','','','','','','','0',''),('3','10.160.27.199',178265031,4,'9','5','18','','','','','','','0',''),('3','10.160.28.9',178265097,4,'11','20','26','','','','','','','0',''),('3','10.160.28.11',178265099,4,'11','20','26','','','','','','','0',''),('3','10.160.28.11',178265099,4,'0','63','26','','','','','','','0',''),('3','10.160.28.10',178265098,4,'11','0','26','','','','','','','0',''),('3','10.160.28.10',178265098,4,'0','63','26','','','','','','','0',''),('3','10.160.28.11',178265099,4,'0','15','26','','','','','','','0',''),('3','10.160.28.10',178265098,4,'11','20','26','','','','','','','0',''),('3','10.160.28.9',178265097,4,'0','63','26','','','','','','','0',''),('3','10.160.28.11',178265099,4,'11','0','26','','','','','','','0',''),('3','10.160.27.198',178265030,4,'10','1','6','','','','','','','0',''),('3','10.160.27.199',178265031,4,'0','47','14','','','','','','','0',''),('3','10.160.27.201',178265033,4,'5','63','1','','','','','','','0',''),('3','10.160.27.196',178265028,4,'10','1','6','','','','','','','0','');
UNLOCK TABLES;
/*!40000 ALTER TABLE `TB_MACCESSC` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

