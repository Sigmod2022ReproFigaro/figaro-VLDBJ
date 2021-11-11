Figaro system
==================

This directory contains the implementation of the Figaro system.
It contains the following files and directories:
- [CMakeLists.txt](CMakeLists.txt) specifies how the system is built; **Note**: LD_LIBRARY_PATH and LIBRARY_PATH in .bashrc were altered to include intel/mkl/2021.2.0/lib/intel64/ path.
- [setup.sh](setup.sh) defines some paths and runs the figaro system with the specified configuration. Run setup.sh --help for more details.
- [clean.sh](clean.sh) cleans the built files;
- src contains the main source files:
    - the file [main.cpp](src/main.cpp) constitutes the entry point to the Figaro system. It first loads the database configuration and the specified data. Then it loads the query. After that it evaluates the query and, if needed, dumps the computed matrix R.
    - [Utils.cpp](src/utils/Utils.cpp) contains functions for setting the number of threads, outputting relations to csv and computing the intersection of vectors.
    - the file [Database.cpp](src/database/Database.cpp) has several groups of functions for different needs: 
        - Database loading and initialization. This group allows loading database schemas (loadDatabaseRelationsSchema), loading data (loadData), and restoring relations to their initial state if computations change them in place (resetComputations)
        - Basic operations on relations. This group includes: sorting a specified relation based on specified attributes (sortRelation), removing specified attributes from the specified relation (dropAttributesFromRelations), updating the schema of a relation which include reordering of attributes and deletion (updateSchemaOfRelation), and one hot encoding all categorical attributes in all relations and then dropping the first column of one hot encoded attributes.
        - Figaro algorithm parts + post-processing that:
            - compute the necessary count queries (computeDownCounts, computeUpAndCircleCounts), 
            - evaluate parts of the figaro algorithm: computeHeadsAndTails, aggregateAwayChildrenRelations, 
            - apply the corresponding post-processing method: computePostprocessing. In particular this method upper-triangularizes the following three types of blocks: 
                - for tails of a relation it calls computeQROfTail;
                - for generalized tails that correspond to a join tree node in the bottom-up traversal of figaro it calls computeQROfGeneralizedTail;
                - for the remaining data when figaro terminates it calls computeQROfGeneralizedHead;
            After that it concatenates the upper-triangularized results and applies again an upper-triangularization method: computeQROfConcatenatedGeneralizedHeadAndTails
        - Applying a post-processing method to the join result (evalPostprocessing)
    - [Relation.cpp](src/database/Relation.cpp) implements the main logic of the operations of figaro on relations. Its functions can again be divided in several groups: 
        - basic modifying and accessing operations on the schema: getAttributeIdx, getAttributesIdxs, getAttributesIdxsComplement, getCategoricalAttributeIdxs, schemaJoins, schemaRemoveNonParJoinAttrs, schemaDropAttrs, schemaOneHotEncode
        - index building for parallel and faster evaluation of the figaro algorithm: getDistinctValsAndBuildIndices. In cases when the composite join attributes is a primary key we skip certain checks. This leads to significant speed-ups in index building.
        - basic modifiers on data + schema: loading and sorting data, dropping attributes and modifying the schema and the corresponding data 
        - count queries used in Figaro:
            - computeDownCounts. This implementation first builds indices over the data using getDistinctValsAndBuildIndices, which are used for parallelization of the algorithm. 
            - computeUpAndCircleCounts. It re-uses the the indices computed in computeDownCounts and parallelizes the computation of the up and circle count queries including the necessary synchronizations.
        - parts of the figaro algorithm that are implementation of Database.cpp functions: computeHeadsAndTails, aggregateAwayChildrenRelations, computeAndScaleGeneralizedHeadAndTail, computeQROfGeneralizedHead, computeQROfTail, computeQROfGeneralizedTail, computeQROfConcatenatedGeneralizedHeadAndTails
        - post-processing on the join result: computeQR 
    - the folder query contains code files for join order specification including the join tree data structure and join tree node types, the join tree visitors and query evaluators.
        - visitors:
            - [ASTJoinAttributesComputeVisitor.cpp](src/database/query/ASTJoinAttributesComputeVisitor.cpp). For a relation in a given join tree, this visitor computes its join attributes and the join attributes it has in common with its parent in the join tree. It is in charge of sorting a relation, changing the schema based on the query configuration, and one hot encoding categorical attributes.
            - [ASTComputeDownCountsVisitor.cpp](src/database/query/ASTComputeDownCountsVisitor.cpp) and [ASTComputeUpAndCircleCountsVisitor.cpp](src/database/query/ASTComputeUpAndCircleCountsVisitor.cpp) compute the count queries.
            - [ASTFigaroFirstPassVisitor.cpp](src/database/query/ASTFigaroFirstPassVisitor.cpp) computes heads and tails of a relation, grouped by the values of the join attributes.
            - [ASTFigaroSecondPassVisitor.cpp](src/database/query/ASTFigaroSecondPassVisitor.cpp) implements the bottom-up pass of figaro and upper-triangularizes the intermediate results.
            - [ASTPostProcQRVisitor.cpp](src/database/query/ASTPostProcQRVisitor.cpp) implements the post-processing on the join result.
        - join tree node type specifications:
            - [ASTNodeJoin.cpp](src/database/query/ASTNodeJoin.cpp) specifies a join tree node with children
            - ASTNodeRelation.cpp gives the relation that is associated with a join tree node.
            - ASTNodePostProcQR.cpp [ASTNodePostProcQR.cpp](src/database/query/ASTNodePostProcQR.cpp) as a root of the join tree specifies that post-processing is applied on the join result.
            - ASTNodeQRGivens.cpp [ASTNodeQRGivens.cpp](src/database/query/ASTNodeQRGivens.cpp) as a root of the join tree specifies that post-processing is applied on the factorized data represented in the join tree.
        - The query evaluator evaluates the query specified by the join tree. If the root is ASTNodePostProcQR it assumes there is only one more node in the join tree: a relational node that holds the result of a join.

            If the root is ASTNodeQRGivens it uses different visiting orders depending on the visitor.  ASTJoinAttributesComputeVisitor, ASTComputeDownCountsVisitor, ASTFigaroFirstPassVisitor, ASTFigaroSecondPassVisitor use postorder, while ASTComputeDownCountsVisitor uses preorder.
