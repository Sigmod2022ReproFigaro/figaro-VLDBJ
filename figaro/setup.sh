function init_global_paths()
{
    FIGARO_ROOT_PATH="${1:-/home/---/Figaro/figaro-code/figaro}"
    FIGARO_DATA_PATH="/home/---/Figaro/data"
    FIGARO_LOG_FILE_PATH="$FIGARO_ROOT_PATH/log/log.txt"
    FIGARO_BUILD_PATH="$FIGARO_ROOT_PATH/build"
    FIGARO_DUMP_FILE_PATH="$FIGARO_ROOT_PATH/dump/R.csv"
    FIGARO_DB_CONFIG_PATH="/home/---/Figaro/figaro-code/system_tests/test2/databases/database_specs5.conf"
    FIGARO_QUERY_CONFIG_PATH="/home/---/Figaro/figaro-code/system_tests/test2/databases/database_specs5.conf"
    FIGARO_TEST_MODE="DEBUG"
    FIGARO_PRECISION=14
    FIGARO_NUM_THREADS=1
    FIGARO_MEMORY_LAYOUT="ROW_MAJOR"
    FIGARO_DECOMP_ALG="HOUSEHOLDER"
    FIGARO_SUB_METHOD="HOUSEHOLDER"
    FIGARO_NUM_SING_VALS=1
    FIGARO_NUM_ITERATIONS=1
    FIGARO_COMPUTE_ALL=false
    FIGARO_PROFILER_DUMP_PATH="..."
    FIGARO_HELP_SHOW=false
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
        --dump_file_path=*)
            EXTENSION="${option#*=}"
            FIGARO_DUMP_FILE_PATH=$EXTENSION
        ;;
        --data_path=*)
            EXTENSION="${option#*=}"
            FIGARO_DATA_PATH=$EXTENSION
        ;;
        --db_config_path=*)
            EXTENSION="${option#*=}"
            FIGARO_DB_CONFIG_PATH=$EXTENSION
        ;;
        --query_config_path=*)
            EXTENSION="${option#*=}"
            FIGARO_QUERY_CONFIG_PATH=$EXTENSION
        ;;
        --precision=*)
            EXTENSION="${option#*=}"
            FIGARO_PRECISION=$EXTENSION
        ;;
        --num_threads=*)
            EXTENSION="${option#*=}"
            FIGARO_NUM_THREADS=$EXTENSION
        ;;
        --memory_layout=*)
            EXTENSION="${option#*=}"
            FIGARO_MEMORY_LAYOUT=$EXTENSION
        ;;
        --decomp_alg=*)
            EXTENSION="${option#*=}"
            FIGARO_DECOMP_ALG=$EXTENSION
        ;;
        --sub_method=*)
            EXTENSION="${option#*=}"
            FIGARO_SUB_METHOD=$EXTENSION
        ;;
        --compute_all=*)
            EXTENSION="${option#*=}"
            FIGARO_COMPUTE_ALL=$EXTENSION
        ;;
        --num_sing_vals=*)
            EXTENSION="${option#*=}"
            FIGARO_NUM_SING_VALS=$EXTENSION
        ;;
        --num_iterations=*)
            EXTENSION="${option#*=}"
            FIGARO_NUM_ITERATIONS=$EXTENSION
        ;;
        --profiler_dump_path=*)
            EXTENSION="${option#*=}"
            FIGARO_PROFILER_DUMP_PATH=$EXTENSION
        ;;
        -h|--help)
        FIGARO_HELP_SHOW=true
        echo $"
Usage: setup.sh [-h --help]
[-r|--root=<PATH>]
[-t|--test_mode=<NAME>]
[--data_path=<PATH>][-l|--log_file_path=<PATH>][--dump_file_path=<PATH>]
[--db_config_path=<PATH>][--query_config_path=<PATH>]
[--precision=<NUMBER>][--num_threads=<NUMBER>]
Run evaluation of Figaro on the specified database for the specified query.
    -r, --root=<PATH>            set the root path to Figaro system;
    -t, --test_mode=<NAME> specifies whether it should build dump, performance, debug or info version of the system. performance is used in performance evaluation by omittin all log outputs. dump dumps the R;
    --data_path=<PATH> specifies path to data. It is used only in unit tests;
    --log_file_path=<PATH> specifies where the log will be stored;
    --dump_file_path=<PATH> specifies where the R will be dumped;
    --db_config_path=<PATH> specifies the location of database configuration used by the system.
    --query_config_path=<PATH> specifies the location of query configuration used by the system.
    --precision=<NUMBER> specifies double precision in which data is dumped.
    --num_threads=<NUMBER> specifies the number of threads used by the system.
    -h, --help                   show help.
