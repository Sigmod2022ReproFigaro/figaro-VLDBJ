########### Clonning scripts ###########
git clone --branch Sigmod-2022-Repro https://gitlab.ifi.uzh.ch/dast/papers/figaro-code.git
mv figaro-code/reproducibility/*.sh .
find . -iname \*.sh -print0 | xargs -r0 chmod 777
####### Old container removal
DOCK_OLD_CID=`docker ps -a | tail -1 | awk '{print $NF}'`
docker stop $DOCK_OLD_CID && docker rm $DOCK_OLD_CID

############### New cointainer creation ###########
docker run -td ubuntu:20.04
DOCK_CID=`docker ps -a | tail -1 | awk '{print $NF}'`
HOME_PATH=/home/zivanovic
echo $DOCK_CID
pwd
docker cp user_install.sh $DOCK_CID:/user_install.sh
docker exec --user root -ti $DOCK_CID bash user_install.sh | tee log_user_install.txt

docker cp library_setup.sh $DOCK_CID:/$HOME_PATH/library_setup.sh
docker cp run_experiments.sh $DOCK_CID:/$HOME_PATH/run_experiments.sh

docker exec --user zivanovic -ti $DOCK_CID bash $HOME_PATH/library_setup.sh | tee log_lib_setup.txt
docker exec --user zivanovic -ti $DOCK_CID bash $HOME_PATH/run_experiments.sh | tee log_run_exp.txt
