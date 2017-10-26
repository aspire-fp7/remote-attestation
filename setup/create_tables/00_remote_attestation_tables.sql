-- phpMyAdmin SQL Dump
-- version 3.4.11.1deb2+deb7u2
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Jun 26, 2016 at 09:09 PM
-- Server version: 5.5.40
-- PHP Version: 5.4.45-0+deb7u2

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `RA_development`
--

DELIMITER $$
--
-- Procedures
--
CREATE DEFINER=`ra_user`@`%` PROCEDURE `RequestsSanitizer`()
    NO SQL
UPDATE ra_request AS r
SET status = 5
WHERE  TIMESTAMPDIFF(SECOND, r.send_time, NOW()) > r.validity_time AND r.status = 0$$

DELIMITER ;

-- --------------------------------------------------------

--
-- Table structure for table `ra_application`
--

CREATE TABLE IF NOT EXISTS `ra_application` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT 'DB id for the application',
  `AID` char(32) NOT NULL COMMENT 'ASPIRE Application Id associated to the application',
  `description` varchar(100) DEFAULT NULL COMMENT 'Optional text field to describe the application',
  PRIMARY KEY (`id`),
  UNIQUE KEY `AID` (`AID`)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1 COMMENT='List of protected applications';

-- --------------------------------------------------------

--
-- Table structure for table `ra_attestator`
--

CREATE TABLE IF NOT EXISTS `ra_attestator` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT 'DB id for the attestator',
  `application_id` bigint(20) unsigned NOT NULL COMMENT 'Foreign key: refers to DB id of the associated record in the ra_application table',
  `attestator_no` bigint(20) unsigned zerofill NOT NULL COMMENT 'number of the attestator inside the application',
  `description` varchar(100) DEFAULT NULL COMMENT 'Optional text filed for comments about the attestator',
  `sleep_avg` int(10) unsigned NOT NULL DEFAULT '10' COMMENT 'Mean period (in seconds) between two attestation request ',
  `sleep_var` int(10) unsigned NOT NULL DEFAULT '1' COMMENT 'Variation around the mean period, for random reasons',
  PRIMARY KEY (`id`),
  UNIQUE KEY `application_id_2` (`application_id`,`attestator_no`),
  KEY `application_id` (`application_id`)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1 COMMENT='List of all the defined attestators';

-- --------------------------------------------------------

--
-- Table structure for table `ra_attest_at_startup_area`
--

