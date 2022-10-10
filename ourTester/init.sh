#! /bin/bash

echo "Applying ourTester permissions..."

#create dir to delete later
mkdir ./ourTesterRoot/uploads/newdir/

#create files to delete later
touch ./ourTesterRoot/uploads/nopermissionfile.cgi
echo "you should never read this!" >> ./ourTesterRoot/uploads/nopermissionfile.cgi
chmod 0000 ./ourTesterRoot/uploads/nopermissionfile.cgi

touch ./ourTesterRoot/uploads/todelete.cgi
echo "this file should not exist after running ourTester" >> ./ourTesterRoot/uploads/todelete.cgi

#create file inside the directory you have no permission to
touch ./ourTesterRoot/uploads/cgi/nopermissiondir.cgi


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

chmod  0000 ./ourTesterRoot/uploads/cgi/
chmod  0300 ./ourTesterRoot/original/nordir
chmod  0500 ./ourTesterRoot/original/nowdir
chmod  0600 ./ourTesterRoot/original/noxdir
chmod  0300 ./ourTesterRoot/index/no/autoindex/nopermission

# chmod 0755 ./ourTesterRoot/original/dir