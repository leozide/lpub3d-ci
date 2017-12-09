#!/bin/bash
#
# Build all LPub3D 3rd-party renderers
#
#  Trevor SANDY <trevor.sandy@gmail.com>
#  Last Update: December 08, 2017
#  Copyright (c) 2017 by Trevor SANDY
#

# sample commands [call from root build directory - e.g. lpub3d]:
# -export WD=$PWD; export DOCKER=true; chmod +x builds/utilities/CreateRenderers.sh && ./builds/utilities/CreateRenderers.sh
# -export OBS=false; source ${SOURCE_DIR}/builds/utilities/CreateRenderers.sh

# NOTE: OBS flag is 'ON' by default, be sure to set this flag in your calling command accordingly
# NOTE: elevated access required for dnf builddeps, execute with sudo if running noninteractive

# Capture elapsed time - reset BASH time counter
SECONDS=0
FinishElapsedTime() {
  # Elapsed execution time
  ELAPSED="Elapsed build time: $(($SECONDS / 3600))hrs $((($SECONDS / 60) % 60))min $(($SECONDS % 60))sec"
  echo "----------------------------------------------------"
  echo "$ME Finished!"
  echo "$ELAPSED"
  echo "----------------------------------------------------"
}

# Grab te script name
ME="$(basename "$(test -L "$0" && readlink "$0" || echo "$0")")"

# Grab the calling dir
CallDir=$PWD

# tell curl to be silent, continue downloads and follow redirects
curlopts="-sL -C -"

# Start message and set sourced flag
if [ "${ME}" = "CreateRenderers.sh" ]; then
  Info "Start $ME execution at $PWD..."
  SOURCED="false"
else
  Info "Start CreateRenderers execution at $PWD..."
  SOURCED="true"
fi

# Get platform
OS_NAME=`uname`
if [ "$OS_NAME" = "Darwin" ]; then
  platform=$(echo `sw_vers -productName` `sw_vers -productVersion`)
else
  platform=$(. /etc/os-release && if test "$PRETTY_NAME" != ""; then echo "$PRETTY_NAME"; else echo `uname`; fi)
fi

# Functions
Info () {
    if [ "${SOURCED}" = "true" ]
    then
        echo "   renderers: ${*}" >&2
    else
        echo "-${*}" >&2
    fi
}

ExtractArchive() {
  # args: $1 = <build folder>, $2 = <valid subfolder>
  Info "Extracting $1.tar.gz..."
  mkdir -p $1 && tar -mxzf $1.tar.gz -C $1 --strip-components=1
  if [ -d $1/$2 ]; then
    Info "Archive $1.tar.gz successfully extracted."
    rm -rf $1.tar.gz && Info "Cleanup archive $1.tar.gz."
    cd $1
  else
    Info "ERROR - $1.tar.gz did not extract properly."
  fi
}

# args: $1 = <log file>, $2 = <position>
DisplayLogTail() {
  if [[ -f "$1" && -s "$1" ]]; then
    logFile="$1"
    if [ "$2" = "" ]; then
      # default to 5 lines from the bottom of the file if not specified
      startPosition=-5
    elif [[ "${2:0:1}" != "-" || "${2:0:1}" != "+" ]]; then
      # default to the bottom of the file if not specified
      startPosition=-$2
    else
      startPosition=$2
    fi
    Info "$1 last $2 lines..."
    tail $startPositin $logFile
  else
    Info "ERROR (log tail) - $1 not found or not valid!"
  fi
}

# args: $1 = <log file>, $2 = <search String>, $3 = <lines Before>, $4 = <lines After>
DisplayCheckStatus() {
  declare -i i; i=0
  for arg in "$@"; do
    i=i+1
    if test $i -eq 1; then s_buildLog="$arg"; fi
    if test $i -eq 2; then s_checkString="$arg"; fi
    if test $i -eq 3; then s_linesBefore="$arg"; fi
    if test $i -eq 4; then s_linesAfter="$arg"; fi
  done
  if [[ -f "$s_buildLog" && -s "$s_buildLog" ]]; then
    if test -z "$s_checkString"; then Info "ERROR - check string not specified."; return 1; fi
    if test -z "$s_linesBefore"; then s_linesBefore=2; Info "INFO - display 2 lines before"; fi
    if test -z "$s_linesAfter"; then s_linesAfter=10; Info "INFO - display 10 lines after"; fi
    grep -B${s_linesBefore} -A${s_linesAfter} "${s_checkString}" $s_buildLog
  else
    Info "ERROR - Check display [$s_buildLog] not found or is not valid!"
  fi
}

