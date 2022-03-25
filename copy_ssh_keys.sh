hosts=("pc201.emulab.net"
       "pc202.emulab.net"
       "pc203.emulab.net"
       "pc204.emulab.net")

rm all-keys.txt
for host in "${hosts[@]}"
do
#  ssh -o StrictHostKeyChecking=no -p 22 -l kkhare ${host} 'cat /dev/zero | ssh-keygen -q -N ""'
  scp -o StrictHostKeyChecking=no kkhare@${host}:~/.ssh/id_rsa.pub .
  cat id_rsa.pub >> all-keys.txt
done

for host in "${hosts[@]}"
do
  scp -o StrictHostKeyChecking=no  all-keys.txt kkhare@${host}:/tmp/.
  ssh -o StrictHostKeyChecking=no -p 22 -l kkhare ${host} "cat /tmp/all-keys.txt >> ~/.ssh/authorized_keys"
done

#for host in "${hosts[@]}"
#do
#  scp -p 22 -o StrictHostKeyChecking=no  all-keys.txt kkhare@${host}:/tmp/.
#  ssh -o StrictHostKeyChecking=no -p 22 -l kkhare ${host} "cat /tmp/all-keys.txt >> ~/.ssh/authorized_keys"
#done
