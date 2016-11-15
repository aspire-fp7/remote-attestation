# -------------------------------------------------------------------------------------------- #
linuxOpensslInclude=/opt/3rd_party/openssl/linux/include/
androidOpensslInclude=/opt/3rd_party/openssl/android/include/

androidToolchainPrefix=/opt/diablo-android-gcc-toolchain/bin/arm-linux-androideabi-
linuxToolchainPrefix=/opt/diablo-gcc-toolchain/bin/arm-diablo-linux-gnueabi-

androidCC=$androidToolchainPrefix'gcc'
androidLD=$androidToolchainPrefix'ld'
androidAR=$androidToolchainPrefix'ar'
linuxCC=$linuxToolchainPrefix'gcc'
linuxLD=$linuxToolchainPrefix'ld'
linuxAR=$linuxToolchainPrefix'ar'

C_FLAGS='-march=armv7-a -mtune=cortex-a8 -std=c99 -mfloat-abi=softfp -msoft-float -mfpu=neon -fpic'
LD_FLAGS='--no-demangle --hash-style=sysv --no-merge-exidx-entries'
# -------------------------------------------------------------------------------------------- #



baseDir=$(dirname $0)/..
srcDir=$baseDir/src
outDir=$baseDir

while getopts "o:s:d" opt
do
#	echo "Got $opt opt"
	case $opt in

	o)
		#outDir=$(realpath $OPTARG)
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
	d)
		debug="-DDEBUG"
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


tmpOutFolder=$outDir/"tmp"
mkdir -p $tmpOutFolder

outDirLinux=$outDir/linux
outDirAndroid=$outDir/android
mkdir -p $outDirLinux
mkdir -p $outDirAndroid

$linuxCC   -I$srcDir/headers -I$srcDir/headers/external $C_FLAGS -c $srcDir/ra-additionals/external/blake2s-ref.c $debug -o $tmpOutFolder/linux_b2.o || (echo "FATAL ERROR" ; exit 1)
$androidCC -I$srcDir/headers -I$srcDir/headers/external $C_FLAGS -c $srcDir/ra-additionals/external/blake2s-ref.c $debug -D__ANDROID -o $tmpOutFolder/android_b2.o || (echo "FATAL ERROR" ; exit 1)

$linuxCC   -I$srcDir/headers -I$srcDir/headers/external -I$linuxOpensslInclude $C_FLAGS -c $srcDir/ra-additionals/generic_functions.c $debug -o $tmpOutFolder/linux_gf.o || (echo "FATAL ERROR" ; exit 1)
$androidCC -I$srcDir/headers -I$srcDir/headers/external -I$linuxOpensslInclude $C_FLAGS -c $srcDir/ra-additionals/generic_functions.c $debug -D__ANDROID -o $tmpOutFolder/android_gf.o || (echo "FATAL ERROR" ; exit 1)

$linuxCC   -I$srcDir/headers -I$srcDir/headers/external -I$linuxOpensslInclude $C_FLAGS -c $srcDir/ra-additionals/ra_print_wrapping.c $debug -o $tmpOutFolder/linux_pw.o || (echo "FATAL ERROR" ; exit 1)
$androidCC -I$srcDir/headers -I$srcDir/headers/external -I$linuxOpensslInclude $C_FLAGS -c $srcDir/ra-additionals/ra_print_wrapping.c $debug -D__ANDROID -o $tmpOutFolder/android_pw.o || (echo "FATAL ERROR" ; exit 1)

$linuxLD   -r $tmpOutFolder/linux_*.o -o $outDirLinux/racommon.o $LD_FLAGS || (echo "FATAL ERROR" ; exit 1)
$androidLD -r $tmpOutFolder/android_*.o -o $outDirAndroid/racommon.o $LD_FLAGS || (echo "FATAL ERROR" ; exit 1)

rm -v $tmpOutFolder/*
rm -rfv $tmpOutFolder









