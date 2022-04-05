apt --yes update && apt --yes upgrade
apt install --yes sudo
LOCAL_PATH=/local/scratch/Figaro/local
mkdir -p  {$LOCAL_PATH,$LOCAL_PATH/include,$LOCAL_PATH/bin,$LOCAL_PATH/lib}
apt install --yes vim
apt install --yes cmake
sudo apt install --yes build-essential wget m4 flex bison
cd $LOCAL_PATH
wget https://ftpmirror.gnu.org/gcc/gcc-10.1.0/gcc-10.1.0.tar.xz
tar xf gcc-10.1.0.tar.xz
cd gcc-10.1.0
contrib/download_prerequisites
mkdir build && cd build
../gcc-10.1.0/configure -v --build=x86_64-linux-gnu --host=x86_64-linux-gnu --target=x86_64-linux-gnu --prefix=$LOCAL_PATH/gcc-10.1 --enable-checking=release --enable-languages=c,c++,fortran --disable-multilib --program-suffix=-10.1
make -j48
make install-strip
PATH_UPDATE="export PATH=/local/scratch/local/bin:\$PATH
export PATH=/local/scratch/local/gcc-10.1/bin:\$PATH
export CPLUS_INCLUDE_PATH=/local/scratch/local/include
export LIBRARY_PATH=/local/scratch/local/gcc-10.1.0/lib64
export LIBRARY_PATH=/local/scratch/local/lib:\$LIBRARY_PATH"
echo $PATH_UPDATE>>~/.bashrc
apt install --yes postgresql postgresql-contrib systemctl
systemctl start postgresql.service
sudo -u postgres psql -c "CREATE USER zivanovic WITH ENCRYPTED PASSWORD '12345' CREATEDB;
#AlTER USer zivanovic SUPERUSER;"
sudo -u postgres psql -c "ALTER SYSTEM SET autovacuum=off"
sudo -u postgres psql -c "ALTER SYSTEM SET fsync=off"
sudo -u postgres psql -c "ALTER SYSTEM SET Synchronous_commit=off"
sudo -u postgres psql -c "ALTER SYSTEM SET Full_page_writes=off"
sudo -u postgres psql -c "ALTER SYSTEM SET Bgwriter_lru_maxpages=0"
sudo -u postgres psql -c "ALTER SYSTEM SET work_mem='64GB'"
systemctl restart postgresql.service
apt install --yes python
apt install --yes git unzip
# C++ libraries
apt install --yes libboost-all-dev libeigen3-dev libtbb-dev libgtest-dev
mkdir -p $LOCAL_PATH/include/nlohmann
wget https://github.com/nlohmann/json/blob/develop/include/nlohmann/json.hpp -P $LOCAL_PATH/include/nlohmann
mkdir -p downloads
cd downloads
wget https://sourceforge.net/projects/openblas/files/v0.3.13/OpenBLAS%200.3.13%20version.zip/download
unzip download
cd xianyi-OpenBLAS-d2b11c4
make -j48
make PREFIX=$LOCAL_PATH install
