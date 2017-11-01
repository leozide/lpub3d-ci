# If there is no version tag in git this one will be used
VERSION = 1.0.0

#-----deprecated---------->
# Past versions available for automatic install (static list)
# PAST_RELEASES = 1.3.5,1.2.3,1.0.0
# Previous tags to capture (array)
# _TAGS = $$list(1 2)
#<-----deprecated----------

# Need to discard STDERR so get path to NULL device
win32 {
    NULL_DEVICE = NUL # Windows doesn't have /dev/null but has NUL
} else {
    NULL_DEVICE = /dev/null
}

GIT_DIR = undefined
# Default location of Git directory
exists($$PWD/.git) {
    GIT_DIR = $$PWD/.git
    message("~~~ GIT_DIR [DEFAULT] $$GIT_DIR ~~~")
}
#-----deprecated---------->
## Location of Git directory when building pkg package
#exists($$PWD/../upstream/lpub3d/.git) {
#    GIT_DIR = $$PWD/../upstream/lpub3d/.git
#    message(~~~ GIT_DIR [PKG] $$GIT_DIR ~~~)
#}
## Location of Git directory when building rpm package
#exists($$PWD/../../SOURCES/lpub3d/.git) {
#    GIT_DIR = $$PWD/../../SOURCES/lpub3d/.git
#    message(~~~ GIT_DIR [RPM] $$GIT_DIR ~~~)
#}
## Location of Git directory when building deb package
#exists($$PWD/../../upstream/lpub3d/.git) {
#    GIT_DIR = $$PWD/../../upstream/lpub3d/.git
#    message(~~~ GIT_DIR [DEB] $$GIT_DIR ~~~)
#}
#<-----deprecated----------

# AppVeyor 64bit Qt MinGW build has git.exe/cygwin conflict returning no .git directory found so use version.info file
appveyor_qt_mingw64: GIT_DIR = undefined
equals(GIT_DIR, undefined) {
    appveyor_qt_mingw64 {
        BUILD_TYPE = release
        CONFIG(debug, debug|release): BUILD_TYPE = debug
        message("~~~ GIT_DIR [APPVEYOR, USING VERSION_INFO FILE] $$GIT_VER_FILE ~~~")
        # Trying to get version from git tag / revision
        RET = $$system($$PWD/builds/utilities/update-config-files.bat $$_PRO_FILE_PWD_ $$basename(PARENT_FOLDER))
    } else {
        message("~~~ GIT_DIR [UNDEFINED, USING VERSION_INFO FILE] $$GIT_VER_FILE ~~~")
    }
    GIT_VER_FILE = $$PWD/builds/utilities/version.info
    exists($$GIT_VER_FILE) {
        GIT_VERSION = $$cat($$GIT_VER_FILE, lines)
    } else {
        message("~~~ ERROR! GIT_DIR $$GIT_VER_FILE NOT FOUND ~~~")
        GIT_VERSION = $${VERSION}-00-00000000-000
    }
    # Token position       0 1 2  3  4   5
    # Version string       2 0 20 17 663 410fdd7
    GIT_VERSION ~= s/\\\"/""
    #message(~~~ DEBUG ~~ GIT_VERSION [FROM FILE RAW]: $$GIT_VERSION)

    # Separate the build number into major, minor and service pack etc.
    VER_MAJOR = $$section(GIT_VERSION, " ", 0, 0)
    VER_MINOR = $$section(GIT_VERSION, " ", 1, 1)
    VER_PATCH = $$section(GIT_VERSION, " ", 2, 2)
    VER_REVISION_STR = $$section(GIT_VERSION, " ", 3, 3)
    VER_BUILD_STR = $$section(GIT_VERSION, " ", 4, 4)
    VER_SHA_HASH_STR = $$section(GIT_VERSION, " ", 5, 5)

    #-----deprecated---------->
    #  Get previous versions from git tag / revision
    # VER_GIT_TAG = $$section(GIT_VERSION, " ", 0, 2)

    # AVAILABLE_VERSIONS = $$VER_GIT_TAG,
    # AVAILABLE_VERSIONS ~= s/" "/"."

    # isEmpty(AVAILABLE_VERSIONS) {
    #      AVAILABLE_VERSIONS = $${VERSION},
    # }
    #message(~~~ DEBUG ~~ AVAILABLE_VERSIONS [FROM FILE RAW]: $$AVAILABLE_VERSIONS)
    #<-----deprecated----------

} else {
    # Need to call git with manually specified paths to repository
    BASE_GIT_COMMAND = git --git-dir $$shell_quote$$GIT_DIR --work-tree $$shell_quote$$PWD

    # Trying to get version from git tag / revision
    GIT_VERSION = $$system($$BASE_GIT_COMMAND describe --long 2> $$NULL_DEVICE)

    # Check if we only have hash without version number (i.e. not version tag found)
    !contains(GIT_VERSION,\d+\.\d+\.\d+) {
        # If there is nothing we simply use version defined manually
        isEmpty(GIT_VERSION) {
            GIT_VERSION = $${VERSION}-00-00000000-000
            message("~~~ ERROR! GIT_VERSION NOT DEFINED, USING $$GIT_VERSION ~~~")
        } else { # otherwise construct proper git describe string
            GIT_COMMIT_COUNT = $$system($$BASE_GIT_COMMAND rev-list HEAD --count 2> $$NULL_DEVICE)
            isEmpty(GIT_COMMIT_COUNT) {
                GIT_COMMIT_COUNT = 0
                message("~~~ ERROR! GIT_COMMIT_COUNT NOT DEFINED, USING $$GIT_COMMIT_COUNT ~~~")
            }
            GIT_VERSION = g$$GIT_VERSION-$$GIT_COMMIT_COUNT
        }
    }
    #message(~~~ DEBUG ~~ GIT_VERSION [RAW]: $$GIT_VERSION)

    # Convert output from gv2.0.20-37-ge99beed-600 into "gv2.0.20.37.ge99beed.600"
    GIT_VERSION ~= s/-/"."
    GIT_VERSION ~= s/g/""
    GIT_VERSION ~= s/v/""
    #message(~~~ DEBUG ~~ GIT_VERSION [FORMATTED]: $$GIT_VERSION)

    # Separate the build number into major, minor and service pack etc.
    VER_MAJOR = $$section(GIT_VERSION, ., 0, 0)
    VER_MINOR = $$section(GIT_VERSION, ., 1, 1)
    VER_PATCH = $$section(GIT_VERSION, ., 2, 2)
    VER_REVISION_STR = $$section(GIT_VERSION, ., 3, 3)
    VER_SHA_HASH_STR = $$section(GIT_VERSION, ., 4, 4)
    VER_BUILD_STR = $$section(GIT_VERSION, ., 5, 5)

    #-----deprecated---------->
    # Get previous versions from git tag / revision
    # VER_GIT_TAG = $$system($$BASE_GIT_COMMAND describe --abbrev=0 2> $$NULL_DEVICE)
    # for(_TAG, $$_TAGS) {
    #     greaterThan(_TAG, 1) {
    #         win32:VER_GIT_TAG_KEY = $$join(VER_GIT_TAG,,,^^)
    #         else: VER_GIT_TAG_KEY = $$join(VER_GIT_TAG,,,^)
    #     }
    #     VER_GIT_TAG = $$system($$BASE_GIT_COMMAND describe --abbrev=0 $$VER_GIT_TAG_KEY 2> $$NULL_DEVICE)
    #     !isEmpty(VER_GIT_TAG) {
    #         AVAILABLE_VERSIONS += $$VER_GIT_TAG,
    #         AVAILABLE_VERSIONS ~= s/v/""
    #     }
    # }
    # isEmpty(AVAILABLE_VERSIONS) {
    #      AVAILABLE_VERSIONS = $${VERSION},
    # }
    #message(~~~ DEBUG ~~ AVAILABLE_VERSIONS [FORMATTED]: $$AVAILABLE_VERSIONS)
    #<-----deprecated----------
}

