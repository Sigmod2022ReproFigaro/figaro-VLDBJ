Figaro system
==================

This directory contains implementation of Figaro system.
It is divided in the following directories:
- [CMakeLists.txt](CMakeLists.txt): specifying how the system is built; **Note**: LD_LIBRARY_PATH and LIBRARY_PATH in .bashrc were altered to include intel/mkl/2021.2.0/lib/intel64/ path.
- [setup.sh](setup.sh) : running figaro system with the specific configuration. Run setup.sh --help for more details.
- [clean.sh](clean.sh) : cleaning built files;
- build: storing binary files;
- dump, log: the same purpose as in root directory;
- src: storing source files; It contains the following files and directories:
    - [main.cpp](src/main.cpp) It contains entry point to Figaro system. It works by first loading database configuration alognside the specified data. Further, it loads query. After that it evaluates query and if needed dumps the computed R.
    - [Utils.cpp](src/utils/Utils.cpp) It contains functions for setting the number of threads, outputting relation to csv and setIntersection.
    - [Database.cpp](src/database/Database.cpp) It has several groups of functions for different needs: 
        - Database loading and initialization. This group allows loading database schema (loadDatabaseRelationsSchema), loading data (loadData), restore relations to their initial state if the computations change them in place (resetComputations)
        - Basic modifying operations on relations. This group includes: sorting a specified relation based on the specified attributes (sortRelation), removing specified attributes from the specified relation (dropAttributesFromRelations), updating schema of relation which include reordering of attributes, and deletion (updateSchemaOfRelation), one hot encoding all categorical attributes in all relations and then dropping the first column of a one hot encoded attributes.
        - Figaro algorithm parts + postprocessing that:
            - compute needed count queries: down count queries (computeDownCounts), up and circle count queries (computeUpAndCircleCounts), 
            - that evaluate parts of Algorithm 1 and 2: computeHeadsAndTails, aggregateAwayChildrenRelations, 
            - apply the corresponding postprocessing method: computePostprocessing. In particular this method upper triangularizes the following three types of blocks: 
                - concatenated tails corresponding to a relation: it calls for each of them computeQROfTail;
                - concatenated generalized tails corresponding to a join tree node: it calls for each of them computeQROfGeneralizedTail;
                - output of Figaro algorithm: it calls: computeQROfGeneralizedHead;
            After that it concatenates upper triangularized Rs and applies again upper triangularization method: computeQROfConcatenatedGeneralizedHeadAndTails
        - Apply postprocessing method to the join result (evalPostprocessing)
    - [Relation.cpp](src/database/Relation.cpp) It implements the main logic of Figaro operations on relations. Its functions can be divided in several groups: 
        - basic modifying and accessing operations on schema: getAttributeIdx, getAttributesIdxs,getAttributesIdxsComplement, getCategoricalAttributeIdxs, schemaJoins, schemaRemoveNonParJoinAttrs, schemaDropAttrs, schemaOneHotEncode
        - mutliple index building for parallel and faster evaluation of Figaro algorithm: getDistinctValsAndBuildIndices. In cases when composite join attribute is a primary key we skip certain checks. This lead to significant speed up in index building.
        - sequential hash table that accepts variable number of arguments. These functions support it:  initHashTableRowIdxs, getChildRowIdx, destroyHashTableRowIdxs. This implementation is used for faster evaluation of a join of two tables when we aggregate away certain join attributes  that do not appear in parent nodes.
        - concurrent hash table that accepts variable number of key elements as tuple (for now up to two supported). These functions support it: 
        initHashTable, insertParDownCntFromHashTable, getParCntFromHashTable. This is used when we compute up and down counts and we need to add values in the table concurrently.
        - basic modifiers on data + schema: 
            - loading data (loadData) 
            - sorting (sortData)
            - dropping attributes: dropAttributes 
            - modifying schema and the corresponding data: updateSchema
    - count queries used in Figaro:
        - computeDownCounts. This implementation first builds indices over the data using getDistinctValsAndBuildIndices. Then it parallelizes computation of for loops in Algorithm using computed indices. 
        - computeUpAndCircleCounts. It reuses computed in the computeDownCounts and parallelizes computation of up and circle queries with needed synchronizations.
    - parts of Figaro algorithm that are implementation of Database.cpp functions: computeHeadsAndTails, aggregateAwayChildrenRelations, computeAndScaleGeneralizedHeadAndTail, computeQROfGeneralizedHead, computeQROfTail, computeQROfGeneralizedTail, computeQROfConcatenatedGeneralizedHeadAndTails
    - postprocessing on the join result: computeQR 
    - database/query: join order specification where we specify: join tree data structure alongside join tree node types, join tree visitors and query evaluators.
        - visitors:
            - [ASTJoinAttributesComputeVisitor.cpp](src/database/query/ASTJoinAttributesComputeVisitor.cpp). Given a join tree, for relation node it computes join attributes and parent join attributes of a relation  and it sorts a relation based on its join attributes. It further changes schema of relations based on the query configuration, one hot encode categorical attributes in relations and drops certain attributes.
            - Down counts computation: [ASTComputeDownCountsVisitor.cpp](src/database/query/ASTComputeDownCountsVisitor.cpp). It computes down counts corresponding to the join tree. (pass 1 of computing count queries)
            - Up and circle computation: [ASTComputeUpAndCircleCountsVisitor.cpp](src/database/query/ASTComputeUpAndCircleCountsVisitor.cpp) It computes up and circle counts corresponding to the join tree. (pass 2 of computing count queries)
            - First pass visitor: [ASTFigaroFirstPassVisitor.cpp](src/database/query/ASTFigaroFirstPassVisitor.cpp) It implements lines 6-7 of Algorithm 1 in the paper. It effectively computes heads and tails of submatrices corresponding to the same tuple of join attributes.
             - Second pass visitor: [ASTFigaroSecondPassVisitor.cpp](src/database/query/ASTFigaroSecondPassVisitor.cpp) It implements the rest of Algorithm 1 and Algorithm 2 in the paper and the postprocessing step applied on the result of Algorithm 2.
             - Postprocessing QR visitor: [ASTPostProcQRVisitor.cpp](src/database/query/ASTPostProcQRVisitor.cpp) It implements the postprocessing computation: thin and MKL on the join result.

        - join tree node type specification:
            - ASTNodeJoin.cpp [ASTNodeJoin.cpp](src/database/query/ASTNodeJoin.cpp)  it specifies join type of a node in a query. Each join type has central relation and at least one children nodes.
            - ASTNodePostProcQR.cpp [ASTNodePostProcQR.cpp](src/database/query/ASTNodePostProcQR.cpp) it specifies postProcQR node. It is used only postprocessing on the join result. It is always a root of the join tree.
            - ASTNodeQRGivens.cpp [ASTNodeQRGivens.cpp](src/database/query/ASTNodeQRGivens.cpp) it specifies QR Givens node in a join tree. It is always a root of the join tree.
            - ASTNodeRelation.cpp it specifies Relation node in a join tree. It is always a leaf in join tree or a central relation in a join node.
        - query evaluator:
            It evaluates query specified by the join tree. If the root is ASTNodePostProcQR it assumes there is only one more node in the join tree: relational node.

            If the root is ASTNodeQRGivens it uses different visiting orders depending on the visitor.  ASTJoinAttributesComputeVisitor, ASTComputeDownCountsVisitor, ASTFigaroFirstPassVisitor, ASTFigaroSecondPassVisitor use postorder, while ASTComputeDownCountsVisitor uses preorder.
