#include "UtilTest.h"
#include "database/storage/ArrayStorage.h"
#include "database/storage/Matrix.h"
#include "database/Database.h"
#include "database/Relation.h"
#include "database/query/Query.h"
#include "database/query/visitor/result/ASTVisitorResultQR.h"
#include "database/query/visitor/result/ASTVisitorResultSVD.h"
#include <vector>
#include <string>
#include <cmath>

using namespace Figaro;


TEST(Database, DropAttrs) {
    static const std::string DB_CONFIG_PATH = getConfigPath(5) + DB_CONFIG_PATH_IN;
    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError = database.getInitializationErrorCode();
    Figaro::ErrorCode loadError;
    std::vector<std::string> attrNames;
    std::vector<std::string> expAttrNames = {"X25", "Y52", "Y53"};
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);

    database.dropAttributesFromRelations({"Y51"});

    attrNames = database.getRelationAttributeNames("R5");

    for (uint32_t idx = 0; idx < expAttrNames.size(); idx++)
    {
        EXPECT_EQ(attrNames[idx], expAttrNames[idx]);
    }
}

TEST(Database, BasicInput) {
    static const std::string DB_CONFIG_PATH = getConfigPath(1) + DB_CONFIG_PATH_IN;
    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
}


TEST(Database, PathQuery3) {
    static const std::string DB_CONFIG_PATH = getConfigPath(2) + DB_CONFIG_PATH_IN;
    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;
    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);
    database.sortData();
}


TEST(Database, BasicQueryParsing)
{
    static const std::string DB_CONFIG_PATH = getConfigPath(5) + DB_CONFIG_PATH_IN;
    static const std::string QUERY_CONFIG_PATH = getConfigPath(5) + "qr/" + QUERY_CONFIG_PATH_IN;

    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;
    std::map<std::vector<double>, uint32_t> downCounts;
    std::map<std::vector<double>, uint32_t> downParCounts;
    std::map<std::vector<double>, uint32_t> upParCounts;
    std::map<std::vector<double>, uint32_t> circCounts;

    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);

    Figaro::Query query(&database);
    EXPECT_EQ(query.loadQuery(QUERY_CONFIG_PATH), Figaro::ErrorCode::NO_ERROR);
}

