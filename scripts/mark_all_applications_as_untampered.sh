#!/bin/bash

baseDir=$(dirname $0)
. $baseDir/deploy.env

echo "Application: $dbAppId"
echo "mysql --defaults-file=$baseDir/mysql.cnf -e \"UPDATE ra_reaction SET reaction_status_id = 0 WHERE 1"
mysql --defaults-file=$baseDir/mysql.cnf -e "UPDATE ra_reaction SET reaction_status_id = 0 WHERE 1"

