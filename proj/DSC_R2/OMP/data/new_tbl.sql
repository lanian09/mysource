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
  `HBIT_32` int(11) DEFAULT '0',
  `SM_INT_ERR` int(11) DEFAULT '0',
  `OP_ERR` int(11) DEFAULT '0',
  `OP_TIMEOUT` int(11) DEFAULT '0',
  `api_req_err` int(11) DEFAULT '0',
  `api_timeout` int(11) DEFAULT '0',
  `ETC_FAIL` int(11) DEFAULT '0',
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
  `HBIT_32` int(11) DEFAULT '0',
  `SM_INT_ERR` int(11) DEFAULT '0',
  `OP_ERR` int(11) DEFAULT '0',
  `OP_TIMEOUT` int(11) DEFAULT '0',
  `api_req_err` int(11) DEFAULT '0',
  `api_timeout` int(11) DEFAULT '0',
  `ETC_FAIL` int(11) DEFAULT '0',
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
  `HBIT_32` int(11) DEFAULT '0',
  `SM_INT_ERR` int(11) DEFAULT '0',
  `OP_ERR` int(11) DEFAULT '0',
  `OP_TIMEOUT` int(11) DEFAULT '0',
  `api_req_err` int(11) DEFAULT '0',
  `api_timeout` int(11) DEFAULT '0',
  `ETC_FAIL` int(11) DEFAULT '0',
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
  `HBIT_32` int(11) DEFAULT '0',
  `SM_INT_ERR` int(11) DEFAULT '0',
  `OP_ERR` int(11) DEFAULT '0',
  `OP_TIMEOUT` int(11) DEFAULT '0',
  `api_req_err` int(11) DEFAULT '0',
  `api_timeout` int(11) DEFAULT '0',
  `ETC_FAIL` int(11) DEFAULT '0',
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
  `HBIT_32` int(11) DEFAULT '0',
  `SM_INT_ERR` int(11) DEFAULT '0',
  `OP_ERR` int(11) DEFAULT '0',
  `OP_TIMEOUT` int(11) DEFAULT '0',
  `api_req_err` int(11) DEFAULT '0',
  `api_timeout` int(11) DEFAULT '0',
  `ETC_FAIL` int(11) DEFAULT '0',
  `stat_cnt` int(11) DEFAULT '0',
  `stat_date` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `stat_week` varchar(3) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
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