TEST(Database, QRFigaroComputingCounts)
{
    static const std::string DB_CONFIG_PATH = getConfigPath(5) + DB_CONFIG_PATH_IN;
    static const std::string QUERY_CONFIG_PATH = getConfigPath(5) + + "qr/" + QUERY_CONFIG_PATH_IN;

    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;
    std::map<std::vector<uint32_t>, uint32_t> downCounts;
    std::map<std::vector<uint32_t>, uint32_t> downParCounts;
    std::map<std::vector<uint32_t>, uint32_t> upParCounts;
    std::map<std::vector<uint32_t>, uint32_t> circCounts;

    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);

    Figaro::Query query(&database);
    EXPECT_EQ(query.loadQuery(QUERY_CONFIG_PATH), Figaro::ErrorCode::NO_ERROR);
    query.evaluateQuery(false, {{"headsAndTails", true}});

    /*********************************** R4 ****************/
    downCounts =  database.getDownCounts("R4");
    EXPECT_EQ(downCounts.at({1}), 2);
    EXPECT_EQ(downCounts.at({2}), 3);
    EXPECT_EQ(downCounts.at({3}), 1);

    downParCounts = database.getParDownCnts("R4", {"X24"});
    EXPECT_EQ(downParCounts.at({1}), 2);
    EXPECT_EQ(downParCounts.at({2}), 3);
    EXPECT_EQ(downParCounts.at({3}), 1);

    upParCounts = database.getParUpCnts("R4", {"X24"});
    EXPECT_EQ(upParCounts.at({1}), 35);
    EXPECT_EQ(upParCounts.at({2}), 32);
    EXPECT_EQ(upParCounts.at({3}), 8);

    circCounts = database.getCircCounts("R4");
    EXPECT_EQ(circCounts.at({1}), 35);
    EXPECT_EQ(circCounts.at({2}), 32);
    EXPECT_EQ(circCounts.at({3}), 8);

    /*********************************** R5 ****************/
    downCounts =  database.getDownCounts("R5");
    EXPECT_EQ(downCounts.at({1}), 3);
    EXPECT_EQ(downCounts.at({2}), 1);
    EXPECT_EQ(downCounts.at({3}), 2);

    downParCounts = database.getParDownCnts("R5", {"X25"});
    EXPECT_EQ(downParCounts.at({1}), 3);
    EXPECT_EQ(downParCounts.at({2}), 1);
    EXPECT_EQ(downParCounts.at({3}), 2);

    upParCounts = database.getParUpCnts("R5", {"X25"});
    EXPECT_EQ(upParCounts.at({1}), 29);
    EXPECT_EQ(upParCounts.at({2}), 27);
    EXPECT_EQ(upParCounts.at({3}), 30);

    circCounts = database.getCircCounts("R5");
    EXPECT_EQ(circCounts.at({1}), 29);
    EXPECT_EQ(circCounts.at({2}), 27);
    EXPECT_EQ(circCounts.at({3}), 30);

    /*********************************** R3 ****************/
    downCounts =  database.getDownCounts("R3");
    EXPECT_EQ(downCounts.at({1}), 1);
    EXPECT_EQ(downCounts.at({2}), 2);
    EXPECT_EQ(downCounts.at({3}), 3);

    downParCounts = database.getParDownCnts("R3", {"X13"});
    EXPECT_EQ(downParCounts.at({1}), 1);
    EXPECT_EQ(downParCounts.at({2}), 2);
    EXPECT_EQ(downParCounts.at({3}), 3);

    upParCounts = database.getParUpCnts("R3", {"X13"});
    EXPECT_EQ(upParCounts.at({1}), 38);
    EXPECT_EQ(upParCounts.at({2}), 20);
    EXPECT_EQ(upParCounts.at({3}), 32);

    circCounts = database.getCircCounts("R3");
    EXPECT_EQ(circCounts.at({1}), 38);
    EXPECT_EQ(circCounts.at({2}), 20);
    EXPECT_EQ(circCounts.at({3}), 32);

    /*********************************** R2 ****************/
    downCounts =  database.getDownCounts("R2");
    EXPECT_EQ(downCounts.at({1, 1, 1}), 6);
    EXPECT_EQ(downCounts.at({1, 1, 3}), 4);
    EXPECT_EQ(downCounts.at({1, 2, 1}), 9);
    EXPECT_EQ(downCounts.at({1, 2, 3}), 6);
    EXPECT_EQ(downCounts.at({2, 2, 1}), 9);
    EXPECT_EQ(downCounts.at({2, 2, 2}), 3);
    EXPECT_EQ(downCounts.at({2, 3, 2}), 1);
    EXPECT_EQ(downCounts.at({3, 1, 2}), 2);
    EXPECT_EQ(downCounts.at({3, 1, 3}), 4);
    EXPECT_EQ(downCounts.at({3, 3, 2}), 1);

    downParCounts = database.getParDownCnts("R2", {"X12"});
    EXPECT_EQ(downParCounts.at({1}), 25);
    EXPECT_EQ(downParCounts.at({2}), 13);
    EXPECT_EQ(downParCounts.at({3}), 7);

    upParCounts = database.getParUpCnts("R2", {"X12"});
    EXPECT_EQ(upParCounts.at({1}), 4);
    EXPECT_EQ(upParCounts.at({2}), 3);
    EXPECT_EQ(upParCounts.at({3}), 5);

    circCounts =  database.getCircCounts("R2");
    EXPECT_EQ(circCounts.at({1, 1, 1}), 24);
    EXPECT_EQ(circCounts.at({1, 1, 3}), 16);
    EXPECT_EQ(circCounts.at({1, 2, 1}), 36);
    EXPECT_EQ(circCounts.at({1, 2, 3}), 24);
    EXPECT_EQ(circCounts.at({2, 2, 1}), 27);
    EXPECT_EQ(circCounts.at({2, 2, 2}), 9);
    EXPECT_EQ(circCounts.at({2, 3, 2}), 3);
    EXPECT_EQ(circCounts.at({3, 1, 2}), 10);
    EXPECT_EQ(circCounts.at({3, 1, 3}), 20);
    EXPECT_EQ(circCounts.at({3, 3, 2}), 5);

    /*********************************** R1 ****************/
    downCounts =  database.getDownCounts("R1");
    EXPECT_EQ(downCounts.at({1, 1}), 25);
    EXPECT_EQ(downCounts.at({1, 3}), 75);
    EXPECT_EQ(downCounts.at({2, 1}), 13);
    EXPECT_EQ(downCounts.at({2, 2}), 26);
    EXPECT_EQ(downCounts.at({3, 2}), 14);
    EXPECT_EQ(downCounts.at({3, 3}), 21);

    circCounts = database.getCircCounts("R1");
    EXPECT_EQ(circCounts.at({1, 1}), 25);
    EXPECT_EQ(circCounts.at({1, 3}), 75);
    EXPECT_EQ(circCounts.at({2, 1}), 13);
    EXPECT_EQ(circCounts.at({2, 2}), 26);
    EXPECT_EQ(circCounts.at({3, 2}), 14);
    EXPECT_EQ(circCounts.at({3, 3}), 21);
}

