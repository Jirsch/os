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
    int _chainLength;

    // indicates if the block should be attached to the real-time longest chain
    bool _toLongest;
public:

    /*
     * Constructor
     */
    Block(int blockNum, Block* predecessor);

    int getChainLength() const
    {
        return _chainLength;
    }

/*
     * deletes all the successors by calling their "delete" method
     * and deletes the current Block
     */
    ~Block();

    bool isGenesis();

    Block* getPredecessor();

    void setPredecessor(Block *predecessor);

    void addSuccessor(Block* toAdd);

    list<Block *> &getSuccessors()
    {
        return _successors;
    }

    void setToLongest(bool toLongest)
    {
        _toLongest = toLongest;
    }

    bool isToLongest() const
    {
        return _toLongest;
    }

    void setHashedData(char *hashedData)
    {
        _hashedData = hashedData;
    }

    int getBlockNum() const
    {
        return _blockNum;
    }
};


#endif //OS_BLOCK_H