# args: 1 = <pid>, 2 = <message interval>, [3 = <pretty label>]
TreatLongProcess() {
  declare -i i; i=0
  for arg in "$@"; do
    i=i+1
    if test $i -eq 1; then s_pid="$arg"; fi    # pid
    if test $i -eq 2; then s_msgint="$arg"; fi # message interval
    if test $i -eq 3; then s_plabel="$arg"; fi # pretty label
  done
  # Validate the optional pretty label
  if test -z "$s_plabel"; then s_plabel="Create renderer"; fi
  
  # Spawn a process that coninually reports the command is running
  while Info "$(date): $s_plabel process $s_pid is running..."; do sleep $s_msgint; done &
  messenger=$!

  # Set a trap to kill the messenger when the process finishes
  trap 'kill $messenger' 0

  # Wait for the process to finish
  if wait $s_pid; then
    Info "$s_plabel finished (returned $?)"
  else
    Info "$s_plabel FAILED! (returned $?)"
  fi
}

# args: 1 = <build folder>
InstallDependencies() {
  if [ "$OS_NAME" = "Linux" ]; then
    Info &&  Info "Install $1 build dependencies..."
    platform_=$(. /etc/os-release && echo $ID)
    useSudo="sudo"
    Info "Using sudo..........[Yes]"
    case ${platform_} in
    fedora|redhat|suse|mageia|arch|ubuntu|debian)
      true
      ;;
    *)
      if [ "$OBS" = "true" ]; then
        if [ "$TARGET_VENDOR" != "" ]; then
          platform_=$TARGET_VENDOR
        else
          Info "ERROR - Open Build Service did not provide a target platform."
        fi
      else
        Info "ERROR - Unable to process this target platform: [$platform_]."
      fi
      ;;
    esac
    depsLog=${LOG_PATH}/${ME}_deps_${1}.log
    Info "Platform............[${platform_}]"
    case ${platform_} in
    fedora|redhat|suse|mageia)
      # Initialize install mesa
      case $1 in
      ldglite)
        specFile="$PWD/obs/ldglite.spec"
        buildOSMesa=1
        ;;
      ldview)
        cp -f QT/LDView.spec QT/ldview-lp3d-qt5.spec
        sed -e 's/define qt5 0/define qt5 1/g' -e 's/kdebase-devel/make/g' -e 's/, kdelibs-devel//g' -i QT/ldview-lp3d-qt5.spec
        specFile="$PWD/QT/ldview-lp3d-qt5.spec"
        buildOSMesa=1
        ;;
      povray)
        specFile="$PWD/unix/obs/povray.spec"
       ;;
      esac;
      debbuildDeps="TBD"
      Info "Spec File...........[${specFile}]"
      Info "Dependencies List...[${debbuildDeps}]"
      if [[ ${buildOSMesa} = 1 && ! ${OSMesaBuilt} = 1 ]]; then
        mesaSpecDir="$CallDir/builds/utilities/mesa"
        mesaBuildDeps="TBD"
        mesaDepsLog=${LOG_PATH}/${ME}_mesadeps_${1}.log
        mesaBuildLog=${LOG_PATH}/${ME}_mesabuild_${1}.log
        
        Info "Update OSMesa.......[Yes]"
        Info "OSMesa Dependencies.[${mesaBuildDeps}]"
        Info
        $useSudo dnf builddep -y mesa > $mesaDepsLog 2>&1
        Info "${1} Mesa dependencies installed." && DisplayLogTail $mesaDepsLog 10
        $useSudo dnf builddep -y "${mesaSpecDir}/glu.spec" >> $mesaDepsLog 2>&1
        Info "${1} GLU dependencies installed." && DisplayLogTail $mesaDepsLog 5

        Info && Info "Build OSMesa and GLU static libraries..."
        chmod +x "${mesaSpecDir}/buildosmesa.sh"
        if [ "${OBS}" = "true" ]; then
          "${mesaSpecDir}/buildosmesa.sh" &
        else
          "${mesaSpecDir}/buildosmesa.sh" > $mesaBuildLog 2>&1 &
        fi
        TreatLongProcess $! 60 "OSMesa and GLU build"
        
        if [[ -f "$WD/${DIST_DIR}/mesa/lib/libOSMesa32.a" && \
              -f "$WD/${DIST_DIR}/mesa/lib/libGLU.a" ]]; then
          Info &&  Info "OSMesa and GLU build check..."
          DisplayCheckStatus "$mesaBuildLog" "Libraries have been installed in:" "1" "16"
          OSMesaBuilt=1
        else
          if [ ! -f "$WD/${DIST_DIR}/mesa/lib/libOSMesa32.a" ]; then
            Info && Info "ERROR - libOSMesa32 not found. Binary was not successfully built"
          fi
          if [ ! -f "$WD/${DIST_DIR}/mesa/lib/libGLU.a" ]; then
            Info && Info "ERROR - libGLU not found. Binary was not successfully built"
          fi
          DisplayLogTail $mesaBuildLog 10
        fi
        Info && Info "${1} library OSMesa build finished."
      fi
      Info
      $useSudo dnf builddep -y $specFile > $depsLog 2>&1
      Info "${1} dependencies installed." && DisplayLogTail $depsLog 10
      ;;
    arch)
      case $1 in
      ldglite)
        pkgbuildFile="$PWD/obs/PKGBUILD"
        ;;
      ldview)
        pkgbuildFile="$PWD/QT/OBS/PKGBUILD"
        $useSudo mkdir /usr/share/mime
        ;;
      povray)
        pkgbuildFile="$PWD/unix/obs/PKGBUILD"
        ;;
      esac;
      pkgbuildDeps="$(echo `grep -wr 'depends' $pkgbuildFile | cut -d= -f2| sed -e 's/(//g' -e "s/'//g" -e 's/)//g'` \
                           `grep -wr 'makedepends' $pkgbuildFile | cut -d= -f2| sed -e 's/(//g' -e "s/'//g" -e 's/)//g'`)"
      pkgbuildDeps="$(echo "$pkgbuildDeps" | sed 's/kdelibs//g')"
      Info "PKGBUILD File.......[${pkgbuildFile}]"
      Info "Dependencies List...[${pkgbuildDeps}]"
      Info
      $useSudo pacman -Syy --noconfirm --needed > $depsLog 2>&1
      $useSudo pacman -Syu --noconfirm --needed >> $depsLog 2>&1
      $useSudo pacman -S --noconfirm --needed $pkgbuildDeps >> $depsLog 2>&1
      Info "${1} dependencies installed." && DisplayLogTail $depsLog 10
      ;;
    ubuntu|debian)
      case $1 in
      ldglite)
        controlFile="$PWD/obs/debian/control"
        ;;
      ldview)
        sed -e '/#Qt4.x/d' -e '/libqt4-dev/d' -e 's/#Build-Depends/Build-Depends/g' \
            -e 's/kdelibs5-dev//g' -e '/^Build-Depends:/ s/$/ libtinyxml-dev libgl2ps-dev/' -i QT/debian/control
        controlFile="$PWD/QT/debian/control"
        ;;
      povray)
        controlFile="$PWD/unix/obs/debian/control"
        ;;
      esac;
      controlDeps=`grep Build-Depends $controlFile | cut -d: -f2| sed 's/(.*)//g' | tr -d ,`
      Info "Control File........[${controlFile}]"
      Info "Dependencies List...[${controlDeps}]"
      Info
      $useSudo apt-get update -qq > $depsLog 2>&1
      $useSudo apt-get install -y $controlDeps >> $depsLog 2>&1
      Info "${1} dependencies installed." && DisplayLogTail $depsLog 10
      ;;
      *)
      Info "ERROR - Unknown platform [$platform_]"
      ;;
    esac;
  else
    Info "ERROR - Platform is undefined or invalid [$OS_NAME] - Cannot continue."
  fi
}

