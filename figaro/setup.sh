function init_global_paths()
{
    FIGARO_ROOT_PATH="${1:-/home/popina/Figaro/figaro-code/figaro}"
    FIGARO_LOG_PATH="$FIGARO_ROOT_PATH/log"
    FIGARO_DUMP_PATH="$FIGARO_ROOT_PATH/dump"
    FIGARO_BUILD_PATH="$FIGARO_ROOT_PATH/build"
}

function get_str_args()
{
    local EXTENSION=""
    for option in "$@"
    do
    case $option in
        -r=*|--root_path=*)
            EXTENSION="${option#*=}"
            init_global_paths $EXTENSION
        ;;
        
        -l=*|--log_path=*)
            EXTENSION="${option#*=}"
            FIGARO_LOG_PATH="${EXTENSION}"
            echo
        ;;
        -t=*|--test_type=*)
            EXTENSION="${option#*=}"
            FIGARO_LOG_PATH=$EXTENSION
        ;;
        *)    # unknown option
        echo "${option}"
        echo "Wrong  argument" $option
        ;;
    esac
    done
}

export CC=/usr/bin/gcc-10
export CXX=/usr/bin/g++-10

init_global_paths
get_str_args "$@"

echo "${FIGARO_LOG_PATH}"
echo "$@"
cd "${FIGARO_BUILD_PATH}"
#cmake ../. -D FIGARO_RUN=ON 
# Used for generation of tests and libs. 
cmake ../. -D FIGARO_TEST=ON
#cmake ../. -D FIGARO_RUN=ON -D FIGARO_TEST=ON -D FIGARO_LIB=ON
make -j8
#./figaro
./figaro_test --gtest_filter=*ComputeSimpleHeadByOneAttrName > "${FIGARO_LOG_PATH}/log.txt" 2>&1
#valgrind --leak-check=yes ./figaro_test --gtest_filter=*ComputeSimpleHeadByOneAttrName > ../log/log.txt 2>&1