# Here we process the build date and time
win32 {
    BUILD_DATE = $$system( date /t )
    BUILD_TIME = $$system( echo %time% )
} else {
    BUILD_DATE = $$system( date "+%d/%m/%Y/%H:%M:%S" )
    BUILD_TIME = $$section(BUILD_DATE, /, 3, 3)
}
#message(~~~ DEBUG ~~ BUILD_DATE: $$BUILD_DATE) # output the current date
#message(~~~ DEBUG ~~ BUILD_TIME: $$BUILD_TIME) # output the current time

# Separate the date into day month, year.
appveyor_ci {
    # AppVeyor CI uses date format 'Day MM/DD/YY'
    BUILD_DATE ~= s/[\sA-Za-z\s]/""
    DATE_MM = $$section(BUILD_DATE, /, 0, 0)
    DATE_DD = $$section(BUILD_DATE, /, 1, 1)
    DATE_YY = $$section(BUILD_DATE, /, 2, 2)
} else {
    DATE_DD = $$section(BUILD_DATE, /, 0, 0)
    DATE_MM = $$section(BUILD_DATE, /, 1, 1)
    DATE_YY = $$section(BUILD_DATE, /, 2, 2)
}

# C preprocessor #DEFINE to use in C++ code
DEFINES += VER_MAJOR=\\\"$$VER_MAJOR\\\"
DEFINES += VER_MINOR=\\\"$$VER_MINOR\\\"
DEFINES += VER_PATCH=\\\"$$VER_PATCH\\\"

DEFINES += BUILD_TIME=\\\"$$BUILD_TIME\\\"
DEFINES += DATE_YY=\\\"$$DATE_YY\\\"
DEFINES += DATE_MM=\\\"$$DATE_MM\\\"
DEFINES += DATE_DD=\\\"$$DATE_DD\\\"