- include: storing header files for the corresponding source files. The only additional files are inside utils and database/storage. In particular:
    - in utils ErrorCode.h: error code, Logger.h: logging functions, Performance.h: microbenchmark functions.
    in database/storage : ArrayStorage.h: specifies inlined data structure that is used for storing data and Matrix.h: specifies a matrix data structure in row-major order. It implements the following functions:
        - computeQRGivens: computes QR using one of approaches (thin or MKL) based on the passed hint. It can also evaluate other approaches that are not specified in the paper. 
        - computeGivensRotation: computes cos, sin and r of Givens rotation that would set entry b to zero if this Givens rotation is applied on two rows with entries a and b in the same column. 
        - applyGivens: applies Givens rotation in specified row and column. We assume all entries in both rows left of startColIdx are zero.
        - computeQRGivensSequentialBlockDiag apply sequence of Givens rotation sequentally to upper triangularize the matrix according to the Matrix computations book 
        - computeQRGivensParallelBlockDiag: parallelized version of computeQRGivensSequentialBlockDiag
        - computeQRGivensParallelizedThinMatrix: apply thin approach of computing R: splitting matrix in submatrices, upper-triangularazing each of the submatrices sequentially and then concatenating Rs and again applying computeQRGivensSequentialBlockDiag.
        - computeQRGivensParallelizedLaPack: compute R using MKL lapack API dgeqrf. We don't need to copy anything since the matrix is already stored in c-array like.
        - makeDiagonalElementsPositiveInR: multiplies R with ortogonal matrix so it sets entries on the diagonal to positive.