# args: 1 = <build type (release|debug)>, 2 = <build log>
BuildLDGLite() {
  BUILD_CONFIG="CONFIG+=BUILD_CHECK"
  if [ "$1" = "debug" ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=debug"
  else
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=release"
  fi
  if [ ${buildOSMesa} = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=USE_OSMESA_STATIC"
  fi
  ${QMAKE_EXE} CONFIG+=3RD_PARTY_INSTALL=../../${DIST_DIR} ${BUILD_CONFIG}
  if [ "${OBS}" = "true" ]; then
    make
    make install
  else
    make > $2 2>&1
    make install >> $2 2>&1
  fi
}

# args: 1 = <build type (release|debug)>, 2 = <build log>
BuildLDView() {
  BUILD_CONFIG="CONFIG+=BUILD_CUI_ONLY CONFIG+=USE_SYSTEM_LIBS CONFIG+=BUILD_CHECK"
  if [ "$1" = "debug" ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=debug"
  else
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=release"
  fi
  if [ ${buildOSMesa} = 1 ]; then
    BUILD_CONFIG="$BUILD_CONFIG CONFIG+=USE_OSMESA_STATIC"
  fi
  ${QMAKE_EXE} CONFIG+=3RD_PARTY_INSTALL=../../${DIST_DIR} ${BUILD_CONFIG}
  if [ "${OBS}" = "true" ]; then
    make
    make install
  else
    make > $2 2>&1 &
    TreatLongProcess "$!" "60" "LDView make"
    make install >> $2 2>&1
  fi
}

# args: 1 = <build type (release|debug)>, 2 = <build log>
BuildPOVRay() {
  BUILD_CONFIG="--prefix=${DIST_PKG_DIR} LPUB3D_3RD_PARTY=yes --with-libsdl2 --enable-watch-cursor"
  if [ "$1" = "debug" ]; then
    BUILD_CONFIG="$BUILD_CONFIG --enable-debug"
  fi
  export POV_IGNORE_SYSCONF_MSG="yes"
  cd unix && chmod +x prebuild3rdparty.sh && ./prebuild3rdparty.sh && cd ../
  ./configure COMPILED_BY="Trevor SANDY <trevor.sandy@gmail.com> for LPub3D." ${BUILD_CONFIG}
  if [ "${OBS}" = "true" ]; then
    make
    make install
  else
    make > $2 2>&1 &
    TreatLongProcess "$!" "60" "POV-Ray make" 
    make check >> $2 2>&1
    make install >> $2 2>&1
  fi
}

Info && Info "Building............[LPub3D 3rd Party Renderers]"

# Check for required 'WD' variable
if [ "${WD}" = "" ]; then
  parent_dir=${PWD##*/}
  if  [ "$parent_dir" = "utilities" ]; then
    if [ "$OS_NAME" = "Darwin" ]; then
      chkdir="$(realpath ../../../)"
    else
      chkdir="$(readlink - ../../../)"
    fi
    if [ -d "$chkdir" ]; then
      WD=$chkdir
    fi
  else
    WD=$PWD
  fi
  Info "WARNING - 'WD' environment varialbe not specified. Using $WD"
fi

# Initialize OBS if not in command line input
if [[ "${OBS}" = "" && "${DOCKER}" = "" &&  "${TRAVIS}" = "" ]]; then
  OBS=true
fi

# Platform ID
if [ "${DOCKER}" = "true" ]; then
  Info "Platform............[Docker - ${platform}]"
elif [ "${TRAVIS}" = "true" ]; then
  Info "Platform............[Travis CI - ${platform}]"
elif [ "${OBS}" = "true" ]; then
  if [ "$platform" = "" ]; then
    platform=$(echo `uname`)
  fi
  Info "Platform............[Open Build System - ${platform}]"
else
  Info "Platform............[${platform}]"
fi
Info "Working directory...[$WD]"

# Distribution directory
if [ "$OS_NAME" = "Darwin" ]; then
  DIST_DIR=lpub3d_macos_3rdparty
else
  DIST_DIR=lpub3d_linux_3rdparty
fi
DIST_PKG_DIR=${WD}/${DIST_DIR}
if [ ! -d ${DIST_PKG_DIR} ]; then
  mkdir -p ${DIST_PKG_DIR} && Info "Dist Directory......[${DIST_PKG_DIR}]"
fi

# Change to Working directory
cd ${WD}

# Setup LDraw Library - for testing LDView and LDGLite
if [ "$OS_NAME" = "Darwin" ]; then
  LDRAWDIR_ROOT=${HOME}/Library
else
  LDRAWDIR_ROOT=${HOME}
fi
export LDRAWDIR=${LDRAWDIR_ROOT}/ldraw
if [ ! -d ${LDRAWDIR}/parts ]; then
  Info && Info "LDraw library not found at ${LDRAWDIR}. Checking for complete.zip archive..."
  if [ ! -f complete.zip ]; then
    Info "Library archive complete.zip not found at $PWD. Downloading archive..."
    curl -s -O http://www.ldraw.org/library/updates/complete.zip;
  fi
  Info "Extracting LDraw library into ${LDRAWDIR}..."
  unzip -of -d ${LDRAWDIR_ROOT} -q complete.zip;
  if [ -d ${LDRAWDIR} ]; then
    Info "LDraw library extracted. LDRAWDIR defined."
  fi
elif [ ! "$OS_NAME" = "Darwin" ]; then
  Info "LDraw library.......[${LDRAWDIR}]"
fi
#additional LDraw configuration for MacOS
if [ "$OS_NAME" = "Darwin" ]; then
  Info "LDraw library.......[${LDRAWDIR}]"
  Info && Info "set LDRAWDIR in environment.plist..."
  chmod +x ${LPUB3D}/builds/utilities/set-ldrawdir.command && ./${LPUB3D}/builds/utilities/set-ldrawdir.command
  grep -A1 -e 'LDRAWDIR' ~/.MacOSX/environment.plist
  Info "set LDRAWDIR Completed."
  
  # Qt setup - MacOS
  QMAKE_EXE=qmake
else
  # Qt setup - Linux
  export QT_SELECT=qt5
  if [ -x /usr/bin/qmake ] ; then
    QMAKE_EXE=qmake
  elif [ -x /usr/bin/qmake-qt5 ] ; then
    QMAKE_EXE=qmake-qt5
  fi
fi

# get Qt version
Info && ${QMAKE_EXE} -v
QMAKE_EXE="${QMAKE_EXE} -makefile"

# set log output path
LOG_PATH=${WD}

# initialize mesa build flag
buildOSMesa=0
OSMesaBuilt=0

# define build architecture
if [ $(uname -m) = x86_64 ]; then 
  buildArch="64bit_release"; 
else 
  buildArch="32bit_release"; 
fi

# install build dependencies for MacOS
if [ "$OS_NAME" = "Darwin" ]; then
  Info &&  Info "Install $OS_NAME build dependencies..."
  brewDeps="openexr sdl2 tinyxml gl2ps libtiff libjpeg minizip"
  Info "Platform............[macos]"
  Info "Using sudo..........[No]"
  Info "Dependencies List...[${brewDeps}]"
  depsLog=${LOG_PATH}/${ME}_deps_$OS_NAME.log
  brew update > $depsLog 2>&1
  brew install $brewDeps >> $depsLog 2>&1
  Info "$OS_NAME dependencies installed." && DisplayLogTail $depsLog 10
fi

# Main loop
for buildDir in ldglite ldview povray; do
  buildLog=${LOG_PATH}/${ME}_build_${buildDir}.log
  linesBefore=1
  case ${buildDir} in
  ldglite)
    curlCommand="https://github.com/trevorsandy/ldglite/archive/master.tar.gz"
    checkString="LDGLite Output"
    linesAfter="2"
    buildCommand="BuildLDGLite"
    validSubDir="app"
    validExe="${validSubDir}/${buildArch}/ldglite"
    buildType="release"
    ;;
  ldview)
    curlCommand="https://github.com/trevorsandy/ldview/archive/qmake-build.tar.gz"
    checkString="LDView Image Output"
    linesAfter="9"
    buildCommand="BuildLDView"
    validSubDir="OSMesa"
    validExe="${validSubDir}/${buildArch}/ldview"
    buildType="release"
    ;;
  povray)
    curlCommand="https://github.com/trevorsandy/povray/archive/lpub3d/raytracer-cui.tar.gz"
    checkString="Render Statistics"
    linesAfter="42"
    buildCommand="BuildPOVRay"
    validSubDir="unix"
    validExe="${validSubDir}/lpub3d_trace_cui"
    buildType="release"
    ;;
  esac
  if [ "$OBS" = "true" ]; then
    if [ -f ${buildDir}.tar.gz ]; then
      ExtractArchive ${buildDir} ${validSubDir}
    else
      Info && Info "ERROR - Unable to find ${buildDir}.tar.gz at $PWD"
    fi
  elif [ ! -d ${buildDir}/${validSubDir} ]; then
    Info && Info "`echo ${buildDir} | awk '{print toupper($0)}'` build folder does not exist. Checking for tarball archive..."
    if [ ! -f ${buildDir}.tar.gz ]; then
      Info "`echo ${buildDir} | awk '{print toupper($0)}'` tarball ${buildDir}.tar.gz does not exist. Downloading..."
      curl $curlopts ${curlCommand} -o ${buildDir}.tar.gz
    fi
    ExtractArchive ${buildDir} ${validSubDir}
  else
    cd ${buildDir}
  fi
  if [ ! "$OS_NAME" = "Darwin" ]; then
    InstallDependencies ${buildDir}
  fi
  sleep .5
  Info && Info "Build ${buildDir}..."
  Info "----------------------------------------------------"
  ${buildCommand} ${buildType} ${buildLog}
  if [ -f "${validExe}" ]; then
    Info && Info "Build check - ${buildDir}..."
    DisplayCheckStatus "${buildLog}" "${checkString}" "${linesBefore}" "${linesAfter}"
  else
    Info && Info "ERROR - ${validExe} not found. Binary was not successfully built"
  fi
  DisplayLogTail ${buildLog} 10
  Info && Info "Build ${buildDir} finished."
  cd ${WD}
done

# Elapsed execution time
FinishElapsedTime
