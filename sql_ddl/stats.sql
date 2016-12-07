CREATE TABLE `stats` (
  `dns_name` char(255) NOT NULL DEFAULT '',
  `time_average` double NOT NULL DEFAULT '0',
  `time_deviation` double NOT NULL DEFAULT '0',
  `count` int(11) NOT NULL DEFAULT '0',
  `first_timestamp` int(11) NOT NULL DEFAULT '0',
  `last_timestamp` int(11) NOT NULL DEFAULT '0',
  UNIQUE KEY `dns_name` (`dns_name`)
) ENGINE=InnoDB DEFAULT CHARSET=ascii;
