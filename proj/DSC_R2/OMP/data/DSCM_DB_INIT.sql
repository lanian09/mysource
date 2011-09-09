-- MySQL dump 10.13  Distrib 5.1.32, for pc-solaris2.10 (i386)
--
-- Host: localhost    Database: DSCM
-- ------------------------------------------------------
-- Server version	5.1.32-log

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
-- Table structure for table `INI_VALUES`
--

DROP TABLE IF EXISTS `INI_VALUES`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `INI_VALUES` (
  `time_stamp` datetime DEFAULT NULL,
  `se_ip` varchar(20) DEFAULT NULL,
  `value_type` smallint(6) DEFAULT NULL,
  `value_key` varchar(80) DEFAULT NULL,
  `value` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `RPT_LUR`
--

DROP TABLE IF EXISTS `RPT_LUR`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `RPT_LUR` (
  `TIME_STAMP` datetime DEFAULT NULL,
  `RECORD_SOURCE` int(11) DEFAULT NULL,
  `LINK_ID` tinyint(4) DEFAULT NULL,
  `GENERATOR_ID` tinyint(4) DEFAULT NULL,
  `GLBL_USG_CNT_ID` int(11) DEFAULT NULL,
  `CONFIGURED_DURATION` int(11) DEFAULT NULL,
  `DURATION` int(11) DEFAULT NULL,
  `END_TIME` int(11) DEFAULT NULL,
  `UPSTREAM_VOLUME` int(11) DEFAULT NULL,
  `DOWNSTREAM_VOLUME` int(11) DEFAULT NULL,
  `SESSIONS` int(11) DEFAULT NULL,
  `SECONDS` int(11) DEFAULT NULL,
  `CONCURRENT_SESSIONS` int(11) DEFAULT NULL,
  `ACTIVE_SUBSCRIBERS` int(11) DEFAULT NULL,
  `TOTAL_ACTIVE_SUBSCRIBERS` int(11) DEFAULT NULL,
  KEY `RPT_LUR_I1` (`END_TIME`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `alarm_code`
--

DROP TABLE IF EXISTS `alarm_code`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `alarm_code` (
  `alarm_name` varchar(32) NOT NULL DEFAULT '',
  `alarm_category` varchar(40) NOT NULL DEFAULT '0',
  PRIMARY KEY (`alarm_name`,`alarm_category`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `alarm_code_help`
--

DROP TABLE IF EXISTS `alarm_code_help`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `alarm_code_help` (
  `alarm_code` char(10) NOT NULL,
  `category` smallint(6) DEFAULT NULL,
  `help_file_name` varchar(20) DEFAULT NULL,
  `information` varchar(40) DEFAULT NULL,
  PRIMARY KEY (`alarm_code`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `alarm_history`
--

DROP TABLE IF EXISTS `alarm_history`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `alarm_history` (
  `system_type` varchar(16) NOT NULL DEFAULT '',
  `system_group` varchar(16) NOT NULL DEFAULT '',
  `system_name` varchar(16) NOT NULL DEFAULT '',
  `alarm_type` smallint(6) unsigned NOT NULL DEFAULT '0',
  `alarm_code` smallint(6) DEFAULT '0',
  `alarm_level` smallint(6) unsigned NOT NULL DEFAULT '0',
  `occur_flag` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `alarm_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `information` varchar(64) DEFAULT NULL,
  KEY `idx1` (`alarm_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `current_alarm`
--

DROP TABLE IF EXISTS `current_alarm`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `current_alarm` (
  `system_type` varchar(16) NOT NULL DEFAULT '',
  `system_group` varchar(16) NOT NULL DEFAULT '',
  `system_name` varchar(16) NOT NULL DEFAULT '',
  `alarm_type` smallint(6) unsigned NOT NULL DEFAULT '0',
  `alarm_code` smallint(6) DEFAULT '0',
  `alarm_level` smallint(6) unsigned NOT NULL DEFAULT '0',
  `alarm_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `information` varchar(64) NOT NULL DEFAULT '',
  `mask` decimal(3,0) DEFAULT '0',
  PRIMARY KEY (`system_type`,`system_group`,`system_name`,`alarm_type`,`information`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `fault_5minute_statistics`
--

DROP TABLE IF EXISTS `fault_5minute_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `fault_5minute_statistics` (
  `system_type` varchar(16) NOT NULL DEFAULT '',
  `system_name` varchar(16) NOT NULL DEFAULT '',
  `cpu_min_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `cpu_maj_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `cpu_cri_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `mem_min_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `mem_maj_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `mem_cri_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `etc_hw_min_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `etc_hw_maj_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `etc_hw_cri_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `proc_min_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `proc_maj_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `proc_cri_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `stat_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  PRIMARY KEY (`stat_date`,`system_type`,`system_name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `fault_day_statistics`
--

DROP TABLE IF EXISTS `fault_day_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `fault_day_statistics` (
  `system_type` varchar(16) NOT NULL DEFAULT '',
  `system_name` varchar(16) NOT NULL DEFAULT '',
  `cpu_min_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `cpu_maj_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `cpu_cri_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `mem_min_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `mem_maj_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `mem_cri_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `etc_hw_min_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `etc_hw_maj_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `etc_hw_cri_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `proc_min_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `proc_maj_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `proc_cri_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `stat_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  PRIMARY KEY (`stat_date`,`system_type`,`system_name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `fault_hour_statistics`
--

DROP TABLE IF EXISTS `fault_hour_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `fault_hour_statistics` (
  `system_type` varchar(16) NOT NULL DEFAULT '',
  `system_name` varchar(16) NOT NULL DEFAULT '',
  `cpu_min_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `cpu_maj_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `cpu_cri_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `mem_min_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `mem_maj_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `mem_cri_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `etc_hw_min_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `etc_hw_maj_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `etc_hw_cri_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `proc_min_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `proc_maj_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `proc_cri_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `stat_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  PRIMARY KEY (`stat_date`,`system_type`,`system_name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `fault_month_statistics`
--

DROP TABLE IF EXISTS `fault_month_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `fault_month_statistics` (
  `system_type` varchar(16) NOT NULL DEFAULT '',
  `system_name` varchar(16) NOT NULL DEFAULT '',
  `cpu_min_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `cpu_maj_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `cpu_cri_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `mem_min_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `mem_maj_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `mem_cri_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `etc_hw_min_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `etc_hw_maj_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `etc_hw_cri_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `proc_min_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `proc_maj_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `proc_cri_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `stat_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  PRIMARY KEY (`stat_date`,`system_type`,`system_name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `fault_week_statistics`
--

DROP TABLE IF EXISTS `fault_week_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `fault_week_statistics` (
  `system_type` varchar(16) NOT NULL DEFAULT '',
  `system_name` varchar(16) NOT NULL DEFAULT '',
  `cpu_min_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `cpu_maj_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `cpu_cri_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `mem_min_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `mem_maj_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `mem_cri_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `etc_hw_min_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `etc_hw_maj_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `etc_hw_cri_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `proc_min_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `proc_maj_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `proc_cri_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `stat_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  PRIMARY KEY (`stat_date`,`system_type`,`system_name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `flow_5minute_statistics`
--

DROP TABLE IF EXISTS `flow_5minute_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `flow_5minute_statistics` (
  `system_name` varchar(10) NOT NULL,
  `avg_flow` int(11) DEFAULT '0',
  `min_flow` int(11) DEFAULT '0',
  `max_flow` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT NULL,
  KEY `flow_idx` (`system_name`,`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `flow_day_statistics`
--

DROP TABLE IF EXISTS `flow_day_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `flow_day_statistics` (
  `system_name` varchar(10) NOT NULL,
  `avg_flow` int(11) DEFAULT '0',
  `min_flow` int(11) DEFAULT '0',
  `max_flow` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `flow_hour_statistics`
--

DROP TABLE IF EXISTS `flow_hour_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `flow_hour_statistics` (
  `system_name` varchar(10) NOT NULL,
  `avg_flow` int(11) DEFAULT '0',
  `min_flow` int(11) DEFAULT '0',
  `max_flow` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `flow_month_statistics`
--

DROP TABLE IF EXISTS `flow_month_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `flow_month_statistics` (
  `system_name` varchar(10) NOT NULL,
  `avg_flow` int(11) DEFAULT '0',
  `min_flow` int(11) DEFAULT '0',
  `max_flow` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `flow_report_imsi`
--

DROP TABLE IF EXISTS `flow_report_imsi`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `flow_report_imsi` (
  `system_name` varchar(10) NOT NULL,
  `ReportTime` int(11) DEFAULT '0',
  `StringTime` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `FlowNum` int(11) unsigned DEFAULT '0',
  KEY `flow_imsi1` (`system_name`,`ReportTime`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `flow_week_statistics`
--

DROP TABLE IF EXISTS `flow_week_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `flow_week_statistics` (
  `system_name` varchar(10) NOT NULL,
  `avg_flow` int(11) DEFAULT '0',
  `min_flow` int(11) DEFAULT '0',
  `max_flow` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `ip_code_tbl`
--

DROP TABLE IF EXISTS `ip_code_tbl`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `ip_code_tbl` (
  `type` int(11) DEFAULT NULL,
  `code` varchar(16) DEFAULT NULL,
  `name` varchar(64) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `leg_5minute_statistics`
--

DROP TABLE IF EXISTS `leg_5minute_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `leg_5minute_statistics` (
  `system_name` varchar(10) DEFAULT NULL,
  `pdsn_ip` varchar(18) DEFAULT NULL,
  `rx_cnt` int(11) DEFAULT NULL,
  `start` int(11) DEFAULT NULL,
  `interim` int(11) DEFAULT NULL,
  `disconnect` int(11) DEFAULT NULL,
  `STOP` int(11) DEFAULT NULL,
  `start_logon_cnt` int(11) DEFAULT NULL,
  `int_logon_cnt` int(11) DEFAULT NULL,
  `disc_logon_cnt` int(11) DEFAULT NULL,
  `logout_cnt` int(11) DEFAULT NULL,
  `stat_cnt` int(11) DEFAULT NULL,
  `stat_date` datetime DEFAULT NULL,
  `stat_week` varchar(3) DEFAULT NULL,
  KEY `update_idx` (`system_name`,`pdsn_ip`,`stat_date`),
  KEY `select_idx` (`system_name`,`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `leg_day_statistics`
--

DROP TABLE IF EXISTS `leg_day_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `leg_day_statistics` (
  `system_name` varchar(10) DEFAULT NULL,
  `pdsn_ip` varchar(18) DEFAULT NULL,
  `rx_cnt` int(11) DEFAULT NULL,
  `start` int(11) DEFAULT NULL,
  `interim` int(11) DEFAULT NULL,
  `disconnect` int(11) DEFAULT NULL,
  `STOP` int(11) DEFAULT NULL,
  `start_logon_cnt` int(11) DEFAULT NULL,
  `int_logon_cnt` int(11) DEFAULT NULL,
  `disc_logon_cnt` int(11) DEFAULT NULL,
  `logout_cnt` int(11) DEFAULT NULL,
  `stat_cnt` int(11) DEFAULT NULL,
  `stat_date` datetime DEFAULT NULL,
  `stat_week` varchar(3) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
  KEY `update_idx` (`system_name`,`pdsn_ip`,`stat_date`),
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `leg_history`
--

DROP TABLE IF EXISTS `leg_history`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `leg_history` (
  `system_name` varchar(8) DEFAULT NULL,
  `login_cnt` int(10) unsigned DEFAULT '0',
  `logout_cnt` int(10) unsigned DEFAULT '0',
  `login_succ` int(10) unsigned DEFAULT '0',
  `logout_succ` int(10) unsigned DEFAULT '0',
  `rrstart_cnt` int(10) unsigned DEFAULT '0',
  `rrinterim_cnt` int(10) unsigned DEFAULT '0',
  `rrstop_cnt` int(10) unsigned DEFAULT '0',
  `log_date` int(10) unsigned NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `leg_hour_statistics`
--

DROP TABLE IF EXISTS `leg_hour_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `leg_hour_statistics` (
  `system_name` varchar(10) DEFAULT NULL,
  `pdsn_ip` varchar(18) DEFAULT NULL,
  `rx_cnt` int(11) DEFAULT NULL,
  `start` int(11) DEFAULT NULL,
  `interim` int(11) DEFAULT NULL,
  `disconnect` int(11) DEFAULT NULL,
  `STOP` int(11) DEFAULT NULL,
  `start_logon_cnt` int(11) DEFAULT NULL,
  `int_logon_cnt` int(11) DEFAULT NULL,
  `disc_logon_cnt` int(11) DEFAULT NULL,
  `logout_cnt` int(11) DEFAULT NULL,
  `stat_cnt` int(11) DEFAULT NULL,
  `stat_date` datetime DEFAULT NULL,
  `stat_week` varchar(3) DEFAULT NULL
  KEY `update_idx` (`system_name`,`pdsn_ip`,`stat_date`),
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `leg_month_statistics`
--

DROP TABLE IF EXISTS `leg_month_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `leg_month_statistics` (
  `system_name` varchar(10) DEFAULT NULL,
  `pdsn_ip` varchar(18) DEFAULT NULL,
  `rx_cnt` int(11) DEFAULT NULL,
  `start` int(11) DEFAULT NULL,
  `interim` int(11) DEFAULT NULL,
  `disconnect` int(11) DEFAULT NULL,
  `STOP` int(11) DEFAULT NULL,
  `start_logon_cnt` int(11) DEFAULT NULL,
  `int_logon_cnt` int(11) DEFAULT NULL,
  `disc_logon_cnt` int(11) DEFAULT NULL,
  `logout_cnt` int(11) DEFAULT NULL,
  `stat_cnt` int(11) DEFAULT NULL,
  `stat_date` datetime DEFAULT NULL,
  `stat_week` varchar(3) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `leg_week_statistics`
--

DROP TABLE IF EXISTS `leg_week_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `leg_week_statistics` (
  `system_name` varchar(10) DEFAULT NULL,
  `pdsn_ip` varchar(18) DEFAULT NULL,
  `rx_cnt` int(11) DEFAULT NULL,
  `start` int(11) DEFAULT NULL,
  `interim` int(11) DEFAULT NULL,
  `disconnect` int(11) DEFAULT NULL,
  `STOP` int(11) DEFAULT NULL,
  `start_logon_cnt` int(11) DEFAULT NULL,
  `int_logon_cnt` int(11) DEFAULT NULL,
  `disc_logon_cnt` int(11) DEFAULT NULL,
  `logout_cnt` int(11) DEFAULT NULL,
  `stat_cnt` int(11) DEFAULT NULL,
  `stat_date` datetime DEFAULT NULL,
  `stat_week` varchar(3) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `load_5minute_statistics`
--

DROP TABLE IF EXISTS `load_5minute_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `load_5minute_statistics` (
  `system_type` varchar(16) NOT NULL DEFAULT '',
  `system_name` varchar(16) NOT NULL DEFAULT '',
  `avr_cpu0` int(10) unsigned NOT NULL DEFAULT '0',
  `max_cpu0` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_cpu1` int(10) unsigned NOT NULL DEFAULT '0',
  `max_cpu1` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_cpu2` int(10) unsigned NOT NULL DEFAULT '0',
  `max_cpu2` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_cpu3` int(10) unsigned NOT NULL DEFAULT '0',
  `max_cpu3` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_memory` int(10) unsigned NOT NULL DEFAULT '0',
  `max_memory` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_disk` int(10) unsigned NOT NULL DEFAULT '0',
  `max_disk` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_msgQ` int(10) unsigned NOT NULL DEFAULT '0',
  `max_msgQ` int(10) unsigned NOT NULL DEFAULT '0',
  `stat_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  PRIMARY KEY (`stat_date`,`system_type`,`system_name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `load_day_statistics`
--

DROP TABLE IF EXISTS `load_day_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `load_day_statistics` (
  `system_type` varchar(16) NOT NULL DEFAULT '',
  `system_name` varchar(16) NOT NULL DEFAULT '',
  `avr_cpu0` int(10) unsigned NOT NULL DEFAULT '0',
  `max_cpu0` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_cpu1` int(10) unsigned NOT NULL DEFAULT '0',
  `max_cpu1` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_cpu2` int(10) unsigned NOT NULL DEFAULT '0',
  `max_cpu2` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_cpu3` int(10) unsigned NOT NULL DEFAULT '0',
  `max_cpu3` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_memory` int(10) unsigned NOT NULL DEFAULT '0',
  `max_memory` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_disk` int(10) unsigned NOT NULL DEFAULT '0',
  `max_disk` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_msgQ` int(10) unsigned NOT NULL DEFAULT '0',
  `max_msgQ` int(10) unsigned NOT NULL DEFAULT '0',
  `stat_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  PRIMARY KEY (`stat_date`,`system_type`,`system_name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `load_hour_statistics`
--

DROP TABLE IF EXISTS `load_hour_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `load_hour_statistics` (
  `system_type` varchar(16) NOT NULL DEFAULT '',
  `system_name` varchar(16) NOT NULL DEFAULT '',
  `avr_cpu0` int(10) unsigned NOT NULL DEFAULT '0',
  `max_cpu0` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_cpu1` int(10) unsigned NOT NULL DEFAULT '0',
  `max_cpu1` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_cpu2` int(10) unsigned NOT NULL DEFAULT '0',
  `max_cpu2` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_cpu3` int(10) unsigned NOT NULL DEFAULT '0',
  `max_cpu3` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_memory` int(10) unsigned NOT NULL DEFAULT '0',
  `max_memory` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_disk` int(10) unsigned NOT NULL DEFAULT '0',
  `max_disk` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_msgQ` int(10) unsigned NOT NULL DEFAULT '0',
  `max_msgQ` int(10) unsigned NOT NULL DEFAULT '0',
  `stat_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  PRIMARY KEY (`system_type`,`system_name`,`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `load_month_statistics`
--

DROP TABLE IF EXISTS `load_month_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `load_month_statistics` (
  `system_type` varchar(16) NOT NULL DEFAULT '',
  `system_name` varchar(16) NOT NULL DEFAULT '',
  `avr_cpu0` int(10) unsigned NOT NULL DEFAULT '0',
  `max_cpu0` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_cpu1` int(10) unsigned NOT NULL DEFAULT '0',
  `max_cpu1` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_cpu2` int(10) unsigned NOT NULL DEFAULT '0',
  `max_cpu2` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_cpu3` int(10) unsigned NOT NULL DEFAULT '0',
  `max_cpu3` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_memory` int(10) unsigned NOT NULL DEFAULT '0',
  `max_memory` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_disk` int(10) unsigned NOT NULL DEFAULT '0',
  `max_disk` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_msgQ` int(10) unsigned NOT NULL DEFAULT '0',
  `max_msgQ` int(10) unsigned NOT NULL DEFAULT '0',
  `stat_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  PRIMARY KEY (`system_type`,`system_name`,`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `load_week_statistics`
--

DROP TABLE IF EXISTS `load_week_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `load_week_statistics` (
  `system_type` varchar(16) NOT NULL DEFAULT '',
  `system_name` varchar(16) NOT NULL DEFAULT '',
  `avr_cpu0` int(10) unsigned NOT NULL DEFAULT '0',
  `max_cpu0` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_cpu1` int(10) unsigned NOT NULL DEFAULT '0',
  `max_cpu1` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_cpu2` int(10) unsigned NOT NULL DEFAULT '0',
  `max_cpu2` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_cpu3` int(10) unsigned NOT NULL DEFAULT '0',
  `max_cpu3` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_memory` int(10) unsigned NOT NULL DEFAULT '0',
  `max_memory` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_disk` int(10) unsigned NOT NULL DEFAULT '0',
  `max_disk` int(10) unsigned NOT NULL DEFAULT '0',
  `avr_msgQ` int(10) unsigned NOT NULL DEFAULT '0',
  `max_msgQ` int(10) unsigned NOT NULL DEFAULT '0',
  `stat_cnt` int(10) unsigned NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  PRIMARY KEY (`system_type`,`system_name`,`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `logon_5minute_statistics`
--

DROP TABLE IF EXISTS `logon_5minute_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `logon_5minute_statistics` (
  `system_name` varchar(10) NOT NULL,
  `sm_ch_id` tinyint(4) DEFAULT '0',
  `log_mod` tinyint(4) DEFAULT '0',
  `log_req` int(11) DEFAULT '0',
  `log_succ` int(11) DEFAULT '0',
  `log_fail` int(11) DEFAULT '0',
  `succ_rate` tinyint(4) DEFAULT '0',
  `HBIT_0` int(11) DEFAULT '0',
  `HBIT_1` int(11) DEFAULT '0',
  `HBIT_2` int(11) DEFAULT '0',
  `HBIT_3` int(11) DEFAULT '0',
  `HBIT_4` int(11) DEFAULT '0',
  `HBIT_5` int(11) DEFAULT '0',
  `HBIT_6` int(11) DEFAULT '0',
  `HBIT_7` int(11) DEFAULT '0',
  `HBIT_8` int(11) DEFAULT '0',
  `HBIT_9` int(11) DEFAULT '0',
  `HBIT_10` int(11) DEFAULT '0',
  `HBIT_11` int(11) DEFAULT '0',
  `HBIT_12` int(11) DEFAULT '0',
  `HBIT_13` int(11) DEFAULT '0',
  `HBIT_14` int(11) DEFAULT '0',
  `HBIT_15` int(11) DEFAULT '0',
  `HBIT_16` int(11) DEFAULT '0',
  `HBIT_17` int(11) DEFAULT '0',
  `HBIT_18` int(11) DEFAULT '0',
  `HBIT_19` int(11) DEFAULT '0',
  `HBIT_20` int(11) DEFAULT '0',
  `HBIT_21` int(11) DEFAULT '0',
  `HBIT_22` int(11) DEFAULT '0',
  `HBIT_23` int(11) DEFAULT '0',
  `HBIT_24` int(11) DEFAULT '0',
  `HBIT_25` int(11) DEFAULT '0',
  `HBIT_26` int(11) DEFAULT '0',
  `HBIT_27` int(11) DEFAULT '0',
  `HBIT_28` int(11) DEFAULT '0',
  `HBIT_29` int(11) DEFAULT '0',
  `HBIT_30` int(11) DEFAULT '0',
  `HBIT_31` int(11) DEFAULT '0',
  `SM_INT_ERR` int(11) DEFAULT '0',
  `OP_ERR` int(11) DEFAULT '0',
  `OP_TIMEOUT` int(11) DEFAULT '0',
  `ETC_FAIL` int(11) DEFAULT '0',
  `API_REQ_ERR` int(11) DEFAULT '0',
  `API_TIMEOUT` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT NULL,
  KEY `logon_idx` (`system_name`,`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `logon_day_statistics`
--

DROP TABLE IF EXISTS `logon_day_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `logon_day_statistics` (
  `system_name` varchar(10) NOT NULL,
  `sm_ch_id` tinyint(4) DEFAULT '0',
  `log_mod` tinyint(4) DEFAULT '0',
  `log_req` int(11) DEFAULT '0',
  `log_succ` int(11) DEFAULT '0',
  `log_fail` int(11) DEFAULT '0',
  `succ_rate` tinyint(4) DEFAULT '0',
  `HBIT_0` int(11) DEFAULT '0',
  `HBIT_1` int(11) DEFAULT '0',
  `HBIT_2` int(11) DEFAULT '0',
  `HBIT_3` int(11) DEFAULT '0',
  `HBIT_4` int(11) DEFAULT '0',
  `HBIT_5` int(11) DEFAULT '0',
  `HBIT_6` int(11) DEFAULT '0',
  `HBIT_7` int(11) DEFAULT '0',
  `HBIT_8` int(11) DEFAULT '0',
  `HBIT_9` int(11) DEFAULT '0',
  `HBIT_10` int(11) DEFAULT '0',
  `HBIT_11` int(11) DEFAULT '0',
  `HBIT_12` int(11) DEFAULT '0',
  `HBIT_13` int(11) DEFAULT '0',
  `HBIT_14` int(11) DEFAULT '0',
  `HBIT_15` int(11) DEFAULT '0',
  `HBIT_16` int(11) DEFAULT '0',
  `HBIT_17` int(11) DEFAULT '0',
  `HBIT_18` int(11) DEFAULT '0',
  `HBIT_19` int(11) DEFAULT '0',
  `HBIT_20` int(11) DEFAULT '0',
  `HBIT_21` int(11) DEFAULT '0',
  `HBIT_22` int(11) DEFAULT '0',
  `HBIT_23` int(11) DEFAULT '0',
  `HBIT_24` int(11) DEFAULT '0',
  `HBIT_25` int(11) DEFAULT '0',
  `HBIT_26` int(11) DEFAULT '0',
  `HBIT_27` int(11) DEFAULT '0',
  `HBIT_28` int(11) DEFAULT '0',
  `HBIT_29` int(11) DEFAULT '0',
  `HBIT_30` int(11) DEFAULT '0',
  `HBIT_31` int(11) DEFAULT '0',
  `SM_INT_ERR` int(11) DEFAULT '0',
  `OP_ERR` int(11) DEFAULT '0',
  `OP_TIMEOUT` int(11) DEFAULT '0',
  `ETC_FAIL` int(11) DEFAULT '0',
  `API_REQ_ERR` int(11) DEFAULT '0',
  `API_TIMEOUT` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `logon_hour_statistics`
--

DROP TABLE IF EXISTS `logon_hour_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `logon_hour_statistics` (
  `system_name` varchar(10) NOT NULL,
  `sm_ch_id` tinyint(4) DEFAULT '0',
  `log_mod` tinyint(4) DEFAULT '0',
  `log_req` int(11) DEFAULT '0',
  `log_succ` int(11) DEFAULT '0',
  `log_fail` int(11) DEFAULT '0',
  `succ_rate` tinyint(4) DEFAULT '0',
  `HBIT_0` int(11) DEFAULT '0',
  `HBIT_1` int(11) DEFAULT '0',
  `HBIT_2` int(11) DEFAULT '0',
  `HBIT_3` int(11) DEFAULT '0',
  `HBIT_4` int(11) DEFAULT '0',
  `HBIT_5` int(11) DEFAULT '0',
  `HBIT_6` int(11) DEFAULT '0',
  `HBIT_7` int(11) DEFAULT '0',
  `HBIT_8` int(11) DEFAULT '0',
  `HBIT_9` int(11) DEFAULT '0',
  `HBIT_10` int(11) DEFAULT '0',
  `HBIT_11` int(11) DEFAULT '0',
  `HBIT_12` int(11) DEFAULT '0',
  `HBIT_13` int(11) DEFAULT '0',
  `HBIT_14` int(11) DEFAULT '0',
  `HBIT_15` int(11) DEFAULT '0',
  `HBIT_16` int(11) DEFAULT '0',
  `HBIT_17` int(11) DEFAULT '0',
  `HBIT_18` int(11) DEFAULT '0',
  `HBIT_19` int(11) DEFAULT '0',
  `HBIT_20` int(11) DEFAULT '0',
  `HBIT_21` int(11) DEFAULT '0',
  `HBIT_22` int(11) DEFAULT '0',
  `HBIT_23` int(11) DEFAULT '0',
  `HBIT_24` int(11) DEFAULT '0',
  `HBIT_25` int(11) DEFAULT '0',
  `HBIT_26` int(11) DEFAULT '0',
  `HBIT_27` int(11) DEFAULT '0',
  `HBIT_28` int(11) DEFAULT '0',
  `HBIT_29` int(11) DEFAULT '0',
  `HBIT_30` int(11) DEFAULT '0',
  `HBIT_31` int(11) DEFAULT '0',
  `SM_INT_ERR` int(11) DEFAULT '0',
  `OP_ERR` int(11) DEFAULT '0',
  `OP_TIMEOUT` int(11) DEFAULT '0',
  `ETC_FAIL` int(11) DEFAULT '0',
  `API_REQ_ERR` int(11) DEFAULT '0',
  `API_TIMEOUT` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `logon_month_statistics`
--

DROP TABLE IF EXISTS `logon_month_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `logon_month_statistics` (
  `system_name` varchar(10) NOT NULL,
  `sm_ch_id` tinyint(4) DEFAULT '0',
  `log_mod` tinyint(4) DEFAULT '0',
  `log_req` int(11) DEFAULT '0',
  `log_succ` int(11) DEFAULT '0',
  `log_fail` int(11) DEFAULT '0',
  `succ_rate` tinyint(4) DEFAULT '0',
  `HBIT_0` int(11) DEFAULT '0',
  `HBIT_1` int(11) DEFAULT '0',
  `HBIT_2` int(11) DEFAULT '0',
  `HBIT_3` int(11) DEFAULT '0',
  `HBIT_4` int(11) DEFAULT '0',
  `HBIT_5` int(11) DEFAULT '0',
  `HBIT_6` int(11) DEFAULT '0',
  `HBIT_7` int(11) DEFAULT '0',
  `HBIT_8` int(11) DEFAULT '0',
  `HBIT_9` int(11) DEFAULT '0',
  `HBIT_10` int(11) DEFAULT '0',
  `HBIT_11` int(11) DEFAULT '0',
  `HBIT_12` int(11) DEFAULT '0',
  `HBIT_13` int(11) DEFAULT '0',
  `HBIT_14` int(11) DEFAULT '0',
  `HBIT_15` int(11) DEFAULT '0',
  `HBIT_16` int(11) DEFAULT '0',
  `HBIT_17` int(11) DEFAULT '0',
  `HBIT_18` int(11) DEFAULT '0',
  `HBIT_19` int(11) DEFAULT '0',
  `HBIT_20` int(11) DEFAULT '0',
  `HBIT_21` int(11) DEFAULT '0',
  `HBIT_22` int(11) DEFAULT '0',
  `HBIT_23` int(11) DEFAULT '0',
  `HBIT_24` int(11) DEFAULT '0',
  `HBIT_25` int(11) DEFAULT '0',
  `HBIT_26` int(11) DEFAULT '0',
  `HBIT_27` int(11) DEFAULT '0',
  `HBIT_28` int(11) DEFAULT '0',
  `HBIT_29` int(11) DEFAULT '0',
  `HBIT_30` int(11) DEFAULT '0',
  `HBIT_31` int(11) DEFAULT '0',
  `SM_INT_ERR` int(11) DEFAULT '0',
  `OP_ERR` int(11) DEFAULT '0',
  `OP_TIMEOUT` int(11) DEFAULT '0',
  `ETC_FAIL` int(11) DEFAULT '0',
  `API_REQ_ERR` int(11) DEFAULT '0',
  `API_TIMEOUT` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `logon_week_statistics`
--

DROP TABLE IF EXISTS `logon_week_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `logon_week_statistics` (
  `system_name` varchar(10) NOT NULL,
  `sm_ch_id` tinyint(4) DEFAULT '0',
  `log_mod` tinyint(4) DEFAULT '0',
  `log_req` int(11) DEFAULT '0',
  `log_succ` int(11) DEFAULT '0',
  `log_fail` int(11) DEFAULT '0',
  `succ_rate` tinyint(4) DEFAULT '0',
  `HBIT_0` int(11) DEFAULT '0',
  `HBIT_1` int(11) DEFAULT '0',
  `HBIT_2` int(11) DEFAULT '0',
  `HBIT_3` int(11) DEFAULT '0',
  `HBIT_4` int(11) DEFAULT '0',
  `HBIT_5` int(11) DEFAULT '0',
  `HBIT_6` int(11) DEFAULT '0',
  `HBIT_7` int(11) DEFAULT '0',
  `HBIT_8` int(11) DEFAULT '0',
  `HBIT_9` int(11) DEFAULT '0',
  `HBIT_10` int(11) DEFAULT '0',
  `HBIT_11` int(11) DEFAULT '0',
  `HBIT_12` int(11) DEFAULT '0',
  `HBIT_13` int(11) DEFAULT '0',
  `HBIT_14` int(11) DEFAULT '0',
  `HBIT_15` int(11) DEFAULT '0',
  `HBIT_16` int(11) DEFAULT '0',
  `HBIT_17` int(11) DEFAULT '0',
  `HBIT_18` int(11) DEFAULT '0',
  `HBIT_19` int(11) DEFAULT '0',
  `HBIT_20` int(11) DEFAULT '0',
  `HBIT_21` int(11) DEFAULT '0',
  `HBIT_22` int(11) DEFAULT '0',
  `HBIT_23` int(11) DEFAULT '0',
  `HBIT_24` int(11) DEFAULT '0',
  `HBIT_25` int(11) DEFAULT '0',
  `HBIT_26` int(11) DEFAULT '0',
  `HBIT_27` int(11) DEFAULT '0',
  `HBIT_28` int(11) DEFAULT '0',
  `HBIT_29` int(11) DEFAULT '0',
  `HBIT_30` int(11) DEFAULT '0',
  `HBIT_31` int(11) DEFAULT '0',
  `SM_INT_ERR` int(11) DEFAULT '0',
  `OP_ERR` int(11) DEFAULT '0',
  `OP_TIMEOUT` int(11) DEFAULT '0',
  `ETC_FAIL` int(11) DEFAULT '0',
  `API_REQ_ERR` int(11) DEFAULT '0',
  `API_TIMEOUT` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `mmc_command`
--

DROP TABLE IF EXISTS `mmc_command`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `mmc_command` (
  `cmd_name` varchar(32) NOT NULL DEFAULT '',
  `cmd_category` varchar(40) NOT NULL DEFAULT '0',
  `dst_sysname` varchar(16) NOT NULL DEFAULT '',
  `dst_appname` varchar(16) NOT NULL DEFAULT '',
  `privilege` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `param_count` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `cmd_help` text,
  PRIMARY KEY (`cmd_name`,`cmd_category`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `mmc_parameter`
--

DROP TABLE IF EXISTS `mmc_parameter`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `mmc_parameter` (
  `cmd_name` varchar(32) NOT NULL DEFAULT '',
  `param_name` varchar(16) NOT NULL DEFAULT '',
  `param_no` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `mand_flag` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `param_type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `min_value` int(11) NOT NULL DEFAULT '0',
  `max_value` int(11) NOT NULL DEFAULT '0',
  `para_desc` varchar(32) NOT NULL DEFAULT '',
  `enum_list` blob,
  PRIMARY KEY (`cmd_name`,`param_name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `pkt_delay_5minute_statistics`
--

DROP TABLE IF EXISTS `pkt_delay_5minute_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `pkt_delay_5minute_statistics` (
  `system_name` varchar(10) DEFAULT NULL,
  `stat_date` datetime DEFAULT '0000-00-00 00:00:00',
  `insert_date` datetime DEFAULT '0000-00-00 00:00:00',
  `match_cnt` int(11) DEFAULT '0',
  `min_usec` double(15,6) DEFAULT '0.000000',
  `max_usec` double(15,6) DEFAULT '0.000000',
  `avg_usec` double(15,6) DEFAULT '0.000000'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `pkt_delay_day_statistics`
--

DROP TABLE IF EXISTS `pkt_delay_day_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `pkt_delay_day_statistics` (
  `system_name` varchar(10) DEFAULT NULL,
  `stat_date` datetime DEFAULT '0000-00-00 00:00:00',
  `match_cnt` int(11) DEFAULT NULL,
  `min_usec` double(15,6) DEFAULT NULL,
  `max_usec` double(15,6) DEFAULT NULL,
  `avg_usec` double(15,6) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `pkt_delay_hour_statistics`
--

DROP TABLE IF EXISTS `pkt_delay_hour_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `pkt_delay_hour_statistics` (
  `system_name` varchar(10) DEFAULT NULL,
  `stat_date` datetime DEFAULT '0000-00-00 00:00:00',
  `insert_date` datetime DEFAULT '0000-00-00 00:00:00',
  `match_cnt` int(11) DEFAULT NULL,
  `min_usec` double(15,6) DEFAULT NULL,
  `max_usec` double(15,6) DEFAULT NULL,
  `avg_usec` double(15,6) DEFAULT NULL,
  KEY `pkt_idx` (`system_name`,`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `pkt_delay_month_statistics`
--

DROP TABLE IF EXISTS `pkt_delay_month_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `pkt_delay_month_statistics` (
  `system_name` varchar(10) DEFAULT NULL,
  `stat_date` datetime DEFAULT '0000-00-00 00:00:00',
  `match_cnt` int(11) DEFAULT NULL,
  `min_usec` double(15,6) DEFAULT NULL,
  `max_usec` double(15,6) DEFAULT NULL,
  `avg_usec` double(15,6) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `pkt_delay_week_statistics`
--

DROP TABLE IF EXISTS `pkt_delay_week_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `pkt_delay_week_statistics` (
  `system_name` varchar(10) DEFAULT NULL,
  `stat_date` datetime DEFAULT '0000-00-00 00:00:00',
  `match_cnt` int(11) DEFAULT NULL,
  `min_usec` double(15,6) DEFAULT NULL,
  `max_usec` double(15,6) DEFAULT NULL,
  `avg_usec` double(15,6) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_block_5minute_statistics`
--

DROP TABLE IF EXISTS `rdr_block_5minute_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_block_5minute_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `subscriber_id` varchar(64) DEFAULT '',
  `package_id` smallint(6) DEFAULT NULL,
  `service_id` int(11) DEFAULT NULL,
  `protocol_id` smallint(6) DEFAULT NULL,
  `initiating_side` tinyint(4) DEFAULT NULL,
  `block_reason` smallint(6) DEFAULT NULL,
  `block_rdr_cnt` int(11) DEFAULT '0',
  `redirected` tinyint(4) DEFAULT '0',
  `stat_cnt` int(11) NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  KEY `stat_date` (`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_block_day_statistics`
--

DROP TABLE IF EXISTS `rdr_block_day_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_block_day_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `subscriber_id` varchar(64) DEFAULT '',
  `package_id` smallint(6) DEFAULT NULL,
  `service_id` int(11) DEFAULT NULL,
  `protocol_id` tinyint(4) DEFAULT NULL,
  `initiating_side` tinyint(4) DEFAULT NULL,
  `block_reason` tinyint(4) DEFAULT NULL,
  `block_rdr_cnt` int(11) DEFAULT '0',
  `redirected` tinyint(4) DEFAULT '0',
  `stat_cnt` int(11) NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  KEY `stat_date` (`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_block_hour_statistics`
--

DROP TABLE IF EXISTS `rdr_block_hour_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_block_hour_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `subscriber_id` varchar(64) DEFAULT '',
  `package_id` smallint(6) DEFAULT NULL,
  `service_id` int(11) DEFAULT NULL,
  `protocol_id` tinyint(4) DEFAULT NULL,
  `initiating_side` tinyint(4) DEFAULT NULL,
  `block_reason` tinyint(4) DEFAULT NULL,
  `block_rdr_cnt` int(11) DEFAULT '0',
  `redirected` tinyint(4) DEFAULT '0',
  `stat_cnt` int(11) NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  KEY `stat_date` (`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_block_month_statistics`
--

DROP TABLE IF EXISTS `rdr_block_month_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_block_month_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `subscriber_id` varchar(64) DEFAULT '',
  `package_id` smallint(6) DEFAULT NULL,
  `service_id` int(11) DEFAULT NULL,
  `protocol_id` tinyint(4) DEFAULT NULL,
  `initiating_side` tinyint(4) DEFAULT NULL,
  `block_reason` tinyint(4) DEFAULT NULL,
  `block_rdr_cnt` int(11) DEFAULT '0',
  `redirected` tinyint(4) DEFAULT '0',
  `stat_cnt` int(11) NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  KEY `stat_date` (`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_block_week_statistics`
--

DROP TABLE IF EXISTS `rdr_block_week_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_block_week_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `subscriber_id` varchar(64) DEFAULT '',
  `package_id` smallint(6) DEFAULT NULL,
  `service_id` int(11) DEFAULT NULL,
  `protocol_id` tinyint(4) DEFAULT NULL,
  `initiating_side` tinyint(4) DEFAULT NULL,
  `block_reason` tinyint(4) DEFAULT NULL,
  `block_rdr_cnt` int(11) DEFAULT '0',
  `redirected` tinyint(4) DEFAULT '0',
  `stat_cnt` int(11) NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  KEY `stat_date` (`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_endtime`
--

DROP TABLE IF EXISTS `rdr_endtime`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_endtime` (
  `record_source` varchar(16) DEFAULT NULL,
  `link_id` tinyint(4) DEFAULT NULL,
  `upstream_volume` bigint(20) DEFAULT NULL,
  `downstream_volume` bigint(20) DEFAULT NULL,
  `stat_date` varchar(16) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_lur_5minute_statistics`
--

DROP TABLE IF EXISTS `rdr_lur_5minute_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_lur_5minute_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `link_id` tinyint(4) DEFAULT NULL,
  `upstream_volume` bigint(20) DEFAULT '0',
  `downstream_volume` bigint(20) DEFAULT '0',
  `stat_cnt` int(11) NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  KEY `update_idx` (`record_source`,`link_id`,`stat_date`),
  KEY `select_idx` (`record_source`,`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_lur_day_statistics`
--

DROP TABLE IF EXISTS `rdr_lur_day_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_lur_day_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `link_id` tinyint(4) DEFAULT NULL,
  `upstream_volume` bigint(20) DEFAULT '0',
  `downstream_volume` bigint(20) DEFAULT '0',
  `stat_cnt` int(11) NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_lur_hour_statistics`
--

DROP TABLE IF EXISTS `rdr_lur_hour_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_lur_hour_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `link_id` tinyint(4) DEFAULT NULL,
  `upstream_volume` bigint(20) DEFAULT '0',
  `downstream_volume` bigint(20) DEFAULT '0',
  `stat_cnt` int(11) NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_lur_month_statistics`
--

DROP TABLE IF EXISTS `rdr_lur_month_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_lur_month_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `link_id` tinyint(4) DEFAULT NULL,
  `upstream_volume` bigint(20) DEFAULT '0',
  `downstream_volume` bigint(20) DEFAULT '0',
  `stat_cnt` int(11) NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_lur_week_statistics`
--

DROP TABLE IF EXISTS `rdr_lur_week_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_lur_week_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `link_id` tinyint(4) DEFAULT NULL,
  `upstream_volume` bigint(20) DEFAULT '0',
  `downstream_volume` bigint(20) DEFAULT '0',
  `stat_cnt` int(11) NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_ruleent_5minute_statistics`
--

DROP TABLE IF EXISTS `rdr_ruleent_5minute_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_ruleent_5minute_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `rule_ent_id` smallint(6) DEFAULT NULL,
  `rule_ent_name` varchar(64) DEFAULT NULL,
  `session` int(11) DEFAULT '0',
  `upstream_volume` bigint(20) DEFAULT '0',
  `downstream_volume` bigint(20) DEFAULT '0',
  `block_cnt` int(12) DEFAULT '0',
  `redirect_cnt` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT '',
  KEY `update_idx` (`record_source`,`rule_ent_id`,`rule_ent_name`,`stat_date`),
  KEY `select_idx` (`record_source`,`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_ruleent_day_statistics`
--

DROP TABLE IF EXISTS `rdr_ruleent_day_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_ruleent_day_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `rule_ent_id` smallint(6) DEFAULT NULL,
  `rule_ent_name` varchar(64) DEFAULT NULL,
  `session` int(11) DEFAULT '0',
  `upstream_volume` bigint(20) DEFAULT '0',
  `downstream_volume` bigint(20) DEFAULT '0',
  `block_cnt` int(12) DEFAULT '0',
  `redirect_cnt` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT '',
  KEY `entry_id` (`record_source`,`rule_ent_id`,`rule_ent_name`),
  KEY `dateidx` (`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_ruleent_hour_statistics`
--

DROP TABLE IF EXISTS `rdr_ruleent_hour_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_ruleent_hour_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `rule_ent_id` smallint(6) DEFAULT NULL,
  `rule_ent_name` varchar(64) DEFAULT NULL,
  `session` int(11) DEFAULT '0',
  `upstream_volume` bigint(20) DEFAULT '0',
  `downstream_volume` bigint(20) DEFAULT '0',
  `block_cnt` int(12) DEFAULT '0',
  `redirect_cnt` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT '',
  KEY `entry_id` (`record_source`,`rule_ent_id`,`rule_ent_name`),
  KEY `dateidx` (`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_ruleent_month_statistics`
--

DROP TABLE IF EXISTS `rdr_ruleent_month_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_ruleent_month_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `rule_ent_id` smallint(6) DEFAULT NULL,
  `rule_ent_name` varchar(64) DEFAULT NULL,
  `session` int(11) DEFAULT '0',
  `upstream_volume` bigint(20) DEFAULT '0',
  `downstream_volume` bigint(20) DEFAULT '0',
  `block_cnt` int(12) DEFAULT '0',
  `redirect_cnt` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_ruleent_week_statistics`
--

DROP TABLE IF EXISTS `rdr_ruleent_week_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_ruleent_week_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `rule_ent_id` smallint(6) DEFAULT NULL,
  `rule_ent_name` varchar(64) DEFAULT NULL,
  `session` int(11) DEFAULT '0',
  `upstream_volume` bigint(20) DEFAULT '0',
  `downstream_volume` bigint(20) DEFAULT '0',
  `block_cnt` int(12) DEFAULT '0',
  `redirect_cnt` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_ruleset_5minute_statistics`
--

DROP TABLE IF EXISTS `rdr_ruleset_5minute_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_ruleset_5minute_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `pkg_id` smallint(6) DEFAULT NULL,
  `rule_set_id` varchar(6) DEFAULT NULL,
  `rule_set_name` varchar(64) DEFAULT NULL,
  `session` int(11) DEFAULT '0',
  `upstream_volume` bigint(20) DEFAULT '0',
  `downstream_volume` bigint(20) DEFAULT '0',
  `block_cnt` int(12) DEFAULT '0',
  `redirect_cnt` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT '',
  KEY `update_idx` (`record_source`,`pkg_id`,`rule_set_id`,`rule_set_name`,`stat_date`),
  KEY `select_idx` (`record_source`,`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_ruleset_day_statistics`
--

DROP TABLE IF EXISTS `rdr_ruleset_day_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_ruleset_day_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `pkg_id` smallint(6) DEFAULT NULL,
  `rule_set_id` varchar(6) DEFAULT NULL,
  `rule_set_name` varchar(64) DEFAULT NULL,
  `session` int(11) DEFAULT '0',
  `upstream_volume` bigint(20) DEFAULT '0',
  `downstream_volume` bigint(20) DEFAULT '0',
  `block_cnt` int(12) DEFAULT '0',
  `redirect_cnt` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_ruleset_hour_statistics`
--

DROP TABLE IF EXISTS `rdr_ruleset_hour_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_ruleset_hour_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `pkg_id` smallint(6) DEFAULT NULL,
  `rule_set_id` varchar(6) DEFAULT NULL,
  `rule_set_name` varchar(64) DEFAULT NULL,
  `session` int(11) DEFAULT '0',
  `upstream_volume` bigint(20) DEFAULT '0',
  `downstream_volume` bigint(20) DEFAULT '0',
  `block_cnt` int(12) DEFAULT '0',
  `redirect_cnt` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT ''
  KEY `update_idx` (`record_source`,`pkg_id`,`rule_set_id`,`rule_set_name`,`stat_date`),
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_ruleset_month_statistics`
--

DROP TABLE IF EXISTS `rdr_ruleset_month_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_ruleset_month_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `pkg_id` smallint(6) DEFAULT NULL,
  `rule_set_id` varchar(6) DEFAULT NULL,
  `rule_set_name` varchar(64) DEFAULT NULL,
  `session` int(11) DEFAULT '0',
  `upstream_volume` bigint(20) DEFAULT '0',
  `downstream_volume` bigint(20) DEFAULT '0',
  `block_cnt` int(12) DEFAULT '0',
  `redirect_cnt` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_ruleset_week_statistics`
--

DROP TABLE IF EXISTS `rdr_ruleset_week_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_ruleset_week_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `pkg_id` smallint(6) DEFAULT NULL,
  `rule_set_id` varchar(6) DEFAULT NULL,
  `rule_set_name` varchar(64) DEFAULT NULL,
  `session` int(11) DEFAULT '0',
  `upstream_volume` bigint(20) DEFAULT '0',
  `downstream_volume` bigint(20) DEFAULT '0',
  `block_cnt` int(12) DEFAULT '0',
  `redirect_cnt` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_timestamp`
--

DROP TABLE IF EXISTS `rdr_timestamp`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_timestamp` (
  `record_source` varchar(16) DEFAULT NULL,
  `link_id` tinyint(4) DEFAULT NULL,
  `upstream_volume` bigint(20) DEFAULT NULL,
  `downstream_volume` bigint(20) DEFAULT NULL,
  `stat_date` varchar(16) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_tr_5minute_statistics`
--

DROP TABLE IF EXISTS `rdr_tr_5minute_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_tr_5minute_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `package_id` smallint(6) DEFAULT NULL,
  `service_id` int(11) DEFAULT NULL,
  `session` int(11) DEFAULT '0',
  `upstream_volume` bigint(20) DEFAULT '0',
  `downstream_volume` bigint(20) DEFAULT '0',
  `stat_cnt` int(11) NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  KEY `stat_date` (`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_tr_day_statistics`
--

DROP TABLE IF EXISTS `rdr_tr_day_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_tr_day_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `package_id` smallint(6) DEFAULT NULL,
  `service_id` int(11) DEFAULT NULL,
  `protocol_id` tinyint(4) DEFAULT NULL,
  `upstream_volume` bigint(20) DEFAULT '0',
  `downstream_volume` bigint(20) DEFAULT '0',
  `stat_cnt` int(11) NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  KEY `stat_date` (`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_tr_hour_statistics`
--

DROP TABLE IF EXISTS `rdr_tr_hour_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_tr_hour_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `package_id` smallint(6) DEFAULT NULL,
  `service_id` int(11) DEFAULT NULL,
  `protocol_id` tinyint(4) DEFAULT NULL,
  `upstream_volume` bigint(20) DEFAULT '0',
  `downstream_volume` bigint(20) DEFAULT '0',
  `stat_cnt` int(11) NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  KEY `stat_date` (`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_tr_month_statistics`
--

DROP TABLE IF EXISTS `rdr_tr_month_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_tr_month_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `package_id` smallint(6) DEFAULT NULL,
  `service_id` int(11) DEFAULT NULL,
  `protocol_id` tinyint(4) DEFAULT NULL,
  `upstream_volume` bigint(20) DEFAULT '0',
  `downstream_volume` bigint(20) DEFAULT '0',
  `stat_cnt` int(11) NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  KEY `stat_date` (`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rdr_tr_week_statistics`
--

DROP TABLE IF EXISTS `rdr_tr_week_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rdr_tr_week_statistics` (
  `record_source` varchar(16) DEFAULT '',
  `package_id` smallint(6) DEFAULT NULL,
  `service_id` int(11) DEFAULT NULL,
  `protocol_id` tinyint(4) DEFAULT NULL,
  `upstream_volume` bigint(20) DEFAULT '0',
  `downstream_volume` bigint(20) DEFAULT '0',
  `stat_cnt` int(11) NOT NULL DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) NOT NULL DEFAULT '',
  KEY `stat_date` (`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `rule_editor_login`
--

DROP TABLE IF EXISTS `rule_editor_login`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `rule_editor_login` (
  `id` varchar(32) DEFAULT NULL,
  `login_date` datetime DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `sms_5minute_statistics`
--

DROP TABLE IF EXISTS `sms_5minute_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `sms_5minute_statistics` (
  `system_name` varchar(10) DEFAULT NULL,
  `smsc_ip` varchar(18) DEFAULT NULL,
  `req` int(11) DEFAULT '0',
  `succ` int(11) DEFAULT '0',
  `fail` int(11) DEFAULT '0',
  `smpp_err` int(11) DEFAULT '0',
  `svr_err` int(11) DEFAULT '0',
  `smsc_err` int(11) DEFAULT '0',
  `etc_err` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT NULL,
  KEY `update_idx` (`system_name`,`smsc_ip`,`stat_date`),
  KEY `select_idx` (`system_name`,`stat_date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `sms_day_statistics`
--

DROP TABLE IF EXISTS `sms_day_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `sms_day_statistics` (
  `system_name` varchar(10) DEFAULT NULL,
  `smsc_ip` varchar(18) DEFAULT NULL,
  `req` int(11) DEFAULT '0',
  `succ` int(11) DEFAULT '0',
  `fail` int(11) DEFAULT '0',
  `smpp_err` int(11) DEFAULT '0',
  `svr_err` int(11) DEFAULT '0',
  `smsc_err` int(11) DEFAULT '0',
  `etc_err` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `sms_history`
--

DROP TABLE IF EXISTS `sms_history`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `sms_history` (
  `system_name` varchar(8) DEFAULT NULL,
  `smsc_ip` varchar(18) DEFAULT NULL,
  `PKG_ID` smallint(6) DEFAULT NULL,
  `rule_set_id` varchar(6) DEFAULT NULL,
  `SUBS_ID` varchar(64) NOT NULL,
  `BLK_TIME` int(16) unsigned DEFAULT '0',
  `SMS_MSG` varchar(160) DEFAULT NULL,
  `DELIV_TIME` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `DELIV_STS` smallint(6) unsigned DEFAULT NULL,
  `REPORT_TIME` varchar(21) DEFAULT '0',
  `REPORT_STS` smallint(6) unsigned DEFAULT NULL,
  `SERIAL_NUM` int(11) DEFAULT NULL,
  `REQ` int(11) DEFAULT '0',
  `SUCC` int(11) DEFAULT '0',
  `SMPP_ERR` int(11) DEFAULT '0',
  `SVR_ERR` int(11) DEFAULT '0',
  `SMSC_ERR` int(11) DEFAULT '0',
  `ETC_ERR` int(11) DEFAULT '0',
  KEY `sms_idx1` (`SUBS_ID`,`SERIAL_NUM`),
  KEY `sms_idx2` (`SUBS_ID`,`DELIV_TIME`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `sms_hour_statistics`
--

DROP TABLE IF EXISTS `sms_hour_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `sms_hour_statistics` (
  `system_name` varchar(10) DEFAULT NULL,
  `smsc_ip` varchar(18) DEFAULT NULL,
  `req` int(11) DEFAULT '0',
  `succ` int(11) DEFAULT '0',
  `fail` int(11) DEFAULT '0',
  `smpp_err` int(11) DEFAULT '0',
  `svr_err` int(11) DEFAULT '0',
  `smsc_err` int(11) DEFAULT '0',
  `etc_err` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `sms_month_statistics`
--

DROP TABLE IF EXISTS `sms_month_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `sms_month_statistics` (
  `system_name` varchar(10) DEFAULT NULL,
  `smsc_ip` varchar(18) DEFAULT NULL,
  `req` int(11) DEFAULT '0',
  `succ` int(11) DEFAULT '0',
  `fail` int(11) DEFAULT '0',
  `smpp_err` int(11) DEFAULT '0',
  `svr_err` int(11) DEFAULT '0',
  `smsc_err` int(11) DEFAULT '0',
  `etc_err` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `sms_msg`
--

DROP TABLE IF EXISTS `sms_msg`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `sms_msg` (
  `date` datetime DEFAULT NULL,
  `msg` char(80) DEFAULT NULL,
  `id` varchar(32) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `sms_week_statistics`
--

DROP TABLE IF EXISTS `sms_week_statistics`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `sms_week_statistics` (
  `system_name` varchar(10) DEFAULT NULL,
  `smsc_ip` varchar(18) DEFAULT NULL,
  `req` int(11) DEFAULT '0',
  `succ` int(11) DEFAULT '0',
  `fail` int(11) DEFAULT '0',
  `smpp_err` int(11) DEFAULT '0',
  `svr_err` int(11) DEFAULT '0',
  `smsc_err` int(11) DEFAULT '0',
  `etc_err` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Table structure for table `tab_port_name`
--

DROP TABLE IF EXISTS `tab_port_name`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `tab_port_name` (
  `dev_name` char(5) DEFAULT NULL,
  `port_index` int(11) NOT NULL,
  `port_grp` char(11) DEFAULT NULL,
  `port_number` int(11) DEFAULT NULL,
  `port_name` char(20) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2010-09-01  7:19:46
