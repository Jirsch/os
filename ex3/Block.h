//
// Created by orenbm21 on 5/2/2015.
//

#ifndef OS_BLOCK_H
#define OS_BLOCK_H

#include <list>

using std::list;

class Block {
private:
    char* _hashedData;
    Block* _predecessor;
    list<Block*> _successors;
    int _blockNum;
//    int nonce;
public:

    /*
     * Constructor
     */
    Block(int blockNum, Block* predecessor);

    /*
     * deletes all the successors by calling their "delete" method
     * and deletes the current Block
     */
    ~Block();

    Block* getPredecessor();


};


#endif //OS_BLOCK_H