TEST(Database, QRFigaroHeadsAndTails)
{
    static const std::string DB_CONFIG_PATH = getConfigPath(5) + DB_CONFIG_PATH_IN;
    static const std::string QUERY_CONFIG_PATH = getConfigPath(5) + "qr/" +QUERY_CONFIG_PATH_IN;

    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;
    static constexpr uint32_t NUM_RELS = 5;
    std::array<Figaro::MatrixEigenT, NUM_RELS> head;
    std::array<Figaro::MatrixEigenT, NUM_RELS> expHead;
    std::array<Figaro::MatrixEigenT, NUM_RELS> tail;
    std::array<Figaro::MatrixEigenT, NUM_RELS> expTail;
    std::array<Figaro::MatrixEigenT, NUM_RELS> scales;
    std::array<Figaro::MatrixEigenT, NUM_RELS> expScales;
    std::array<Figaro::MatrixEigenT, NUM_RELS> dataScales;
    std::array<Figaro::MatrixEigenT, NUM_RELS> expDataScales;
    std::array<std::string, NUM_RELS> fileInputExpHead;
    std::array<std::string, NUM_RELS> fileInputExpTail;
    std::array<std::string, NUM_RELS> fileInputExpScales;
    std::array<std::string, NUM_RELS> fileInputExpDataScales;


    for (uint32_t idxRel = 0; idxRel < NUM_RELS; idxRel ++)
    {
        fileInputExpHead[idxRel] = getDataPath(5) + "qr/expectedHead" +
            std::to_string(idxRel + 1) + ".csv";
        fileInputExpTail[idxRel] = getDataPath(5) + "qr/expectedTail" +
            std::to_string(idxRel + 1) + ".csv";
        fileInputExpScales[idxRel] = getDataPath(5) + "qr/expectedScalesFirstPass" +
            std::to_string(idxRel + 1) + ".csv";
        fileInputExpDataScales[idxRel] = getDataPath(5) + "qr/expectedDataScalesFirstPass" +
            std::to_string(idxRel + 1) + ".csv";

        readMatrixDense(fileInputExpHead[idxRel], expHead[idxRel]);
        readMatrixDense(fileInputExpTail[idxRel], expTail[idxRel]);
        readMatrixDense(fileInputExpScales[idxRel], expScales[idxRel]);
        readMatrixDense(fileInputExpDataScales[idxRel], expDataScales[idxRel]);
    }

    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);

    Figaro::Query query(&database);
    EXPECT_EQ(query.loadQuery(QUERY_CONFIG_PATH), Figaro::ErrorCode::NO_ERROR);
    query.evaluateQuery(false, {{"headsAndTails", true}});

    for (uint32_t idxRel = 0; idxRel < NUM_RELS; idxRel++)
    {
        const std::string relName = "R" + std::to_string(idxRel + 1);
        const auto& headDT = database.getHead(relName);
        const auto& tailDT = database.getTail(relName);
        const auto& scaleDT = database.getScales(relName);
        const auto& dataScaleDT = database.getDataScales(relName);

        Figaro::Relation::copyMatrixDRowTToMatrixEigen(headDT, head[idxRel]);
        Figaro::Relation::copyMatrixDRowTToMatrixEigen(tailDT, tail[idxRel]);
        Figaro::Relation::copyMatrixDRowTToMatrixEigen(scaleDT, scales[idxRel]);
        Figaro::Relation::copyMatrixDRowTToMatrixEigen(dataScaleDT, dataScales[idxRel]);
        compareMatrices(head[idxRel], expHead[idxRel], true, true);
        compareMatrices(tail[idxRel], expTail[idxRel], true, true);
        compareMatrices(scales[idxRel], expScales[idxRel], true, true);
        compareMatrices(dataScales[idxRel], expDataScales[idxRel], true, true);
    }
}

