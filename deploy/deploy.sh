#!/bin/bash


baseDir=$(dirname $0)
filename=$(basename "$0")

. $baseDir/deploy.env

srcDir=$baseDir/../src

while getopts "b:a:e:p:" opt
do
#	echo "Got $opt opt"
	case $opt in

	a)
		echo "**** -a option: $OPTARG "
		stringAID=$OPTARG
		;;

	b)
		echo "**** -b option: $OPTARG "
		diabloOutPath=$(dirname $OPTARG)
		binaryName=$OPTARG
		;;

	e)
		echo "**** -e option: $OPTARG "
		blocksPath=$OPTARG/ra_annotation_output
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

echo "**** start Aspire Portal if it is not running ****"
PORTAL_RUNNING=$(pidof uwsgi)
[ "${PORTAL_RUNNING}" == "" ] && /opt/ASCL/aspire-portal/start-aspire-portal.sh

echo "**** Launching RA server setup ****"
/opt/RA/setup/remote_attestation_setup.sh || exit 1


echo "**** Deploying RA application components on server ****"
if [ -z ${stringAID} ]; then
	echo "AID option required: -a <AID string>"
fi

echo "**** New application RA script ****"
echo "AID: $stringAID"
echo "Diablo output dir: $diabloOutPath"
echo "Extractor output dir: $blocksPath"
echo "Binary name: $binaryName"
echo "$(ls $blocksPath/ | wc -w) attestators defined:"
echo "$(ls $blocksPath)"



# Add new application in the DB
echo "Removing application with AID data from database"
mysql -u ra_user -h localhost $raDbName -e "DELETE FROM ra_application WHERE AID = '$stringAID'"
echo "Adding new AID in database"
dbAppId=$(mysql -u ra_user -h localhost $raDbName \
	-B --disable-column-names -e \
	"INSERT INTO ra_application( id, AID ) VALUES (null, '$stringAID' ); SELECT LAST_INSERT_ID()")

mysql -u ra_user -h localhost $raDbName -e "INSERT INTO ra_reaction(id, application_id, reaction_status_id) VALUES (NULL, '$dbAppId', 2)" &>/dev/null

if [ ! "$(ls -A $blocksPath)" ]; then
        echo "No remote attestation extracted"
	echo "Nothing to do, exiting"
        exit 0
fi

for currAttestatorName in $(ls $blocksPath)
do

	echo "Working on attestator: $currAttestatorName"
	currAttestatorNumber=$(printf "%020lu" $((16#$(hexdump -s 16 -n 8 -v  $diabloOutPath/ads_$currAttestatorName -e '1/4 "%08X" "\n"' | tac | tr -d '\n'))))
	echo "Attestator number: $currAttestatorNumber"
	mkdir -pv $backendsDir/$stringAID/remote_attestation/$currAttestatorNumber/binaries || exit 1

	echo "Building server side application dependent components"
	echo "Block definition file: $blocksPath/$currAttestatorName"

	cd $baseDir
	makeOutTmpDir=./tmpOut || exit 1
	make -s all-specific-clean DBG=1 OUTDIR=$makeOutTmpDir SRCDIR=$srcDir BLOCKS_INCLUDE_FILE=$blocksPath/$currAttestatorName || exit 1
	mv -fv $makeOutTmpDir/verifier_exe 	$backendsDir/$stringAID/remote_attestation/$currAttestatorNumber/verifier || exit 1
	mv -fv $makeOutTmpDir/extractor_exe 	$backendsDir/$stringAID/remote_attestation/$currAttestatorNumber/extractor || exit 1
	rm -rfv $makeOutTmpDir || exit 1
	cd -

	cp -fv $diabloOutPath/ads_$currAttestatorName 	$backendsDir/$stringAID/remote_attestation/$currAttestatorNumber/binaries/ads || exit 1
	cp -fv $binaryName				$backendsDir/$stringAID/remote_attestation/$currAttestatorNumber/binaries/exe || exit 1

	echo "Extract attestator frequency"
	frequency=$(cat $blocksPath/../frequencies/$currAttestatorName.freq)

	echo "Add attestator in the DB (nr: $currAttestatorNumber, name: $currAttestatorName, f: $frequency)"
	currAttestatorDbId=$(mysql -u ra_user -h $serverAddress $raDbName -B --disable-column-names -e \
		"INSERT INTO ra_attestator(id, application_id, attestator_no, sleep_avg) VALUES (null, '$dbAppId' , '$currAttestatorNumber' , '$frequency' ); SELECT LAST_INSERT_ID()")

	echo "Generating initial 100 prepared data for current attestator (launching extractor)"
	$backendsDir/$stringAID/remote_attestation/$currAttestatorNumber/extractor $stringAID $currAttestatorNumber 32 100 > $blocksPath/extractor_$currAttestatorName.log #/dev/null

	if [ -z ${currAttestatorDbId} ];
	then
		echo "Error inserting attestator $currAttestatorNumber"
		exit 1
	else
		echo "Attestator inserted with ID: $currAttestatorDbId"
		totalStartupAreas=$(cat $diabloOutPath/startup_labels_$currAttestatorName | wc -l)

		echo "Inserting startup areas in the DB, found $totalStartupAreas areas"
		for areaLabel in $(cat $diabloOutPath/startup_labels_$currAttestatorName)
		do
			echo "Startup area: $areaLabel"
			mysql -u ra_user -h $serverAddress $raDbName -B --disable-column-names -e \
				"INSERT INTO ra_attest_at_startup_area(attestator_id, memory_area) VALUES ($currAttestatorDbId,$areaLabel)" > /dev/null
		done
	fi
	echo ""
done

echo "**** RA application components deployed on server successfully ****"
echo ""
exit 0
