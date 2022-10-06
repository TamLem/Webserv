#! /bin/bash

echo "Applying ourTester permissions..."

#filepermissions

chmod  0200 ./ourTesterRoot/original/dir/norfile
chmod  0200 ./ourTesterRoot/original/dir/norfile.cgi
chmod  0200 ./ourTesterRoot/original/dir/norfile.ext
chmod  0400 ./ourTesterRoot/original/dir/nowfile
chmod  0400 ./ourTesterRoot/original/dir/nowfile.cgi
chmod  0400 ./ourTesterRoot/original/dir/nowfile.ext
chmod  0600 ./ourTesterRoot/original/dir/noxfile.cgi
chmod  0600 ./ourTesterRoot/server1/myfakecgi

# chmod  0644 ./ourTesterRoot/original/dir/file

#dirpermissions

chmod  0300 ./ourTesterRoot/original/nordir
chmod  0500 ./ourTesterRoot/original/nowdir
chmod  0600 ./ourTesterRoot/original/noxdir
chmod  0300 ./ourTesterRoot/index/no/autoindex/nopermission

# chmod 0755 ./ourTesterRoot/original/dir