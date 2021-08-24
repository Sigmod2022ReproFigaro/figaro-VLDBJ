function init_global_paths()
{
    FIGARO_ROOT_PATH="${1:-/home/popina/Figaro/figaro-code/figaro}"
    FIGARO_DATA_PATH="/home/popina/Figaro/data"
    FIGARO_LOG_FILE_PATH="$FIGARO_ROOT_PATH/log/log.txt"
    FIGARO_BUILD_PATH="$FIGARO_ROOT_PATH/build"
    FIGARO_DUMP_FILE_PATH="$FIGARO_ROOT_PATH/dump/R.csv"
    FIGARO_DB_CONFIG_PATH="/home/popina/Figaro/figaro-code/system_tests/test2/databases/database_specs5.conf"
    FIGARO_QUERY_CONFIG_PATH="/home/popina/Figaro/figaro-code/system_tests/test2/databases/database_specs5.conf"
    FIGARO_TEST_MODE="DEBUG"
    FIGARO_PRECISION=14
    FIGARO_NUM_REPS=1
    FIGARO_NUM_THREADS=1
    FIGARO_POSTPROCESS="THICK"
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
        --num_repetitions=*)
            EXTENSION="${option#*=}"
            FIGARO_NUM_REPS=$EXTENSION
        ;;
        --postprocess=*)
            EXTENSION="${option#*=}"
            FIGARO_POSTPROCESS=$EXTENSION
        ;;
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

    echo "$@"
    cd "${FIGARO_BUILD_PATH}"
    echo "TESTMODE ${FIGARO_TEST_MODE}"
    echo "POSTPROCESSING ${FIGARO_POSTPROCESS}"
    echo "POSTPROCESSING ${FIGARO_LOG_FILE_PATH}"
    if [[ $FIGARO_TEST_MODE == DEBUG ]]; then
        cmake ../. -D FIGARO_RUN=ON -D FIGARO_DEBUG=ON
    elif [[ $FIGARO_TEST_MODE == INFO ]]; then
        cmake ../. -D FIGARO_RUN=ON -D FIGARO_INFO=ON
    elif [[ $FIGARO_TEST_MODE == UNIT_TEST ]]; then
        cmake ../. -D FIGARO_TEST=ON -D FIGARO_DEBUG=ON
    else
         cmake ../. -D FIGARO_RUN=ON
    fi
    # Used for generation of tests and libs.
    #cmake ../. -D FIGARO_RUN=ON -D FIGARO_TEST=ON -D FIGARO_LIB=ON
    make -j40 || exit 1
    case "${FIGARO_TEST_MODE}" in
    "DEBUG"|"INFO")
        ./figaro --db_config_path "${FIGARO_DB_CONFIG_PATH}" \
        --query_config_path "${FIGARO_QUERY_CONFIG_PATH}" \
        --precision "${FIGARO_PRECISION}" \
        --num_threads "${FIGARO_NUM_THREADS}" \
        --postprocess "${FIGARO_POSTPROCESS}" > "${FIGARO_LOG_FILE_PATH}" 2>&1;
        ;;
    "DUMP")
        ./figaro --db_config_path "${FIGARO_DB_CONFIG_PATH}" --dump_file_path "${FIGARO_DUMP_FILE_PATH}" \
            --query_config_path "${FIGARO_QUERY_CONFIG_PATH}" --precision "${FIGARO_PRECISION}" \
            --num_threads "${FIGARO_NUM_THREADS}" \
            --postprocess "${FIGARO_POSTPROCESS}" \
            >  "${FIGARO_LOG_FILE_PATH}"  2>&1;
        ;;
    "PERFORMANCE")
        ./figaro --db_config_path "${FIGARO_DB_CONFIG_PATH}" --query_config_path "${FIGARO_QUERY_CONFIG_PATH}" \
        --precision "${FIGARO_PRECISION}" --num_repetitions "${FIGARO_NUM_REPS}" \
        --num_threads "${FIGARO_NUM_THREADS}" \
        --postprocess "${FIGARO_POSTPROCESS}" >> \
         "${FIGARO_LOG_FILE_PATH}" 2>&1;
        ;;
    "UNIT_TEST")
        echo "*****************Running unit tests*****************"
        valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all ./figaro_test  ${FIGARO_DATA_PATH} \
        >   "${FIGARO_LOG_FILE_PATH}" 2>&1
        #./figaro_test \

       # ./figaro_test --gtest_filter=*ComputeSimpleHeadByOneMultipleAttributes \
        ;;
    esac
}
main $@
#valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all
#valgrind --leak-check=yes ./figaro_test --gtest_filter=*ComputeSimpleHeadByOneAttrName > ../log/log.txt 2>&1