- the folder include stores header files for the corresponding source files. Additional files are inside utils (for error handling, logging and benchmarking) and database/storage. In particular, ArrayStorage.h specifies an inlined data structure that is used for storing data and Matrix.h specifies a matrix data structure in row-major order. It implements the following functions:
:
	- computeQRGivens: several approaches for computing the QR computation
        - computeGivensRotation: computes the parameters of a Givens rotation matrix
        - applyGivens: applies a Givens rotation for two specified rows and all columns to the right of a specified starting column. We assume that all entries in both rows left of the starting column are zero.
        - computeQRGivensSequentialBlockDiag: applying sequences of Givens rotation sequentially to upper-triangularize the matrix 
        - computeQRGivensParallelBlockDiag: parallelized version of computeQRGivensSequentialBlockDiag
        - computeQRGivensParallelizedThinMatrix: apply the "thin" approach of computing R: split matrix in blocks of rows, upper-triangularize each of the blocks in parallel and then concatenate the intermediate Rs and again apply computeQRGivensSequentialBlockDiag.
        - computeQRGivensParallelizedLaPack: compute R using MKL lapack API dgeqrf.
        - makeDiagonalElementsPositiveInR: changes the signs of the entries in some columns to make the diagonal entries positive.
- build: stores binary files;
- the folders dump and log have the same purpose as the folders with the same name in the root directory;
