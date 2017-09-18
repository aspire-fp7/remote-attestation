-- ATTENTION: This SQL script adds a column in the ra_request table. So, the script that creates the ra_request table must be called before this one.

--
-- Database: `RA_development`
--
-- --------------------------------------------------------
--
-- Table structure for table `rm_status`
--
-- --------------------------------------------------------

CREATE TABLE IF NOT EXISTS `rm_status` (
  `id` bigint(20) NOT NULL COMMENT 'DB id for RA request RM status',
  `name` varchar(32) NOT NULL COMMENT 'Enumerative name of the request RM status',
  `decription` varchar(255) DEFAULT NULL COMMENT 'Optional textual description of the request RM status value',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='List of all the possible RA request RM statuses';

-- --------------------------------------------------------
--
-- Dumping data for table `rm_status`
--
-- --------------------------------------------------------

INSERT INTO `rm_status` (`id`, `name`, `decription`) VALUES
(0, 'RM_PENDING', 'The ra_request has not been processed yet by the RM'),
(1, 'RM_COMPLETED', 'Request processed successfully: no further action is required on the ra_request'),
(2, 'RM_IN_PROGRESS', 'RM is currently processing the ra_request'),
(3, 'RM_EXPIRED', 'The ra_request is outdated should be dropped');

-- --------------------------------------------------------
--
-- Constraints and new column for table `ra_request`
--
-- --------------------------------------------------------
ALTER TABLE `ra_request`
  ADD `rm_status` bigint(20) NOT NULL DEFAULT '0' COMMENT 'Foreign key: refers to the RM status DB id this request is associated to',
  ADD CONSTRAINT `ra_request_ibfk_6` FOREIGN KEY (`rm_status`) REFERENCES `rm_status` (`id`) ON DELETE CASCADE ON UPDATE CASCADE;

