#!/bin/bash
#
# Deploy LPub3D assets to sourceforge.net using OpenSSH and rsync
#
#  Trevor SANDY <trevor.sandy@gmail.com>
#  Last Update: December 31, 2017
#  Copyright (c) 2017 by Trevor SANDY
#
#  Note: this script requires a host private key

# load environment variables
echo && echo "- Deploying to Sourceforge.net..."
echo && echo "- source set_bash_vars.sh..."
source builds/utilities/ci/set_bash_vars.sh

# set local update and download asset paths
LP3D_UPDATE_ASSETS=$LP3D_BUILD_UPDATE_TARGET
LP3D_DOWNLOAD_ASSETS=$LP3D_BUILD_DOWNLOAD_TARGET

# set working directory
sfParent_dir=${PWD##*/}
if  [ "$sfParent_dir" = "ci" ]; then
  sfChkdir="$(realpath ../../../../)"
  [ -d "$chkdir" ] && sfWD=$sfChkdir || sfWD=$PWD
else
  sfWD=$PWD
fi
echo && echo "  Working directory............[$sfWD]" && echo

# add host public key to known_hosts - prevent interactive prompt
LP3D_SF_REMOTE_HOST=216.34.181.57 # frs.sourceforge.net
[ ! -d ~/.ssh ] && mkdir -p ~/.ssh && touch ~/.ssh/known_hosts || \
[ ! -f ~/.ssh/known_hosts ] && touch ~/.ssh/known_hosts || true
[ -z `ssh-keygen -F $LP3D_SF_REMOTE_HOST` ] && ssh-keyscan -H $LP3D_SF_REMOTE_HOST >> ~/.ssh/known_hosts || \
echo && echo  "- Remote host public key for $LP3D_SF_REMOTE_HOST exist in known_hosts."

# add host private key to ssh-agent
if [ -f "builds/utilities/ci/secure/.sfdeploy_appveyor_rsa" ]; then
  mv -f "builds/utilities/ci/secure/.sfdeploy_appveyor_rsa" "/tmp/.sfdeploy_appveyor_rsa"
  eval "$(ssh-agent -s)"
  chmod 600 /tmp/.sfdeploy_appveyor_rsa
  ssh-add /tmp/.sfdeploy_appveyor_rsa
else
  echo && echo "  ERROR - builds/utilities/ci/secure/.sfdeploy_appveyor_rsa was not found."
fi

# upload assets
for OPTION in UDPATE DOWNLOAD; do
  case $OPTION in
  UDPATE)
    # Verify release files in the Update directory
    if [ -n "$(find "$LP3D_UPDATE_ASSETS" -maxdepth 0 -type d -empty 2>/dev/null)" ]; then
      echo && echo  "$LP3D_UPDATE_ASSETS is empty. Sourceforge.net update assets deploy aborted.";
    else
      echo && echo  "- Download Release Assets:" && find $LP3D_UPDATE_ASSETS -type f
      rsync -r --quiet $LP3D_UPDATE_ASSETS/* $SECURE_SF_UDPATE_CONNECT/
    fi
    ;;
  DOWNLOAD)
    # Verify release files in the Download directory
    if [ -n "$(find "$LP3D_DOWNLOAD_ASSETS" -maxdepth 0 -type d -empty 2>/dev/null)" ]; then
      echo && echo  "$LP3D_DOWNLOAD_ASSETS is empty. Sourceforge.net download assets deploy aborted.";
    else
      echo && echo  "- Download Release Assets:" && find $LP3D_DOWNLOAD_ASSETS -type f;
      rsync -r --quiet $LP3D_DOWNLOAD_ASSETS/* $SECURE_SF_DOWNLOAD_CONNECT/$LP3D_VERSION/
    fi
    ;;
  esac
done