CREATE TABLE IF NOT EXISTS `ra_attest_at_startup_area` (
  `attestator_id` bigint(20) unsigned NOT NULL COMMENT 'Foreign key: refers to the attestator record (DB id)',
  `memory_area` smallint(5) unsigned NOT NULL COMMENT 'Indicates the label of the memory area to attest at startup',
  PRIMARY KEY (`attestator_id`,`memory_area`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='List of all areas that needs to be attested at startup (associated t their attestator)';

-- --------------------------------------------------------

--
-- Table structure for table `ra_prepared_data`
--

CREATE TABLE IF NOT EXISTS `ra_prepared_data` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT 'DB id for prepared data',
  `attestator_id` bigint(20) unsigned NOT NULL COMMENT 'Foreign key: refers to the DB id of the attestator record associated to the prepared data',
  `nonce` tinyblob NOT NULL COMMENT 'Nonce value used to create this prepared data',
  `data` longblob NOT NULL COMMENT 'Prepared data form which calculate the attestation data to compare with the attestator returned ones',
  `memory_area` smallint(5) unsigned NOT NULL COMMENT 'Label of the memory area this prepared data is associated to',
  PRIMARY KEY (`id`),
  KEY `attestator_id` (`attestator_id`)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1 COMMENT='List of all the nonces and associated prepared data used to craft RA requests and verify RA responses';


-- --------------------------------------------------------

--
-- Table structure for table `ra_reaction`
--

CREATE TABLE IF NOT EXISTS `ra_reaction` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT 'DB id for application status',
  `application_id` bigint(20) unsigned NOT NULL COMMENT 'Foreign key: refers to the DB id of the application this status is associated to',
  `reaction_status_id` bigint(20) NOT NULL COMMENT 'Foreign key: refers to DB id of the Reaction status enumeration value this status is associated to',
  PRIMARY KEY (`id`),
  KEY `ra_reaction_status_ibfk_1` (`application_id`),
  KEY `ra_reaction_ibfk_2` (`reaction_status_id`)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1 COMMENT='Current status of the application';

-- --------------------------------------------------------

--
-- Table structure for table `ra_reaction_status`
--

CREATE TABLE IF NOT EXISTS `ra_reaction_status` (
  `id` bigint(20) NOT NULL COMMENT 'DB if for the reaction status',
  `name` varchar(32) NOT NULL COMMENT 'Enumerative name of the status',
  `decription` varchar(50) DEFAULT NULL COMMENT 'Optional textual description of the status value',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='List of possible application statuses';

--
-- Dumping data for table `ra_reaction_status`
--

INSERT INTO `ra_reaction_status` (`id`, `name`, `decription`) VALUES
(0, 'NONE', 'Application is not tampered, reaction not required'),
(1, 'COMPROMISED', 'Application is tampered, reaction required'),
(2, 'UNKNOWN', 'Application is started but not yet proved as intact');

-- --------------------------------------------------------

--
-- Table structure for table `ra_request`
--

CREATE TABLE IF NOT EXISTS `ra_request` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT 'DB id for RA request',
  `prepared_data_id` bigint(20) unsigned NOT NULL COMMENT 'Foreign key: refers to the prepared data DB id this request is associated to',
  `session_id` bigint(20) unsigned NOT NULL COMMENT 'Foreign key: refers to DB id of the associated record in the ra_session table',
  `send_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'Timestamp relative to the moment when the request has been sent',
  `response_time` int(10) unsigned DEFAULT NULL COMMENT 'Number of seconds passed between RA request sent and response received',
  `status` bigint(20) NOT NULL DEFAULT '0' COMMENT 'Foreign key: refers to the response status DB id this request is associated to',
  `validity_time` smallint(5) unsigned NOT NULL DEFAULT '1800' COMMENT 'Default 30''. Max 1092'' (18h)',
  `is_startup` tinyint(1) NOT NULL DEFAULT '0' COMMENT 'True if the request is relative to a startup attestation, false otherwise',
  PRIMARY KEY (`id`),
  KEY `prepared_data_id` (`prepared_data_id`),
  KEY `status` (`status`),
  KEY `session_id` (`session_id`)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1 COMMENT='List of all the sent RA requests';

--
-- Triggers `ra_request`
--
DROP TRIGGER IF EXISTS `startup_attestation`;
DELIMITER //
CREATE TRIGGER `startup_attestation` AFTER UPDATE ON `ra_request`
 FOR EACH ROW BEGIN
	
          
    
    IF NEW.is_startup = 1 AND NEW.status = 1
    THEN
    	
		SET @reaction_status = 
					( 	SELECT str.id

						FROM ra_request as req, 
							ra_prepared_data as dp, 
							ra_attestator as att, 
							ra_application as app, 
							ra_reaction as stat, 
							ra_reaction_status as str
							
						WHERE req.prepared_data_id = dp.id AND 
							dp.attestator_id = att.id AND 
							att.application_id = app.id AND 
							stat.application_id = app.id AND
							stat.reaction_status_id = str.id AND
							req.id = NEW.id);
							
	
		
		IF @reaction_status = 2 
		THEN
			SET @app_id = 
						(	SELECT 
								app.id AS a_app
								
							FROM 
								ra_attestator AS att,
								ra_application AS app,
								ra_prepared_data AS pd,
								ra_request AS req
								
							WHERE
								att.application_id = app.id AND
								pd.attestator_id = att.id AND
								req.prepared_data_id = pd.id 
								AND	req.id = NEW.id);
                             
                             
			SET @is_app_proved = 
						(	SELECT COUNT(*)
							FROM (
							SELECT a_app, SUM(should_attest_areas) AS saa , SUM(valid_startup_areas) AS vsa
							FROM 	
								(SELECT
									app.id AS a_app, 
									att.id AS a_att, 
									COUNT(area.memory_area) AS should_attest_areas

								FROM
									ra_attest_at_startup_area AS area,
									ra_attestator AS att,
									ra_application AS app
									
								WHERE 
									att.id = area.attestator_id AND
									app.id = att.application_id AND
									app.id = @app_id

								GROUP BY app.id, att.id	) AS should
								
								LEFT JOIN 
								
							   (SELECT 
									app.id AS app, 
									att.id AS att,
									COUNT(*) AS valid_startup_areas

								FROM 
									ra_application AS app,
									ra_attestator AS att,
									ra_session AS ses,
									ra_request AS req

								WHERE
									app.id = att.application_id AND
									att.id = ses.attestator_id AND
									ses.id = req.session_id AND

									req.status = 1 AND
									req.is_startup = TRUE AND
									ses.is_active = TRUE AND
									app.id = @app_id

								GROUP BY
									app.id, 
									att.id) AS valid 
								
								ON should.a_att = valid.att
								
								GROUP BY a_app) as derived
							WHERE saa = vsa);
                            
					
			IF @is_app_proved = 1
            THEN
				UPDATE ra_reaction
				SET reaction_status_id = 0
				WHERE application_id = @app_id;
            END IF;
		END IF;
	
    END IF;

END
//
DELIMITER ;

-- --------------------------------------------------------

--
-- Table structure for table `ra_session`
--

CREATE TABLE IF NOT EXISTS `ra_session` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Primary Key',
  `attestator_id` bigint(20) unsigned NOT NULL COMMENT 'Foreign key: refers to the attestator record (DB id)',
  `start` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'Timestamp relative to the moment when the attestator connects to the RA manager',
  `finish` timestamp NULL DEFAULT NULL COMMENT 'Timestamp relative to the moment when the attestator disconnects from the RA manager',
  `is_active` tinyint(1) NOT NULL COMMENT 'States wether the session is currently active or not',
  PRIMARY KEY (`id`),
  KEY `attestator_id` (`attestator_id`)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `ra_status`
--

CREATE TABLE IF NOT EXISTS `ra_status` (
  `id` bigint(20) NOT NULL COMMENT 'DB id for RA request status',
  `name` varchar(32) NOT NULL COMMENT 'Enumerative name of the request status',
  `decription` varchar(50) DEFAULT NULL COMMENT 'Optional textual description of the request status value',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='List of all the possible RA request statuses';

--
-- Dumping data for table `ra_status`
--

INSERT INTO `ra_status` (`id`, `name`, `decription`) VALUES
(0, 'PENDING', 'Pending request'),
(1, 'SUCCESS', 'Right response received in time'),
(2, 'FAILED', 'Wrong response received in time'),
(3, 'EXPIRED_SUCCESS', 'Right response received out of time'),
(4, 'EXPIRED_FAILED', 'Wrong response received out of time'),
(5, 'EXPIRED_NONE', 'No response received');

-- --------------------------------------------------------


--
-- Constraints for dumped tables
--

--
-- Constraints for table `ra_attestator`
--
ALTER TABLE `ra_attestator`
  ADD CONSTRAINT `ra_attestator_ibfk_2` FOREIGN KEY (`application_id`) REFERENCES `ra_application` (`id`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Constraints for table `ra_attest_at_startup_area`
--
ALTER TABLE `ra_attest_at_startup_area`
  ADD CONSTRAINT `ra_attest_at_startup_area_ibfk_1` FOREIGN KEY (`attestator_id`) REFERENCES `ra_attestator` (`id`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Constraints for table `ra_prepared_data`
--
ALTER TABLE `ra_prepared_data`
  ADD CONSTRAINT `ra_prepared_data_ibfk_2` FOREIGN KEY (`attestator_id`) REFERENCES `ra_attestator` (`id`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Constraints for table `ra_reaction`
--
ALTER TABLE `ra_reaction`
  ADD CONSTRAINT `ra_reaction_ibfk_4` FOREIGN KEY (`application_id`) REFERENCES `ra_application` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `ra_reaction_ibfk_5` FOREIGN KEY (`reaction_status_id`) REFERENCES `ra_reaction_status` (`id`);

--
-- Constraints for table `ra_request`
--
ALTER TABLE `ra_request`
  ADD CONSTRAINT `ra_request_ibfk_7` FOREIGN KEY (`session_id`) REFERENCES `ra_session` (`id`),
  ADD CONSTRAINT `ra_request_ibfk_4` FOREIGN KEY (`prepared_data_id`) REFERENCES `ra_prepared_data` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  ADD CONSTRAINT `ra_request_ibfk_5` FOREIGN KEY (`status`) REFERENCES `ra_status` (`id`) ON DELETE CASCADE ON UPDATE CASCADE;

--
-- Constraints for table `ra_session`
--
ALTER TABLE `ra_session`
  ADD CONSTRAINT `ra_session_ibfk_1` FOREIGN KEY (`attestator_id`) REFERENCES `ra_attestator` (`id`) ON DELETE CASCADE ON UPDATE CASCADE;


/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
