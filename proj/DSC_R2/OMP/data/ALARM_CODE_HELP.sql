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
-- Dumping data for table `alarm_code_help`
--


INSERT INTO `alarm_code_help` VALUES ('A1100',0,NULL,'CPU LOAD MINOR ALARM'),('A1200',0,NULL,'CPU LOAD MAJOR ALARM'),('A1300',0,NULL,'CPU LOAD CRITICAL ALARM'),('A1101',0,NULL,'MEMORY LOAD MINOR ALARM'),('A1201',0,NULL,'MEMORY LOAD MAJOR ALARM'),('A1301',0,NULL,'MEMORY LOAD CRITICAL ALARM'),('A1102',0,NULL,'DISK LOAD MINOR ALARM'),('A1202',0,NULL,'DISK LOAD MAJOR ALARM'),('A1302',0,NULL,'DISK LOAD CRITICAL ALARM'), ('A1303',0,NULL,'MANAGEMENT NETWORK LINK CRITICAL ALARM'),('A1104',0,NULL,'S/W PROCESS MAJOR ALARM'),('A1204',0,NULL,'S/W PROCESS MAJOR ALARM'),('A1304',0,NULL,'S/W PROCESS CRITICAL ALARM'),('A1318',0,NULL,'REMOTE NETWORK LINK ALARM'),('A1120',0,NULL,'NETWORK TIME PROTOCOL MINOR ALARM'),('A1220',0,NULL,'NETWORK TIME PROTOCOL MAJOR ALARM'),('A1320',0,NULL,'NETWORK TIME PROTOCOL CRITICALALARM'),('A1324',0,NULL,'TAP NETWORK ALARM'),('A1226',0,NULL,'DSC H/W MAJOR ALARM'),('A1326',0,NULL,'DSC H/W CRITICAL ALARM'),('A1127',0,NULL,'QUEUE LOAD MINOR ALARM'),('A1227',0,NULL,'QUEUE LOAD MAJOR ALARM'),('A1327',0,NULL,'QUEUE LOAD CRITICAL ALARM'),('A1331',0,NULL,'TAP MGMT STS'),('A1332',0,NULL,'L2SW MGMT STS'),('A1151',0,NULL,'SCE CPU LOAD MINOR ALARM'),('A1251',0,NULL,'SCE CPU LOAD MAJOR ALARM'),('A1351',0,NULL,'SCE CPU LOAD CRITICAL ALARM'),('A1152',0,NULL,'SCE MEMORY LOAD MINOR ALARM'),('A1252',0,NULL,'SCE MEMORY LOAD MAJOR ALARM'),('A1352',0,NULL,'SCE MEMORY LOAD CRITICAL ALARM'),('A1153',0,NULL,'SCE DISK LOAD MINOR ALARM'),('A1253',0,NULL,'SCE DISK LOAD MAJOR ALARM'),('A1353',0,NULL,'SCE DISK LOAD CRITICAL ALARM'), ('A1254',0,NULL,'SCE DEVICE MGMT & SCE DEVICE LINK ALARM'),('A1354',0,NULL,'SCE DEVICE MGMT & SCE DEVICE LINK ALARM'),('A1255',0,NULL,'SCE DEVICE FAN ALARM'),('A1356',0,NULL,'SCE DEVICE PWR ALARM'),('A1261',0,NULL,'SCE DEVICE RDR CONN ALARM'),('A1162',0,NULL,'SCE DEVICE STATUS ALARM'),('A1157',0,NULL,'L2 SWITCH CPU USAGE MINOR ALARM'),('A1257',0,NULL,'L2 SWITCH CPU USAGE MAJOR ALARM'),('A1357',0,NULL,'L2 SWITCH CPU USAGE CRITICAL ALARM'),('A1158',0,NULL,'L2 SWITCH MEMORY USAGE MINOR ALARM'),('A1258',0,NULL,'L2 SWITCH MEMORY USAGE MAJOR ALARM'),('A1358',0,NULL,'L2 SWITCH MEMORY USAGE CRITICAL ALARM'),('A1359',0,NULL,'L2 SWITCH MEMORY USAGE ALARM'),('A1171',0,NULL,'S/W SAMD PROCESS MAJOR ALARM'),('A1271',0,NULL,'S/W SAMD PROCESS MAJOR ALARM'),('A1371',0,NULL,'S/W SAMD PROCESS CRITICAL ALARM'),('A1172',0,NULL,'S/W IXPC PROCESS MAJOR ALARM'),('A1272',0,NULL,'S/W IXPC PROCESS MAJOR ALARM'),('A1372',0,NULL,'S/W IXPC PROCESS CRITICAL ALARM'),('A1173',0,NULL,'S/W FIMD PROCESS MAJOR ALARM'),('A1273',0,NULL,'S/W FIMD PROCESS MAJOR ALARM'),('A1373',0,NULL,'S/W FIMD PROCESS CRITICAL ALARM'),('A1174',0,NULL,'S/W COND PROCESS MAJOR ALARM'),('A1274',0,NULL,'S/W COND PROCESS MAJOR ALARM'),('A1374',0,NULL,'S/W COND PROCESS CRITICAL ALARM'),('A1175',0,NULL,'S/W STMD PROCESS MAJOR ALARM'),('A1275',0,NULL,'S/W STMD PROCESS MAJOR ALARM'),('A1375',0,NULL,'S/W STMD PROCESS CRITICAL ALARM'),('A1176',0,NULL,'S/W MMCD PROCESS MAJOR ALARM'),('A1276',0,NULL,'S/W MMCD PROCESS MAJOR ALARM'),('A1376',0,NULL,'S/W MMCD PROCESS CRITICAL ALARM'),('A1177',0,NULL,'S/W MCDM PROCESS MAJOR ALARM'),('A1277',0,NULL,'S/W MCDM PROCESS MAJOR ALARM'),('A1377',0,NULL,'S/W MCDM PROCESS CRITICAL ALARM'),('A1178',0,NULL,'S/W NMSIF PROCESS MAJOR ALARM'),('A1278',0,NULL,'S/W NMSIF PROCESS MAJOR ALARM'),('A1378',0,NULL,'S/W NMSIF PROCESS CRITICAL ALARM'),('A1179',0,NULL,'S/W CDELAY PROCESS MAJOR ALARM'),('A1279',0,NULL,'S/W CDELAY PROCESS MAJOR ALARM'),('A1379',0,NULL,'S/W CDELAY PROCESS CRITICAL ALARM'),('A1180',0,NULL,'S/W HAMON PROCESS MAJOR ALARM'),('A1280',0,NULL,'S/W HAMON PROCESS MAJOR ALARM'),('A1380',0,NULL,'S/W HAMON PROCESS CRITICAL ALARM'),('A1181',0,NULL,'S/W MMCR PROCESS MAJOR ALARM'),('A1281',0,NULL,'S/W MMCR PROCESS MAJOR ALARM'),('A1381',0,NULL,'S/W MMCR PROCESS CRITICAL ALARM'),('A1182',0,NULL,'S/W RDRANA PROCESS MAJOR ALARM'),('A1282',0,NULL,'S/W RDRANA PROCESS MAJOR ALARM'),('A1382',0,NULL,'S/W RDRANA PROCESS CRITICAL ALARM'),('A1183',0,NULL,'S/W RLEG PROCESS MAJOR ALARM'),('A1283',0,NULL,'S/W RLEG PROCESS MAJOR ALARM'),('A1383',0,NULL,'S/W RLEG PROCESS CRITICAL ALARM'),('A1184',0,NULL,'S/W SMPP PROCESS MAJOR ALARM'),('A1284',0,NULL,'S/W SMPP PROCESS MAJOR ALARM'),('A1384',0,NULL,'S/W SMPP PROCESS CRITICAL ALARM'),('A1185',0,NULL,'S/W PANA PROCESS MAJOR ALARM'),('A1285',0,NULL,'S/W PANA PROCESS MAJOR ALARM'),('A1385',0,NULL,'S/W PANA PROCESS CRITICAL ALARM'),('A1186',0,NULL,'S/W RANA PROCESS MAJOR ALARM'),('A1286',0,NULL,'S/W RANA PROCESS MAJOR ALARM'),('A1386',0,NULL,'S/W RANA PROCESS CRITICAL ALARM'),('A1187',0,NULL,'S/W RDRCAPD PROCESS MAJOR ALARM'),('A1287',0,NULL,'S/W RDRCAPD PROCESS MAJOR ALARM'),('A1387',0,NULL,'S/W RDRCAPD PROCESS CRITICAL ALARM'),('A1188',0,NULL,'S/W CAPD PROCESS MAJOR ALARM'),('A1288',0,NULL,'S/W CAPD PROCESS MAJOR ALARM'),('A1388',0,NULL,'S/W CAPD PROCESS CRITICAL ALARM'),('A1392',0,NULL,'SCM MIRROR PORT ALARM');
/*!40000 ALTER TABLE `alarm_code_help` ENABLE KEYS */;


UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2009-09-25  5:25:05
