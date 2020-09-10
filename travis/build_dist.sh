#!/usr/bin/bash
# build and repair wheels. this should be run on manylinux1 docker image. note
# that PLAT and DOCKER_MNT should be defined in .travis.yml and passed to the
# docker image this script is running on through upload_dist.sh.

# exit if command has nonzero exit status
set -e

# subroutine to build cpython 3 wheels
build_cp3_wheels() {
    # on the manylinux1 images, python installed in /opt/python/*/bin. see
    # https://github.com/pypa/manylinux README.md for more details.
    for PY_BIN in /opt/python/cp3*/bin
    do
        # build wheel for this python version. first install dependencies from
        # travis/requirements.txt, then run sdist bdist_wheel. this is because
        # we mounted repository home to DOCKER_MNT.
        $PY_BIN/pip install -f $DOCKER_MNT/travis/requirements.txt
        #"${PY_BIN}/python" $DOCKER_MNT/setup.py sdist bdist_wheel
        make dist
    done
}

# subroutine to repair wheels using auditwheel (installed on manylinux1 images)
repair_cp3_wheels() {
    # take first argument as name of distribution directory
    DIST_DIR=$1
    # for each of the wheels in DIST_DIR, repair
    for PY_WHL in $DIST_DIR/*.whl
    do
        # if auditwheel doesn't return 0, then skip (non-platform wheel)
        if ! auditwheel show $PY_WHL
        then
            echo "skipping non-platform wheel $PY_WHL"
        # else repair wheel with auditwheel
        else
            auditwheel repair $PY_WHL --plat $PLAT -w $DIST_DIR
        fi
    done
}


# exit if we don't have PLAT or DOCKER_MNT defined
if ! [ $PLAT ]
then
    echo "variable PLAT not defined; exiting"
elif ! [ $DOCKER_MNT ]
then
    echo "variable DOCKER_MNT not defined; exiting"
# also exit if we are on travis
elif [ $TRAVIS ]
    echo "should not be run on travis but on manylinux docker image. exiting"
# else run the wheel building and install process on manylinux1 docker image
else
    # build wheels using Makefile
    build_cp3_wheels
    # repair wheels using auditwheel in manylinux1 image
    repair_cp3_wheels $DOCKER_MNT/dist
    # for each python version, install from wheel and run benchmarks
    for $PY_BIN in /opt/python/cp3*/bin
    do
        $PY_BIN/pip install c_npy_demo --no-index --only-binary :all: \
            -f $DOCKER_MNT/dist
        cd $HOME
        # run extension and vol benchmarks, verbosely (with defaults)
        c_npy_demo.bench.ext -v
        c_npy_demo.bench.vol -v
    done
fi