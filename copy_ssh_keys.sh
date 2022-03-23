hosts=("pc245.emulab.net"
       "pc255.emulab.net"
       "pc254.emulab.net"
       "pc247.emulab.net"
       "pc250.emulab.net"
       "pc246.emulab.net")

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
