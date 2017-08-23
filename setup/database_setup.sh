#!/bin/bash
set -o errexit
set -o pipefail
set -o nounset
#set -o xtrace

baseDir=$(dirname $0)

. $baseDir/config.env

createUserSqlDir=$baseDir/$env_usersDirName
createTablesSqlDir=$baseDir/$env_tablesDirName

function checkError {
	if [ $1 == 1 ]; then
		echo "--- Fatal Error ---"
		exit 1
	fi
}

while getopts "fl:s:" opt
do
#	echo "Got $opt opt"
	case $opt in

	f)
		# Force setup from scratch
		forceDrop=true
		;;
	\?)
		echo "Invalid option: -$OPTARG"
		exit 1
		;;
	:)
		echo "Option -$OPTARG requires an argument."
		exit 1
		;;
	esac
done

if [ -v forceDrop ]; then
	# Dropping all RA database data

	echo "++++ Dropping database"
	mysql -u root --password=aspire -h $serverAddress -e "DROP DATABASE $raDbName"
	checkError $?

fi

# Try to create database
echo "++++ Setting up ASPIRE database"
mysql -u root --password=aspire -h $serverAddress -e "CREATE DATABASE $raDbName" &> /dev/null
if [ $? == 0 ]; then

	# Create database structure
	echo "++++ Creating users"
	for userScript in $(ls $createUserSqlDir)
	do
		echo "++++ Creating user for $userScript"
		mysql -u root --password=aspire -h $serverAddress < $createUserSqlDir/$userScript
		#checkError $? #removed due to a mySQL bug: error when try to create a user already existing
	done;

	for tablesScript in $(ls $createTablesSqlDir)
	do
		echo "++++ Creating tables for $tablesScript"
		mysql -u root --password=aspire -h $serverAddress $raDbName < $createTablesSqlDir/$tablesScript
		checkError $?
	done
else
	# Database already exists
	echo "++++ Database already setup"
fi
