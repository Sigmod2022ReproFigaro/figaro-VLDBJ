function init_global_paths()
{
    FIGARO_ROOT_PATH="${1:-/home/popina/Figaro/figaro-code/figaro}"
    FIGARO_LOG_FILE_PATH="$FIGARO_ROOT_PATH/log/log.txt"
    FIGARO_BUILD_PATH="$FIGARO_ROOT_PATH/build"
    FIGARO_DUMP_PATH="$FIGARO_ROOT_PATH/dump"
    FIGARO_DB_CONFIG_PATH="/home/popina/Figaro/figaro-code/system_tests/test1/databases/database_specs11.conf"
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
        
        -l=*|--log_file_path=*)
            EXTENSION="${option#*=}"
            FIGARO_LOG_FILE_PATH="${EXTENSION}"
        ;;
        -t=*|--test_mode=*)
            EXTENSION="${option#*=}"
            FIGARO_TEST_MODE=$EXTENSION
        ;;
        --dump_path=*)
            EXTENSION="${option#*=}"
            FIGARO_DUMP_PATH=$EXTENSION
        ;;
        --db_config_path=*)
            EXTENSION="${option#*=}"
            FIGARO_DB_CONFIG_PATH=$EXTENSION
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

    echo "$@"
    cd "${FIGARO_BUILD_PATH}"
    echo "TESTMODE ${FIGARO_TEST_MODE}"
    if [[ $FIGARO_TEST_MODE == DEBUG ]]; then 
        cmake ../. -D FIGARO_RUN=ON -D FIGARO_DEBUG=ON
    elif [[ $FIGARO_TEST_MODE == UNIT_TEST ]]; then
        cmake ../. -D FIGARO_TEST=ON -D FIGARO_DEBUG=ON
    else
         cmake ../. -D FIGARO_RUN=ON 
    fi
    # Used for generation of tests and libs. 
    #cmake ../. -D FIGARO_RUN=ON -D FIGARO_TEST=ON -D FIGARO_LIB=ON
    make -j8
    case "${FIGARO_TEST_MODE}" in
    "DEBUG")
        ./figaro --db_config_path "${FIGARO_DB_CONFIG_PATH}" --precision "${FIGARO_PRECISION}" > "${FIGARO_LOG_FILE_PATH}" 2>&1;
        ;;
    "DUMP")
        ./figaro --db_config_path "${FIGARO_DB_CONFIG_PATH}" --dump_path "${FIGARO_DUMP_PATH}" \
            --precision "${FIGARO_PRECISION}"  >  "${FIGARO_LOG_FILE_PATH}"  2>&1;
        ;;
    "PERFORMANCE")
        ./figaro --db_config_path "${FIGARO_DB_CONFIG_PATH}" \
        --precision "${FIGARO_PRECISION}" > "${FIGARO_LOG_FILE_PATH}" 2>&1;
        ;;
    "UNIT_TEST")
        echo "*****************Running unit tests*****************"
        ./figaro_test \
        >   "${FIGARO_LOG_FILE_PATH}" 2>&1
        
       # ./figaro_test --gtest_filter=*ComputeSimpleHeadByOneMultipleAttributes \
        #./figaro_test  --gtest_filter=*BasicQueryParsing \
        ;;
    esac
}
main $@
#valgrind --leak-check=yes ./figaro_test --gtest_filter=*ComputeSimpleHeadByOneAttrName > ../log/log.txt 2>&1