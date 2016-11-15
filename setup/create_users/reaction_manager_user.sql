CREATE USER 'rm_user'@'localhost';

GRANT ALL PRIVILEGES ON *.* TO 'rm_user'@'localhost' WITH GRANT OPTION;

GRANT ALL PRIVILEGES ON `RA_development`.* TO 'rm_user'@'localhost';
