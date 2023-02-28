# Enter name of test file within apps folder (without .x extension)
test_name='test_preempt' 

echo "Enter CSIF username: "
read username
ssh $username@pc02.cs.ucdavis.edu "rm -rf ecs150-fs"
echo "Remnants of previous upload removed"
scp -rq ../ecs150-fs $username@pc02.cs.ucdavis.edu:~/ecs150-fs
echo "New upload complete"
ssh $username@pc02.cs.ucdavis.edu "cd ./ecs150-p2/apps ; make clean ; cd ../libfs ; make ; cd ../apps ; make $test_name.x ; ./$test_name.x" 