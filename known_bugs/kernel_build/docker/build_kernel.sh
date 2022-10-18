#!/bin/bash

show_help() {
cat << EOF
Usage: ${0##*/} [-k KERNEL_PATH] [-c COMMIT_ID] [-p PATCH] [-x COMPILER] [-f CONFIG] [-d]
EOF
}

config=""
kernel=""
compiler=""
patch=""
commit=""
direct=""

while getopts "df:k:c:p:x:" opt; do
        case $opt in
                d)
                        direct="1"
                        ;;
                f)
                        config=($OPTARG)
                        ;;
                k)      
                        kernel=($OPTARG)
                        ;;
                c)  
                        commit=($OPTARG)
                        ;;
                p)
                        patch=($OPTARG)
                        ;;
                x)
                        compiler=($OPTARG)
                        ;;
                *)
                        show_help >&2
                        exit 1
                        ;;
    esac
done
shift $((OPTIND-1))

if [ -z "$kernel" ] ; then
        echo "No kernel path provided."
        exit 1
fi

if [ -z "$compiler" ] ; then
        echo "No compiler provided."
        exit 1
fi

if [ ! -z "$direct" ]; then
        echo "Direct build."
        echo "Entering the kernel directory at $kernel and cleaning..."
        cd $kernel
        echo "Building..."
        make CC=$compiler olddefconfig
        make CC=$compiler -j`nproc`
        exit 0
fi

if [ -z "$commit" ] ; then
        echo "No commit ID provided."
        exit 1
fi

echo "Entering the kernel directory at $kernel and cleaning..."
cd $kernel

echo "Cleaning the directory..."
make distclean && git reset && git checkout . && git clean -fdx

echo "Switching to the commit $commit..."
git checkout $commit

if [ -z "$patch" ] ; then
        echo "No patch provided."
else

        echo "Applying patch..."
        git apply $patch
fi

echo "Copying the config file $config..."
cp $config ./

echo "Building..."
if [ -z "$config" ] ; then
        echo "No kernel config provided. Use the default config."
        make CC=$compiler defconfig
else
        make CC=$compiler olddefconfig
fi
make CC=$compiler -j`nproc`