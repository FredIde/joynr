#!/bin/bash

source /data/src/docker/joynr-base/scripts/global.sh

DBUS='OFF'
GCOV='OFF'
ENABLE_CLANG_FORMATTER='ON'
BUILD_TESTS='ON'
ADDITIONAL_CMAKE_ARGS=''

function usage
{
    echo "usage: cpp-clean-build.sh [--dbus ON|OFF --gcov ON|OFF --jobs X \
    --enableclangformatter ON|OFF --buildtests ON|OFF --additionalcmakeargs <args>]"
    echo "default dbus is $DBUS, gcov is $GCOV, jobs is $JOBS, additionalcmakeargs is \
    $ADDITIONAL_CMAKE_ARGS"
}

while [ "$1" != "" ]; do
    case $1 in
        --dbus )                 shift
                                 DBUS=$1
                                 ;;
        --gcov )                 shift
                                 GCOV=$1
                                 ;;
        --jobs )                 shift
                                 JOBS=$1
                                 ;;
        --enableclangformatter ) shift
                                 ENABLE_CLANG_FORMATTER=$1
                                 ;;
        --buildtests )           shift
                                 BUILD_TESTS=$1
                                 ;;
        --additionalcmakeargs )  shift
                                 ADDITIONAL_CMAKE_ARGS=$1
                                 ;;
        * )                      usage
                                 exit 1
    esac
    shift
done

log "CPP CLEAN BUILD DBUS: $DBUS GCOV: $GCOV JOBS: $JOBS"

log "Enable core dumps"
ulimit -c unlimited

START=$(date +%s)


log "ENVIRONMENT"
env
echo "ADDITIONAL_CMAKE_ARGS is $ADDITIONAL_CMAKE_ARGS"

log "CLEAN BUILD DIRECTORY"
rm -rf ~/.cmake/packages
rm -rf /data/build/joynr
mkdir /data/build/joynr

cd /data/build/joynr

log "RUN CMAKE"

# fail on first error
set -e -x
cmake -DUSE_DBUS_COMMONAPI_COMMUNICATION=$DBUS \
      -DENABLE_GCOV=$GCOV \
      -DPYTHON_EXECUTABLE=/usr/bin/python \
      -DJOYNR_SERVER=localhost:8080 \
      -DCMAKE_BUILD_TYPE=Debug \
      -DENABLE_CLANG_FORMATTER=$ENABLE_CLANG_FORMATTER \
      -DBUILD_TESTS=$BUILD_TESTS \
      -DCMAKE_INSTALL_SYSCONFDIR=/etc \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -DJOYNR_ENABLE_ACCESS_CONTROL:BOOL=ON \
      $ADDITIONAL_CMAKE_ARGS \
      /data/src/cpp

if [ "$GCOV" == "ON" ] ; then
    echo "run coverage build"
    make -j $JOBS UnitCoverageTarget
fi

log "BUILD C++ JOYNR"
make -j $JOBS
log "BUILD C++ JOYNR DOXYGEN DOCUMENTATION"
log "doxygen is disabled"
#make doxygen

tar czf joynr-clean-build.tar.gz bin

END=$(date +%s)
DIFF=$(( $END - $START ))
log "C++ build time: $DIFF seconds"