TEST(Database, QRFigaroGeneralizedHeadsAndTails)
{
    static const std::string DB_CONFIG_PATH = getConfigPath(5) + DB_CONFIG_PATH_IN;
    static const std::string QUERY_CONFIG_PATH = getConfigPath(5) + "qr/" +QUERY_CONFIG_PATH_IN;

    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;
    Figaro::MatrixEigenT headGen1, headGen2, tailGen2;
    Figaro::MatrixEigenT expHeadGen1, expHeadGen2, expTailGen2;

    std::string fileInputExpHead2 = getDataPath(5) + "qr/expectedHeadGen2.csv";
    std::string fileInputExpHead1 = getDataPath(5) + "qr/expectedHeadGen1.csv";
    std::string fileInputExpTail2 = getDataPath(5) + "qr/expectedTailGen2.csv";

    readMatrixDense(fileInputExpHead2, expHeadGen2);
    readMatrixDense(fileInputExpHead1, expHeadGen1);
    readMatrixDense(fileInputExpTail2, expTailGen2);

    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);

    Figaro::Query query(&database);
    EXPECT_EQ(query.loadQuery(QUERY_CONFIG_PATH), Figaro::ErrorCode::NO_ERROR);
    query.evaluateQuery(false, {{"headsAndTails", true}, {"generalizedHeadsAndTails", true}});
    const auto& headDT = database.getGeneralizedHead("R2");
    const auto& tailDT = database.getGeneralizedTail("R2");
    Figaro::Relation::copyMatrixDRowTToMatrixEigen(headDT, headGen2);
    Figaro::Relation::copyMatrixDRowTToMatrixEigen(tailDT, tailGen2);
    compareMatrices(headGen2, expHeadGen2, true, true);
    compareMatrices(tailGen2, expTailGen2, true, true);
}


TEST(Database, QRFigaro)
{
    static const std::string DB_CONFIG_PATH = getConfigPath(5) + DB_CONFIG_PATH_IN;
    static const std::string QUERY_CONFIG_PATH = getConfigPath(5) + "qr/" +QUERY_CONFIG_PATH_IN;

    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;
    Figaro::MatrixEigenT headGen1, headGen2, tailGen2;
    Figaro::MatrixEigenT expHeadGen1, expHeadGen2, expTailGen2;


    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);

    Figaro::Query query(&database);
    EXPECT_EQ(query.loadQuery(QUERY_CONFIG_PATH), Figaro::ErrorCode::NO_ERROR);
    query.evaluateQuery(false, {{"headsAndTails", true}, {"generalizedHeadsAndTails", true},
                                {"postProcessing", true}});
}


TEST(Database, QRFigaroQOrt)
{
    static const std::string DB_CONFIG_PATH = getConfigPath(5) + DB_CONFIG_PATH_IN;
    static const std::string QUERY_CONFIG_PATH = getConfigPath(5) + "qr/" +QUERY_CONFIG_PATH_IN;

    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;
    Figaro::MatrixEigenT headGen1, headGen2, tailGen2;
    Figaro::MatrixEigenT expHeadGen1, expHeadGen2, expTailGen2;


    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);

    Figaro::Query query(&database);
    EXPECT_EQ(query.loadQuery(QUERY_CONFIG_PATH), Figaro::ErrorCode::NO_ERROR);
    query.evaluateQuery(false, {{"headsAndTails", true}, {"generalizedHeadsAndTails", true},
                                {"postProcessing", true}, {"computeQ", true}});
    Figaro::ASTVisitorResultAbs* pResult = query.getResult();
    Figaro::ASTVisitorResultQR* pQrResult = (Figaro::ASTVisitorResultQR*)pResult;
    double ortMeasure = database.checkOrthogonality(pQrResult->getQRelationName(), {});
    EXPECT_NEAR(ortMeasure, 0, GIVENS_TEST_PRECISION_ERROR);
}


