#! /bin/bash

declare -r prog=$(basename $0 .sh)
declare -r progpid=$$
declare -r progdir=$(cd $(dirname $0) >/dev/null && pwd -P)

die() {
    echo "$prog fatal error:  $*"
    kill -TERM $progpid
    sleep 1
    exec 1>/dev/null 2>&1
    kill -KILL $progpid
    exit 1
} >&2

warn() {
    echo $'\n'"$prog warning:  mdsim $(basename $1) failed"
    cat $2
    printf '\n\n\n\n'
} >&2

usage() {
    cat <<- _EOF_
	USAGE:  $prog [ all | mdsim-test1 ... ]
	The specified tests may be either files that you name directly, or
	one of the ".conf" files installed in $progdir:

	$(cd $progdir >/dev/null ; ls -C *.conf | sed $'s/\t/ /g;s/^/  /')

	By default, test.conf and test1.conf are run.
	Specifying "all" will run all the tests.
	_EOF_

    exit 0
}

init() {
    # Construct the list of tests
    #
    if test $# -eq 0
    then
        testlist="$progdir/test.conf $progdir/test1.conf"

    else
        test $# -gt 0 && {
            [[ $1 =~ ^--*h ]] && usage
        }
        if test $# -eq 1 -a "X$1" = Xall
        then testlist=$(echo $progdir/*.conf)
        else testlist="$*"
        fi
    fi

    # Make sure that lustre is configured (mgc directory is in /proc)
    #
    declare mgcdir=/proc/fs/lustre/mgc
    test -d $mgcdir || \
        die "Cannot locate mgcdir:  $mgcdir"
    declare mgclist=$(ls -1 $mgcdir | grep -E '^MGC.*@')
    test ${#mgclist} -gt 1 || \
        die "mgcdir is empty ($mgcdir)"

    # check for sim.ko module.  It creates /dev/c3_ksim.
    #
    declare simul_dev_name=/dev/c2_ksim
    test -c $simul_dev_name || {
        modprobe sim || die "cannot $_"
        test -c $simul_dev_name || \
            die "loading sim.ko did not create $simul_dev_name"
    }

    # We need to know the IP address of MGC.  It is reflected in a directory
    # name in /proc.
    #
    ipaddr_pat=$'[ \t][0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*@'
    mgcip=$(echo "$mgclist" | sed 's/^MGC//;s/@.*//;q')

    # Create a place to stash our temporary stuff.  Blow it away when done.
    #
    tmpdir=$(mktemp -d ${TMPDIR:-/tmp}/mdsim-XXXXXX)
    test -d ${tmpdir} || die "cannot make temp directory"
    trap "rm -rf ${tmpdir}" EXIT
}

run_test() {
    # struggle to find the test.  It may be missing a ".conf" suffix
    # or the implicit directory may be wrong.  Try to find it several ways.
    #
    declare tname=$1
    while :
    do
        test -f "${tname}" && break
        [[ "$tname" =~ .*\.conf$ ]] || tname=${tname}.conf
        test -f "${tname}" && break
        tname=$progdir/$(basename $tname)
        test -f "${tname}" && break
        die "Cannot find $1 test"
    done

    # now that we've found it, replace the IP address that it references with
    # the one we know to be correct.
    #
    declare tbase=$(basename $tname .conf)
    declare script=$tmpdir/$tbase.mdsim
    declare logf=${script%.mdsim}.log
    sed "s%${ipaddr_pat}% ${mgcip}@%g" $tname > $script

    # Try the test and report errors
    #
    if mdsim -c ${script} -l ${logf}
    then echo PASSED: $tname
    else warn $1 $logf
        echo failed: $tname
    fi
}

init ${1+"$@"}

for f in $testlist
do
    run_test $f
done
