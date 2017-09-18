CREATE USER 'rm_user'@'%' IDENTIFIED BY 'aspire';

GRANT ALL PRIVILEGES ON *.* TO 'rm_user'@'%' WITH GRANT OPTION;

GRANT ALL PRIVILEGES ON `RA_development`.* TO 'rm_user'@'%';
