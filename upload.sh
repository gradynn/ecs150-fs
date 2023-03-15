# Enter name of test file within apps folder (without .x extension)
test_name='test_fs' 


echo "Enter CSIF username: "
read username
ssh $username@pc05.cs.ucdavis.edu "rm -rf ecs150-fs"
echo "Remnants of previous upload removed"
scp -rq ../ecs150-fs $username@pc05.cs.ucdavis.edu:~/ecs150-fs
echo "New upload complete"
ssh $username@pc05.cs.ucdavis.edu "cd ./ecs150-fs/apps ; make clean ; cd ../libfs ; make ; cd ../apps ; make $test_name.x "
ssh $username@pc05.cs.ucdavis.edu 