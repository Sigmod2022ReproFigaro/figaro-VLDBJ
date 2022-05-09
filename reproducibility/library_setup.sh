LOCAL_PATH=/home/zivanovic/local
DOWN_PATH=/home/zivanovic/downloads
FIGARO_PSQL_PATH=/figaroPostgresql
FIGARO_DUMP_PATH=/figaroDumps
FIGARO_DATA_PATH=/figaroData

mkdir -p  {$LOCAL_PATH,$LOCAL_PATH/include,$LOCAL_PATH/bin,$LOCAL_PATH/lib,$DOWN_PATH}
sudo apt install --yes vim
sudo apt install --yes cmake
sudo apt install --yes build-essential wget m4 flex bison git unzip rsync libomp-dev

########################### gcc-10.1 from source ################
cd $LOCAL_PATH
wget https://ftpmirror.gnu.org/gcc/gcc-10.1.0/gcc-10.1.0.tar.xz
tar xf gcc-10.1.0.tar.xz
rm gcc-10.1.0.tar.xz
cd gcc-10.1.0
contrib/download_prerequisites
cd ..
mkdir build && cd build
../gcc-10.1.0/configure -v --build=x86_64-linux-gnu --host=x86_64-linux-gnu --target=x86_64-linux-gnu --prefix=$LOCAL_PATH/gcc-10.1 --enable-checking=release --enable-languages=c,c++,fortran --disable-multilib --program-suffix=-10
make -j24
make install-strip
PATH_UPDATE="export PATH=$LOCAL_PATH/bin:\$PATH\n
export PATH=$LOCAL_PATH/gcc-10.1/bin:\$PATH\n
export CPLUS_INCLUDE_PATH=$LOCAL_PATH/include\n
export LIBRARY_PATH=$LOCAL_PATH/lib:\$LIBRARY_PATH"
echo -e $PATH_UPDATE | tee -a ~/.bashrc ~/.non_inter_paths.sh
rm -rf

##################### PSQL ###############
sudo apt install --yes postgresql postgresql-contrib systemctl
sudo chown postgres $FIGARO_PSQL_PATH
sudo chmod 700 $FIGARO_PSQL_PATH
sudo rm -rf $FIGARO_PSQL_PATH/12
sudo rsync -a /var/lib/postgresql/ $FIGARO_PSQL_PATH
sudo rm -rf /var/lib/postgresql
sudo ln -s $FIGARO_PSQL_PATH /var/lib/postgresql
sudo service postgresql start
sudo -u postgres psql -c "CREATE USER zivanovic WITH ENCRYPTED PASSWORD '12345' CREATEDB;
AlTER USer zivanovic SUPERUSER;"
sudo -u postgres psql -c "ALTER SYSTEM SET autovacuum=off"
sudo -u postgres psql -c "ALTER SYSTEM SET fsync=off"
sudo -u postgres psql -c "ALTER SYSTEM SET Synchronous_commit=off"
sudo -u postgres psql -c "ALTER SYSTEM SET Full_page_writes=off"
sudo -u postgres psql -c "ALTER SYSTEM SET Bgwriter_lru_maxpages=0"
sudo -u postgres psql -c "ALTER SYSTEM SET work_mem='64GB'"
sudo service postgresql restart

############# Python ####################
cd $DOWN_PATH
sudo apt-get install python3-dev python3-pip python3-venv python3-wheel -y
pip3 install wheel
pip3 install cython

############# C++ libraries ####################
sudo apt install --yes libboost-all-dev libeigen3-dev libtbb-dev libgtest-dev
mkdir -p $LOCAL_PATH/include/nlohmann
cd $DOWN_PATH
git clone --depth 1 https://github.com/nlohmann/json/
cp json/single_include/nlohmann/json.hpp $LOCAL_PATH/include/nlohmann/json.hpp
rm json -rf

############### OpenBlas ##################
cd $DOWN_PATH
wget https://sourceforge.net/projects/openblas/files/v0.3.13/OpenBLAS%200.3.13%20version.zip/download
unzip download
rm download
cd xianyi-OpenBLAS-d2b11c4
make -j48
make PREFIX=$LOCAL_PATH install
rm xianyi-OpenBLAS-d2b11c4 -rf

################ Intel MKL ######################
wget https://registrationcenter-download.intel.com/akdlm/irc_nas/17769/l_BaseKit_p_2021.2.0.2883_offline.sh -o oneApi.sh
MKL_PATH=$LOCAL_PATH/intel
sh l_BaseKit_p_2021.2.0.2883_offline.sh -a -s --eula accept --install-dir $MKL_PATH
rm l_BaseKit_p_2021.2.0.2883_offline.sh
PATH_UPDATE="export LIBRARY_PATH=$MKL_PATH/mkl/2021.2.0/lib/intel64/:\$LIBRARY_PATH\n
export LD_LIBRARY_PATH=$MKL_PATH/mkl/2021.2.0/lib/intel64/\n
export CPLUS_INCLUDE_PATH=$MKL_PATH/mkl/2021.2.0/include/:\$CPLUS_INCLUDE_PATH\n
source $MKL_PATH/vtune/2021.2.0/vtune-vars.sh\n
export LIBRARY_PATH=$MKL_PATH/tbb/2021.2.0/lib/intel64/gcc4.8:\$LIBRARY_PATH\n"
echo -e $PATH_UPDATE | tee -a ~/.bashrc ~/.non_inter_paths.sh
source ~/.non_inter_paths.sh

################### Numpy from source ##########
cd ~
git clone --depth 1 https://github.com/numpy/numpy numpy-openblas --branch v1.22.0
git clone --depth 1 https://github.com/numpy/numpy numpy-mkl --branch v1.22.0
Config_OpenBlas="
[openblas]\n
libraries = openblas\n
library_dirs = $LOCAL_PATH/lib\n
include_dirs = $LOCAL_PATH/include\n
runtime_library_dirs = $LOCAL_PATH/lib\n
"
Config_MKL="
[mkl]\n
libraries = mkl_rt\n
library_dirs = $MKL_PATH/mkl/2021.2.0/lib/intel64\n
include_dirs = $MKL_PATH/mkl/2021.2.0/include\n
"
mv /home/zivanovic/numpy-mkl/site.cfg.example numpy-mkl/site.cfg
mv /home/zivanovic/numpy-openblas/site.cfg.example numpy-openblas/site.cfg
echo -e $Config_MKL>>numpy-mkl/site.cfg
echo -e $Config_OpenBlas>>numpy-openblas/site.cfg

cd numpy-mkl
git submodule update --init
cd ../numpy-openblas
git submodule update --init
cd ~
python3.8 /home/zivanovic/numpy-mkl/setup.py build
python3.8 /home/zivanovic/numpy-openblas/setup.py build

sudo apt install --yes gnuplot