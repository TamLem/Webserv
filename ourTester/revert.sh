#! /bin/bash

echo "Applying standard permissions..."

#dirpermissions

chmod  0755 ./ourTesterRoot/original/nordir
chmod  0755 ./ourTesterRoot/original/nowdir
chmod  0755 ./ourTesterRoot/original/noxdir
chmod  0755 ./ourTesterRoot/index/no/autoindex/nopermission

# chmod 0755 ./ourTesterRoot/original/dir

#filepermissions

chmod  0644 ./ourTesterRoot/original/dir/norfile
chmod  0644 ./ourTesterRoot/original/dir/norfile.cgi
chmod  0644 ./ourTesterRoot/original/dir/norfile.ext
chmod  0644 ./ourTesterRoot/original/dir/nowfile
chmod  0644 ./ourTesterRoot/original/dir/nowfile.cgi
chmod  0644 ./ourTesterRoot/original/dir/nowfile.ext
chmod  0644 ./ourTesterRoot/original/dir/noxfile.cgi
chmod  0644 ./ourTesterRoot/server1/myfakecgi

# chmod  0644 ./ourTesterRoot/original/dir/file