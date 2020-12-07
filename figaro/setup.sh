function init_global_paths()
{
    FIGARO_ROOT_PATH="${1:-/home/popina/Figaro/figaro-code/figaro}"
    FIGARO_LOG_PATH="$FIGARO_ROOT_PATH/log"
    FIGARO_BUILD_PATH="$FIGARO_ROOT_PATH/build"
    FIGARO_DUMP_PATH="$FIGARO_ROOT_PATH/dump"
    FIGARO_TEST_MODE="DEBUG"
    FIGARO_PRECISION=14
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
        ;;
        -t=*|--test_mode=*)
            EXTENSION="${option#*=}"
            FIGARO_TEST_MODE=$EXTENSION
        ;;
        --dump_path=*)
            EXTENSION="${option#*=}"
            FIGARO_DUMP_PATH=$EXTENSION
        ;;
        --precision=*)
            EXTENSION="${option#*=}"
            FIGARO_PRECISION=$EXTENSION
        ;;
        *)    # unknown option
        echo "${option}"
        echo "Wrong  argument" $option
        ;;
    esac
    done
}

function main()
{
    export CC=/usr/bin/gcc-10
    export CXX=/usr/bin/g++-10

    init_global_paths
    get_str_args "$@"

    echo "${FIGARO_LOG_PATH}"
    echo "$@"
    cd "${FIGARO_BUILD_PATH}"
    echo "TESTMODE ${FIGARO_TEST_MODE}"
    if [[ $FIGARO_TEST_MODE == DEBUG ]]; then 
        cmake ../. -D FIGARO_RUN=ON -D FIGARO_DEBUG=ON
    else
         cmake ../. -D FIGARO_RUN=ON 
    fi
    # Used for generation of tests and libs. 
    #cmake ../. -D FIGARO_TEST=ON
    #cmake ../. -D FIGARO_RUN=ON -D FIGARO_TEST=ON -D FIGARO_LIB=ON
    make -j8
    case "${FIGARO_TEST_MODE}" in
    "DEBUG")
        ./figaro > "${FIGARO_LOG_PATH}/log.txt" 2>&1;
        ;;
    "DUMP")
        ./figaro --dump_path "${FIGARO_DUMP_PATH}" --precision "${FIGARO_PRECISION}"  > \ 
            "${FIGARO_LOG_PATH}/log.txt"  2>&1;
        ;;
    "PERFORMANCE")
        ./figaro > "${FIGARO_LOG_PATH}/log.txt" 2>&1;
        ;;
    "PRECISION")
        ;;
    esac

    #./figaro --dump_path "${FIGARO_DUMP_PATH}" > "${FIGARO_LOG_PATH}/log.txt" 2>&1;
}
#echo """KEYS" $@
main $@
#./figaro_test --gtest_filter=*ComputeSimpleHeadByOneAttrName > "${FIGARO_LOG_PATH}/log.txt" 2>&1
#valgrind --leak-check=yes ./figaro_test --gtest_filter=*ComputeSimpleHeadByOneAttrName > ../log/log.txt 2>&1