TEST(Database, LUFigaroHeadsAndTails)
{
    static const std::string DB_CONFIG_PATH = getConfigPath(5) + DB_CONFIG_PATH_IN;
    static const std::string QUERY_CONFIG_PATH = getConfigPath(5) + "lu/" +QUERY_CONFIG_PATH_IN;

    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;
    static constexpr uint32_t NUM_RELS = 5;
    std::array<Figaro::MatrixEigenT, NUM_RELS> head;
    std::array<Figaro::MatrixEigenT, NUM_RELS> expHead;
    std::array<Figaro::MatrixEigenT, NUM_RELS> tail;
    std::array<Figaro::MatrixEigenT, NUM_RELS> expTail;
    std::array<Figaro::MatrixEigenT, NUM_RELS> scales;
    std::array<Figaro::MatrixEigenT, NUM_RELS> expScales;
    std::array<Figaro::MatrixEigenT, NUM_RELS> dataScales;
    std::array<Figaro::MatrixEigenT, NUM_RELS> expDataScales;
    std::array<std::string, NUM_RELS> fileInputExpHead;
    std::array<std::string, NUM_RELS> fileInputExpTail;
    std::array<std::string, NUM_RELS> fileInputExpScales;
    std::array<std::string, NUM_RELS> fileInputExpDataScales;


    for (uint32_t idxRel = 0; idxRel < NUM_RELS; idxRel ++)
    {
        fileInputExpHead[idxRel] = getDataPath(5) + "lu/expectedHead" +
            std::to_string(idxRel + 1) + ".csv";
        fileInputExpTail[idxRel] = getDataPath(5) + "lu/expectedTail" +
            std::to_string(idxRel + 1) + ".csv";

        readMatrixDense(fileInputExpHead[idxRel], expHead[idxRel]);
        readMatrixDense(fileInputExpTail[idxRel], expTail[idxRel]);
    }

    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);

    Figaro::Query query(&database);
    EXPECT_EQ(query.loadQuery(QUERY_CONFIG_PATH), Figaro::ErrorCode::NO_ERROR);
    query.evaluateQuery(false, {{"headsAndTails", true}});

    for (uint32_t idxRel = 0; idxRel < NUM_RELS; idxRel++)
    {
        const std::string relName = "R" + std::to_string(idxRel + 1);
        const auto& headDT = database.getHead(relName);
        const auto& tailDT = database.getTail(relName);

        Figaro::Relation::copyMatrixDRowTToMatrixEigen(headDT, head[idxRel]);
        Figaro::Relation::copyMatrixDRowTToMatrixEigen(tailDT, tail[idxRel]);
        compareMatrices(head[idxRel], expHead[idxRel], true, true);
        compareMatrices(tail[idxRel], expTail[idxRel], true, true);
    }
}



TEST(Database, LUFigaroGeneralizedHeadsAndTails)
{
    static const std::string DB_CONFIG_PATH = getConfigPath(5) + DB_CONFIG_PATH_IN;
    static const std::string QUERY_CONFIG_PATH = getConfigPath(5) + "lu/" +QUERY_CONFIG_PATH_IN;

    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;
    Figaro::MatrixEigenT headGen1, headGen2, tailGen2;
    Figaro::MatrixEigenT expHeadGen1, expHeadGen2, expTailGen2;

    std::string fileInputExpHead2 = getDataPath(5) + "lu/expectedHeadGen2.csv";
    std::string fileInputExpTail2 = getDataPath(5) + "lu/expectedTailGen2.csv";

    readMatrixDense(fileInputExpHead2, expHeadGen2);
    readMatrixDense(fileInputExpTail2, expTailGen2);

    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);

    Figaro::Query query(&database);
    EXPECT_EQ(query.loadQuery(QUERY_CONFIG_PATH), Figaro::ErrorCode::NO_ERROR);

    query.evaluateQuery(false, {{"headsAndTails", true}, {"generalizedHeadsAndTails", true}});

    const auto& headDT = database.getGeneralizedHead("R2");
    const auto& tailDT = database.getGeneralizedTail("R2");
    Figaro::Relation::copyMatrixDRowTToMatrixEigen(headDT, headGen2);
    Figaro::Relation::copyMatrixDRowTToMatrixEigen(tailDT, tailGen2);
    compareMatrices(headGen2, expHeadGen2, true, true);
    compareMatrices(tailGen2, expTailGen2, true, true);

    FIGARO_LOG_DBG("tailGen2", tailGen2)
    FIGARO_LOG_DBG("expTailGen2", expTailGen2)
}

TEST(Database, LUFigaro)
{
    static const std::string DB_CONFIG_PATH = getConfigPath(5) + DB_CONFIG_PATH_IN;
    static const std::string QUERY_CONFIG_PATH = getConfigPath(5) + "lu/" +QUERY_CONFIG_PATH_IN;

    Figaro::Database database(DB_CONFIG_PATH);
    Figaro::ErrorCode initError;
    Figaro::ErrorCode loadError;

    initError = database.getInitializationErrorCode();
    EXPECT_EQ(initError, Figaro::ErrorCode::NO_ERROR);
    loadError = database.loadData();
    EXPECT_EQ(loadError, Figaro::ErrorCode::NO_ERROR);

    Figaro::Query query(&database);
    EXPECT_EQ(query.loadQuery(QUERY_CONFIG_PATH), Figaro::ErrorCode::NO_ERROR);
    query.evaluateQuery(false, {{"headsAndTails", true}, {"generalizedHeadsAndTails", true},
                                {"postProcessing", true}, {"computeL", true}});
}


