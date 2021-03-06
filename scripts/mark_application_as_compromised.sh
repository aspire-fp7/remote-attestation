#!/bin/bash

if [  $# != 1 ]; then
	echo "AID parameter is needed"
	echo "Usage: $0 AID"
	exit 1
fi

AID=$1

baseDir=$(dirname $0)
filename=$(basename "$0")

. $baseDir/deploy.env

echo "mysql --defaults-file=$baseDir/mysql.cnf -B --disable-column-names -e \"SELECT id FROM ra_application WHERE AID = '$AID'\""
dbAppId=$(mysql --defaults-file=$baseDir/mysql.cnf -B --disable-column-names -e "SELECT id FROM ra_application WHERE AID = '$AID'")

if [ -z "$dbAppId" ];
then
	echo "Application $AID is not present in the DB"
	exit 1
fi

echo "Application: $dbAppId"
echo "mysql --defaults-file=$baseDir/mysql.cnf -e \"UPDATE ra_reaction SET reaction_status_id = 1 WHERE application_id = $dbAppId\""
mysql --defaults-file=$baseDir/mysql.cnf -e "UPDATE ra_reaction SET reaction_status_id = 1 WHERE application_id = $dbAppId"

