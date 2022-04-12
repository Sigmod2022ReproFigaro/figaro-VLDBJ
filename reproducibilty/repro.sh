####### Old container removal
DOCK_OLD_CID=`docker ps -a | tail -1 | awk '{print $NF}'`
docker stop $DOCK_OLD_CID && docker rm $DOCK_OLD_CID

############### New cointainer creation ###########
docker run -td ubuntu:20.04
DOCK_CID=`docker ps -a | tail -1 | awk '{print $NF}'`
HOME_PATH=/home/zivanovic
echo $DOCK_CID
pwd
#docker cp user_create.sh $DOCK_CID:/user_create.sh
#docker exec --user root -ti $DOCK_CID bash user_create.sh | tee log.txt

docker cp library_setup.sh $DOCK_CID:/$HOME_PATH/library_setup.sh
docker cp run_experiments.sh $DOCK_CID:/$HOME_PATH/run_experiments.sh

docker exec --user zivanovic -ti $DOCK_CID bash $HOME_PATH/library_setup.sh | tee log.txt
docker exec --user zivanovic -ti $DOCK_CID bash $HOME_PATH/run_experiments.sh | tee log.txt