DEFINES += VER_BUILD_STR=\\\"$$VER_BUILD_STR\\\"
DEFINES += VER_SHA_HASH_STR=\\\"$$VER_SHA_HASH_STR\\\"
DEFINES += VER_REVISION_STR=\\\"$$VER_REVISION_STR\\\"

# Now we are ready to pass parsed version to Qt ===
VERSION = $$VER_MAJOR"."$$VER_MINOR"."$$VER_PATCH

#-----deprecated---------->
# Append static past versions
# AVAILABLE_VERSIONS = $$join(AVAILABLE_VERSIONS,,,$$PAST_RELEASES)
#<-----deprecated----------

# Update the version number file for win/unix during build
# Generate git version data to the input files indicated. Input files are consumed during the
# build process to set the version informatio for LPub3D executable, its libraries (ldrawini and quazip)
# Update the application version in lpub3d.desktop (desktop configuration file), lpub3d.1 (man page)
# This flag will also add the version number to packaging configuration files PKGBUILD, changelog and
# lpub3d.spec depending on which build is being performed.
message(~~~ VERSION_INFO: $$VER_MAJOR $$VER_MINOR $$VER_PATCH $$VER_REVISION_STR $$VER_BUILD_STR $$VER_SHA_HASH_STR ~~~)

#-----deprecated---------->
# message(~~~ AVAILABLE_VERSIONS: $$AVAILABLE_VERSIONS ~~~)


# COMPLETION_COMMAND = LPub3D Build Finished.
# win32 {
#     # Update config parameters
#     # 1. Present working directory [_PRO_FILE_PWD_]
#     # 2. Major version             [VER_MAJOR]
#     # 3. Minor version             [VER_MINOR]
#     # 4. Patch version             [VER_PATCH]
#     # 5. Revision                  [VER_REVISION_STR]
#     # 6. Build number              [VER_BUILD_STR]
#     # 7. Git sha hash (short)      [VER_SHA_HASH_STR]
#     # 8. Available past versions   [AVAILABLE_VERSIONS]
#     CONFIG_FILES_COMMAND = $$PWD/builds/utilities/update-config-files.bat
#     QMAKE_POST_LINK += $$escape_expand(\n\t)  \
#                        $$shell_quote$${CONFIG_FILES_COMMAND} \
#                        $$shell_quote$$_PRO_FILE_PWD_ \
#                        $${VER_MAJOR} \
#                        $${VER_MINOR} \
#                        $${VER_PATCH} \
#                        $${VER_REVISION_STR} \
#                        $${VER_BUILD_STR} \
#                        $${VER_SHA_HASH_STR} \
#                        $${AVAILABLE_VERSIONS} \
#                        $$escape_expand(\n\t) \
#                        echo $$shell_quote$${COMPLETION_COMMAND}

# } else {
#     CONFIG_FILES_TARGET = $$PWD/builds/utilities/update-config-files.sh
#     CONFIG_FILES_COMMAND = $$CONFIG_FILES_TARGET
#     CHMOD_COMMAND = chmod 755 $$CONFIG_FILES_TARGET
#     QMAKE_POST_LINK += $$escape_expand(\n\t)  \
#                        $$shell_quote$${CHMOD_COMMAND} \
#                        $$escape_expand(\n\t)  \
#                        $$shell_quote$${CONFIG_FILES_COMMAND} \
#                        $$shell_quote$$_PRO_FILE_PWD_ \
#                        $${VER_MAJOR} \
#                        $${VER_MINOR} \
#                        $${VER_PATCH} \
#                        $${VER_REVISION_STR} \
#                        $${VER_BUILD_STR} \
#                        $${VER_SHA_HASH_STR} \
#                        $$escape_expand(\n\t)  \
#                        echo $$shell_quote$${COMPLETION_COMMAND}
#     #<-----deprecated-------------
#     #
#     #-----keep (for now)---------->
#     # On Mac update the Info.plist with version major, version minor, build and add git hash
#     macx {
#         INFO_PLIST_FILE = $$shell_quote($${PWD}/mainApp/Info.plist)
#         PLIST_COMMAND = /usr/libexec/PlistBuddy -c
#         QMAKE_POST_LINK += $$escape_expand(\n\t)   \
#                            $$PLIST_COMMAND \"Set :CFBundleShortVersionString $${VERSION}\" $${INFO_PLIST_FILE}  \
#                            $$escape_expand(\n\t)   \
#                            $$PLIST_COMMAND \"Set :CFBundleVersion $${VER_BUILD_STR}\" $${INFO_PLIST_FILE} \
#                            $$escape_expand(\n\t)   \
#                            $$PLIST_COMMAND \"Set :CFBundleGetInfoString LPub3D $${VERSION} https://github.com/trevorsandy/lpub3d\" $${INFO_PLIST_FILE} \
#                            $$escape_expand(\n\t)   \
#                            $$PLIST_COMMAND \"Set :com.trevorsandy.lpub3d.GitSHA $${VER_SHA_HASH_STR}\" $${INFO_PLIST_FILE}
#      #<-----keep (for now)----------
#     }
# }