TEST(Relation, Join)
{
    static constexpr uint32_t M = 3, N = 3, K= 2;
    Relation::MatrixDRowT A(M, N), B(K, N), C(K, K);

    A[0][0] = 1;
    A[0][1] = 2;
    A[0][2] = 3;

    A[1][0] = 1;
    A[1][1] = 4;
    A[1][2] = 6;

    A[2][0] = 1;
    A[2][1] = 6;
    A[2][2] = 7;

    B[0][0] = 1;
    B[0][1] = 1;
    B[0][2] = 4;


    B[1][0] = 1;
    B[1][1] = 2;
    B[1][2] = 5;

    C[0][0] = 1;
    C[0][1] = 2;

    C[1][0] = 1;
    C[1][1] = 3;


    Relation relA("A", std::move(A),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("A1", Relation::AttributeType::FLOAT),
         Relation::Attribute("A2", Relation::AttributeType::FLOAT)});

    Relation relB("B", std::move(B),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("B1", Relation::AttributeType::FLOAT),
         Relation::Attribute("B2", Relation::AttributeType::FLOAT)});

    Relation relC("C", std::move(C),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("C1", Relation::AttributeType::FLOAT)});

    FIGARO_LOG_DBG("relA", relA)
    FIGARO_LOG_DBG("relB", relB)
    FIGARO_LOG_DBG("relC", relC)
    Relation joinRel1 = relA.joinRelations({&relB, &relC}, {"A"}, {"A"}, {{"A"},{"A"}}, false);
    Relation joinRel2 = relA.joinRelations({&relB}, {"A"}, {}, {{"A"}}, false);

    FIGARO_LOG_DBG(joinRel1);
    FIGARO_LOG_DBG(joinRel2);
}


TEST(Relation, JoinLeapFrog)
{
    static constexpr uint32_t M = 3, N = 3, K= 2;
    Relation::MatrixDRowT A(M, N), B(K, N), C(K, K);

    A[0][0] = 1;
    A[0][1] = 2;
    A[0][2] = 3;

    A[1][0] = 1;
    A[1][1] = 4;
    A[1][2] = 6;

    A[2][0] = 1;
    A[2][1] = 6;
    A[2][2] = 7;

    B[0][0] = 1;
    B[0][1] = 1;
    B[0][2] = 4;


    B[1][0] = 1;
    B[1][1] = 2;
    B[1][2] = 5;

    C[0][0] = 1;
    C[0][1] = 2;

    C[1][0] = 1;
    C[1][1] = 3;


    Relation relA("A", std::move(A),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("A1", Relation::AttributeType::FLOAT),
         Relation::Attribute("A2", Relation::AttributeType::FLOAT)});

    Relation relB("B", std::move(B),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("B1", Relation::AttributeType::FLOAT),
         Relation::Attribute("B2", Relation::AttributeType::FLOAT)});

    Relation relC("C", std::move(C),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("C1", Relation::AttributeType::FLOAT)});
    FIGARO_LOG_DBG("relA", relA)
    FIGARO_LOG_DBG("relB", relB)
    FIGARO_LOG_DBG("relC", relC)
    std::vector<Relation*> vRels = {&relA, &relB, &relC};
    std::vector<Relation*> vParRels = {nullptr, &relA, &relA};
    std::vector<std::vector<std::string> > vvJoinAttrs = {{"A"}, {"A"}, {"A"}};
    std::vector<std::vector<std::string> > vvParJoinAttrs = {{}, {"A"}, {"A"}};
    FIGARO_LOG_DBG("MAJMUN")
    Relation joinRel = Relation::joinRelations(vRels, vParRels, vvJoinAttrs, vvParJoinAttrs, 12);
    FIGARO_LOG_DBG(joinRel);
}

TEST(Relation, Multiply)
{
    static constexpr uint32_t M = 3, N = 4, K= 2;
    Relation::MatrixDRowT A(M, N), B(K, K);

    A[0][0] = 1;
    A[0][1] = 2;
    A[0][2] = 2;
    A[0][3] = 3;

    A[1][0] = 1;
    A[1][1] = 2;
    A[1][2] = 4;
    A[1][3] = 6;

    A[2][0] = 1;
    A[2][1] = 2;
    A[2][2] = 6;
    A[2][3] = 7;

    B[0][0] = 1;
    B[0][1] = 1;

    B[1][0] = 1;
    B[1][1] = 2;


    Relation relA("A", std::move(A),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("AA", Relation::AttributeType::FLOAT),
         Relation::Attribute("A1", Relation::AttributeType::FLOAT),
         Relation::Attribute("A2", Relation::AttributeType::FLOAT)});

    Relation relB("B", std::move(B),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("B1", Relation::AttributeType::FLOAT)});

    Relation rel = relA.multiply(relB, {"A", "AA"}, {"A"});
}

