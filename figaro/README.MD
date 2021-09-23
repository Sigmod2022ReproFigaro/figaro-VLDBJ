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
    - [Database.cpp](src/database/Database.cpp)
     TODO: add serious explanations
    - [Relation.cpp](src/database/Relation.cpp)
    TODO: add serious explanations
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
    TODO: add functions and explanations

## Code example running
5. Position to Figaro algorithm implementation.
6. Run experiments: ./setup.sh --test_mode=UNIT_TEST --root_path=/home/user/zivanovic/Figaro/figaro-code/figaro
