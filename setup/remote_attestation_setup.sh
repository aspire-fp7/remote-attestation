#!/bin/bash


baseDir=$(dirname $0)
filename=$(basename "$0")
filename="${filename%.*}"
launchTime=$(date +"%Y_%m_%d__%H_%M_%S")
logFile=$baseDir/$filename"_"$launchTime.log

. $baseDir/config.env

createUserSqlDir=$baseDir/$env_usersDirName
createTablesSqlDir=$baseDir/$env_tablesDirName
srcDir=$baseDir/../src

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
	l)
		logFile=$OPTARG
		;;
	s)
		srcDir=$OPTARG
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

echo "**** RA server environment setup ****"

if [ ! -z ${forceDrop} ]; then
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



raManagerExe=$backendsDir/ra_manager
raForwarderExe=$backendsDir/ra_forwarder

if [[ -e $raManagerExe && -e $raForwarderExe && -z ${forceDrop} ]]; then
	echo "++++ Server side binaries already built"
else
	echo "++++ Building RA server side components"


	if [ ! -e $backendsDir ]; then
		mkdir -vp $backendsDir
	fi

	cd $baseDir
	makeOutTmpDir=./tmpOut
	make -s  all-indipendent-clean OUTDIR=$makeOutTmpDir SRCDIR=$srcDir ASCL=$asclObj DBG=1 || exit 1
	mv -v $makeOutTmpDir/verifier_forwarder_exe $raForwarderExe
	mv -v $makeOutTmpDir/manager_exe $raManagerExe
	rm -rfv $makeOutTmpDir
	cd -
fi

if [[ $(sudo ps aux | grep -v grep | grep ra_manager | wc -l) -eq 0 || ! -z ${forceDrop} ]]; then
	echo "++++ Making Manager running"
	echo "++++ (log file: $raManagerExe.log)"
	killall ra_manager
	coreNumber=$(cat /proc/cpuinfo | grep processor | wc -l)
	$raManagerExe $coreNumber 2>&1 | cat >> $raManagerExe.log &
	checkError $?
else
	echo "++++ Manager is already running"
fi

echo "**** Server environment setup completed****"
echo ""