TEST(Relation, DISABLED_SelfMatrixMultiply)
{
    static constexpr uint32_t M = 3, N = 4;
    Relation::MatrixDRowT A(M, N);

    A[0][0] = 1;
    A[0][1] = 2;
    A[0][2] = 2;
    A[0][3] = 3;

    A[1][0] = 1;
    A[1][1] = 2;
    A[1][2] = 4;
    A[1][3] = 6;

    A[2][0] = 1;
    A[2][1] = 2;
    A[2][2] = 6;
    A[2][3] = 7;


    Relation relA("A", std::move(A),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("AA", Relation::AttributeType::FLOAT),
         Relation::Attribute("A1", Relation::AttributeType::FLOAT),
         Relation::Attribute("A2", Relation::AttributeType::FLOAT)});

    FIGARO_LOG_DBG("relA", relA)
    Relation rel = relA.selfMatrixMultiply({});
    FIGARO_LOG_DBG("rel", rel)
}


TEST(Relation, CheckOrthogonality)
{
    static constexpr uint32_t M = 4, N = 2;
    Relation::MatrixDRowT A(M, N);

    A[0][0] = -0.109108945117996;
    A[0][1] = -0.829515062006254;

    A[1][0] = -0.327326835353989;
    A[1][1] = -0.439155032826839;

    A[2][0] = -0.545544725589981;
    A[2][1] = -0.048795003647426;

    A[3][0] = -0.763762615825974;
    A[3][1] = 0.341565025531986;


    Relation relA("A", std::move(A),
        {Relation::Attribute("A1", Relation::AttributeType::FLOAT),
         Relation::Attribute("A2", Relation::AttributeType::FLOAT)});

    FIGARO_LOG_DBG("relA", relA)
    double orth = relA.checkOrthogonality({});
    EXPECT_NEAR(orth, 0, QR_TEST_PRECISION_ERROR);
    FIGARO_LOG_DBG("orth", orth)
}


TEST(Relation, Norm)
{
    static constexpr uint32_t M = 3, N = 4;
    Relation::MatrixDRowT A(M, N);

    A[0][0] = 1;
    A[0][1] = 2;
    A[0][2] = 2;
    A[0][3] = 3;

    A[1][0] = 1;
    A[1][1] = 2;
    A[1][2] = 4;
    A[1][3] = 6;

    A[2][0] = 1;
    A[2][1] = 2;
    A[2][2] = 6;
    A[2][3] = 7;


    Relation relA("A", std::move(A),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("AA", Relation::AttributeType::FLOAT),
         Relation::Attribute("A1", Relation::AttributeType::FLOAT),
         Relation::Attribute("A2", Relation::AttributeType::FLOAT)});

    FIGARO_LOG_DBG("relA", relA)
    double norm = relA.norm({"A", "AA"});
    EXPECT_NEAR(norm, 12.247448713916, QR_TEST_PRECISION_ERROR);
    FIGARO_LOG_DBG("norm", norm)
}

TEST(Relation, CondNumber)
{
    static constexpr uint32_t M = 4, N = 3;
    Relation::MatrixDRowT A(M, N);

    A[0][0] = 1;
    A[0][1] = 7;
    A[0][2] = 2;
    //A[0][3] = 3;

    A[1][0] = 27;
    A[1][1] = 13;
    A[1][2] = 4;
    //A[1][3] = 6;

    A[2][0] = 22;
    A[2][1] = 21;
    A[2][2] = 6;
    //A[2][3] = 7;

    A[3][0] = 13;
    A[3][1] = 25;
    A[3][2] = 21;
    //A[3][3] = 23;


    Relation relA("A", std::move(A),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("AA", Relation::AttributeType::FLOAT),
         Relation::Attribute("A1", Relation::AttributeType::FLOAT),
         Relation::Attribute("A2", Relation::AttributeType::FLOAT)});

    FIGARO_LOG_DBG("relA", relA)
    double estCondNumber = relA.estimateConditionNumber({});
    EXPECT_NEAR(estCondNumber, 10.189359555015979, QR_TEST_PRECISION_ERROR);
    FIGARO_LOG_DBG("estCondNumber", estCondNumber)
}