"       ;;
        *)    # unknown option
        echo "Wrong  argument" $option
        ;;
    esac
    done
}

function main()
{
    export CC=gcc-10
    export CXX=g++-10

    init_global_paths
    get_str_args "$@"

    [[ $FIGARO_HELP_SHOW == true ]] && return

    echo "$@"
    cd "${FIGARO_BUILD_PATH}"
    echo "TESTMODE ${FIGARO_TEST_MODE}"
    if [[ $FIGARO_TEST_MODE == DEBUG ]]; then
        cmake ../. -D FIGARO_RUN=ON -D FIGARO_DEBUG=ON
    elif [[ $FIGARO_TEST_MODE == INFO ]]; then
        cmake ../. -D FIGARO_RUN=ON -D FIGARO_INFO=ON
    elif [[ $FIGARO_TEST_MODE == MICRO_BENCHMARK ]]; then
        cmake ../. -D FIGARO_RUN=ON -D FIGARO_MICRO_BENCHMARK=ON
    elif [[ $FIGARO_TEST_MODE == UNIT_TEST ]]; then
        cmake ../. -D FIGARO_TEST=ON -D FIGARO_DEBUG=ON
    else
         cmake ../. -D FIGARO_RUN=ON
    fi
    # Used for generation of tests and libs.
    #cmake ../. -D FIGARO_RUN=ON -D FIGARO_TEST=ON -D FIGARO_LIB=ON
    make -j40 || exit 1

    ARGS=""
    ARGS+="--db_config_path ${FIGARO_DB_CONFIG_PATH} "
    ARGS+="--query_config_path ${FIGARO_QUERY_CONFIG_PATH} "
    ARGS+="--precision ${FIGARO_PRECISION} "
    ARGS+="--num_threads ${FIGARO_NUM_THREADS} "
    ARGS+="--memory_layout ${FIGARO_MEMORY_LAYOUT} "
    ARGS+="--compute_all ${FIGARO_COMPUTE_ALL} "
    ARGS+="--decomposition_algorithm ${FIGARO_DECOMP_ALG} "
    ARGS+="--num_sing_vals ${FIGARO_NUM_SING_VALS} "
    ARGS+="--num_iterations ${FIGARO_NUM_ITERATIONS} "
    ARGS+="--sub_method ${FIGARO_SUB_METHOD} "

    echo $ARGS

    case "${FIGARO_TEST_MODE}" in
    "DEBUG"|"INFO")
        ./figaro $(echo $ARGS) \
        > "${FIGARO_LOG_FILE_PATH}" 2>&1;
        ;;
    "DUMP")
        ./figaro $(echo $ARGS) \
        --dump_file_path "${FIGARO_DUMP_FILE_PATH}" \
        >> "${FIGARO_LOG_FILE_PATH}" 2>&1;
        ;;
    "PERFORMANCE")
        ./figaro $(echo $ARGS) \
        >> "${FIGARO_LOG_FILE_PATH}" 2>&1;
        ;;
    "MICRO_BENCHMARK")
        ./figaro $(echo $ARGS) \
        >> "${FIGARO_LOG_FILE_PATH}" 2>&1;
        ;;
    "PROFILER_THREADS")
        vtune -collect threading -result-dir "${FIGARO_PROFILER_DUMP_PATH}"  \
        ./figaro $(echo $ARGS) \
        > "${FIGARO_LOG_FILE_PATH}" 2>&1;
        ;;
    "PROFILER_MEMORY")
        vtune -collect memory-access -result-dir "${FIGARO_PROFILER_DUMP_PATH}" \
        ./figaro $(echo $ARGS) \
         >"${FIGARO_LOG_FILE_PATH}" 2>&1;
        ;;
    "PROFILER_HOTSPOTS")
        vtune -collect hotspots -result-dir "${FIGARO_PROFILER_DUMP_PATH}" \
        ./figaro $(echo $ARGS) \
         >"${FIGARO_LOG_FILE_PATH}" 2>&1;
        ;;
    "UNIT_TEST")
        echo "*****************Running unit tests*****************"
        #vtune -collect performance-snapshot
        ./figaro_test ${FIGARO_DATA_PATH} --gtest_filter=*Matrix* > "${FIGARO_LOG_FILE_PATH}" 2>&1
        #valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all ./figaro_test  ${FIGARO_DATA_PATH} \
        #>   "${FIGARO_LOG_FILE_PATH}" 2>&1
        #./figaro_test \

       # ./figaro_test --gtest_filter=*Multiplication \
        ;;
    esac
}
main $@
#valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all
#valgrind --leak-check=yes ./figaro_test --gtest_filter=*ComputeSimpleHeadByOneAttrName > ../log/log.txt 2>&1
