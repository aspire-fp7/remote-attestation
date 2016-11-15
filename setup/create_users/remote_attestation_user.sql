CREATE USER 'ra_user'@'localhost';

GRANT ALL PRIVILEGES ON *.* TO 'ra_user'@'localhost' WITH GRANT OPTION;

GRANT ALL PRIVILEGES ON `RA_development`.* TO 'ra_user'@'localhost';
