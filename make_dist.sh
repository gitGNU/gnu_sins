#!/bin/sh

#
# USAGE:
#
# -- sins-$version.tar.gz
# sh make_dist.sh version
#
# Expects a tag to exist with format ver_0_23
# for a version 0.23
#

app=sins
tag=sins
version=cvs

if [ -n "$1" ]; then
        version="$1"
        tag="ver_"`echo "$version" | tr '.' '_'`
fi

outdir="$app-$version"
package="$app-$version.tar.gz"

if [ -d "$outdir" ]; then
        echo "Output directory $outdir already exist"
        exit 1
fi

echo "Exporting tag $tag"
cvs export -r "$tag" -d "$outdir" $app
if [ $? -gt 0 ]; then
        exit 1
fi

# remove .cvsignore, make_dist.sh and HOWTO_RELEASE
echo "Removing .cvsignore and make_dist.sh files"
find "$outdir" -name .cvsignore -exec rm {} \;
rm -f "$outdir"/make_dist.sh "$outdir"/HOWTO_RELEASE

# generating configure script
echo "Running autogen.sh; ./configure"
owd="$PWD"
cd "${outdir}/src"
./autogen.sh
./configure
cd "$owd"

# Run make distclean
echo "Running make distclean"
owd="$PWD"
cd "$outdir"
make distclean
cd "$owd"

echo "Generating $package file"
tar czf "$package" "$outdir"



