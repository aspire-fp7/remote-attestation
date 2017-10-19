#!/bin/bash

waitKey() {
	if [ ! -z ${debug} ]; then
		read -p "Press any key to continue"
	fi
}


baseDir=$(dirname $0)
raDir=$(dirname $baseDir)
srcDir=$raDir/src
objDir=$raDir/obj

#
# Options management and check
#
while getopts "a:o:s:t:d" opt
do

	case $opt in

	a)
		annotationPath=$OPTARG
		echo "Annotation file: $annotationPath"
		if [ !  $annotationPath ]; then
			echo "Annotation file does not exist"
			exit 1
		fi
		;;
	o)
		outDir=$OPTARG
		echo "Output folder: $outDir"

		if [ ! -e $outDir ]; then
			echo "Output folder does not exist..."
			mkdir -vp $outDir
		fi
		;;
	s)
		srcDir=$OPTARG
		;;
	t)
		targetArch=$OPTARG
		if [[ $targetArch != "linux" && $targetArch != "android" ]]; then
			echo 'Only values allowed for option -t are "linux" and "android"'
			echo "     specified: $OPTARG"
			exit 1
		fi
		. $baseDir/config.$targetArch
		;;
	d)
		debug="debug_ON"
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

echo "Base folder: $baseDir"
echo "Source folder: $srcDir"
echo "Target architecture: $targetArch"
echo "Tools:"
echo "cc: $CC"
echo "ld: $LD"
echo "ar: $AR"
# check paramenters existance
if [ -z ${annotationPath} ]; then
	echo "Error: -a option is mandatory (extracted annotations file)"
	exit 1
fi

if [ -z ${outDir} ]; then
	echo "Error: -o(utput directory) option is mandatory"
	exit 1
fi

if [ -z ${targetArch} ]; then
        echo "Error: -t(arget architecture) option is mandatory (linux|android)"
        exit 1
fi

#
#	Annotation parsing and interpretation
#
interpreterOutputDir=$outDir/ra_annotation_output
interpreterCmdPath=$baseDir/ra_interpreter.jar
if [  $interpreterOutputDir ]; then 
	rm -rf $interpreterOutputDir
fi

mkdir $interpreterOutputDir
java -jar $interpreterCmdPath "parseAnnotations" $annotationPath $interpreterOutputDir || exit 1


waitKey

if [ ! "$(ls -A $interpreterOutputDir)" ]; then
	echo "No remote attestation annotations found"
	exit 0
fi

#
#	Characterization and building of parametric files
#
mkdir -pv $outDir/frequencies || exit 1
mv -v $interpreterOutputDir/*.freq $outDir/frequencies || exit 1


i=0
for f in $(ls $interpreterOutputDir)
do
	tmpOutDir=$outDir/tmp_$f
	rm -rf $tmpOutDir
	mkdir -vp $tmpOutDir


	echo "Generating attestator: '$f'"

	echo "Characterization: substitution of parametric symbols"

	# Selection of RA blocks parametric files
	#
	# For each block specified by annotation interpretation output...
	for block in $(sed "s#^.*:=\ ##g" $interpreterOutputDir/$f  | sed "s/$/.c/")
	do
		echo "Elaborating '$block'"
		filename=$(basename "$block")
		filename="${filename%.*}"

		# Characterization: substitution of parametric symbols
		sed "s/NAYjDD3l2s/$f/g" $srcDir/client-parametric-files/cond/$block > $tmpOutDir/$block
	done

	priority=$((65510-$i))

	# For each other parametric file...
	for file in $(ls $srcDir/client-parametric-files/uncond)
	do
		echo  "Elaborating '$file'"
		sed "s/NAYjDD3l2s/$f/g" $srcDir/client-parametric-files/uncond/$file | sed -e "s/#ATTESTATOR_PRIORITY#/$priority/g" > $tmpOutDir/$file
	done

	echo ""
	echo "Building: generate object files"
	for file in $(ls $tmpOutDir/*.c)
	do
		oName="${file%.*}".o
		echo  "Building '$oName'"
		$CC -I$srcDir/headers -I$thirdPartyIncludeLibWS -I$ACCLinclude -I$tmpOutDir -I$opensslInclude $C_FLAGS $DEBUG -c $file -o $oName || exit 1
	done

	echo ""
	echo "Linking: link object files into '$f.obj'"
	$LD -r  $tmpOutDir/*.o -o $outDir/$f.obj $LD_FLAGS || exit 1

	i=$(($i+1))

#	if [ -z ${debug} ]; then
#		rm -v $tmpOutDir/*.o $tmpOutDir/*.c $tmpOutDir/*.h
#	fi
	echo ""
 	waitKey
done

echo "Linking: link all attestators objects into '$outDir/attestators.o'"
$LD -r $outDir/*.obj -o $outDir/attestators.o $LD_FLAGS || exit 1

#echo "Packaging linked object file"
#$AR rcs $outDir/attestators.a $outDir/*.obj|| exit 1

#echo "Cleaning intermediate files"
#if [ -z ${debug} ]; then
#	rm -v $outDir/*.obj
#fi

cp -v $objDir/$targetArch/racommon.o $outDir || exit 1

echo "Attestators creaeted successfully"

#if [ -z ${debug} ]; then
#	rmdir -v $tmpOutDir
#fi