TEST(Relation, AdditionAndSubtraction)
{
    static constexpr uint32_t M = 2, N = 3;
    Relation::MatrixDRowT A(M, N), B(M, N);
    A[0][0] = 1;
    A[0][1] = 2;
    A[0][2] = 3;

    A[1][0] = 4;
    A[1][1] = 5;
    A[1][2] = 6;

    B[0][0] = 7;
    B[0][1] = 8;
    B[0][2] = 9;

    B[1][0] = 10;
    B[1][1] = 11;
    B[1][2] = 12;

    Relation relA("A", std::move(A),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("A1", Relation::AttributeType::FLOAT),
         Relation::Attribute("A2", Relation::AttributeType::FLOAT)});

    Relation relB("B", std::move(B),
        {Relation::Attribute("B", Relation::AttributeType::FLOAT),
         Relation::Attribute("B1", Relation::AttributeType::FLOAT),
         Relation::Attribute("B2", Relation::AttributeType::FLOAT)});

    auto relC = relA.addRelation(relB, {"A"}, {"B"});
    auto relD = relA.subtractRelation(relB, {"A"}, {"B"});
    FIGARO_LOG_DBG("relA", relA)
    FIGARO_LOG_DBG("relB", relB)
    FIGARO_LOG_DBG("relC", relC)
    FIGARO_LOG_DBG("relD", relD)
}

TEST(Database, DISABLED_Multiply2)
{
    static constexpr uint32_t M = 3, N = 4, K= 2;
    Relation::MatrixDRowT A(M, N), B(K, K), R(N, N);

    A[0][0] = 1;
    A[0][1] = 2;
    A[0][2] = 2;
    A[0][3] = 3;

    A[1][0] = 1;
    A[1][1] = 2;
    A[1][2] = 4;
    A[1][3] = 6;

    A[2][0] = 1;
    A[2][1] = 2;
    A[2][2] = 6;
    A[2][3] = 7;

    B[0][0] = 1;
    B[0][1] = 1;

    B[1][0] = 1;
    B[1][1] = 2;


    R[0][0] = -4.899;
    R[0][1] = -9.798;
    R[0][2] = -13.0639;
    R[0][3] = -3.6742;

    R[1][0] = 0;
    R[1][1] = 4;
    R[1][2] = 4;
    R[1][3] = 0;

    R[2][0] = 0;
    R[2][1] = 0;
    R[2][2] = -1.1547;
    R[2][3] = 0;

    R[3][0] = 0;
    R[3][1] = 0;
    R[3][2] = 0;
    R[3][3] = -1.2247;

    Relation relA("A", std::move(A),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("AA", Relation::AttributeType::FLOAT),
         Relation::Attribute("A1", Relation::AttributeType::FLOAT),
         Relation::Attribute("A2", Relation::AttributeType::FLOAT)});

    Relation relB("B", std::move(B),
        {Relation::Attribute("A", Relation::AttributeType::FLOAT),
         Relation::Attribute("B1", Relation::AttributeType::FLOAT)});

    Relation relR("R", std::move(R),
        {Relation::Attribute("R1", Relation::AttributeType::FLOAT),
         Relation::Attribute("R2", Relation::AttributeType::FLOAT),
         Relation::Attribute("R3", Relation::AttributeType::FLOAT),
         Relation::Attribute("R4", Relation::AttributeType::FLOAT)});

    std::vector<Relation> vRels;

    vRels.emplace_back(std::move(relA.copyRelation()));
    vRels.emplace_back(std::move(relB.copyRelation()));
    vRels.emplace_back(std::move(relR.copyRelation()));

    Database database(std::move(vRels));
    FIGARO_LOG_DBG(relA, relB);
    auto joinRelName = database.joinRelations("COPY_A", {"COPY_B"}, {"A"}, {}, {{"A"}}, false);
    auto rInvName = database.inverse("COPY_R", {});

    auto qName = database.multiply(joinRelName, rInvName, {}, {}, 0);
    auto AInvRname = database.multiply("COPY_A", rInvName, {"A"}, {}, 0);
    auto BInvRname = database.multiply("COPY_B", rInvName, {"A"}, {}, 3);
    auto qWay2 = database.joinRelationsAndAddColumns(
            AInvRname, {BInvRname}, {"A"}, {}, {{"A"}}, false );

    database.outputRelation(rInvName);
    database.outputRelation("COPY_A");
    database.outputRelation("COPY_B");
    //database.outputRelation(joinRelName);

    database.outputRelation(AInvRname);
    database.outputRelation(BInvRname);
    database.outputRelation(qName);
    database.outputRelation(qWay2);
}
