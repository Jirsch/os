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

    /*
     * return the number of blocks from the genesis to the block
     */
    int getChainLength() const
    {
        return _chainLength;
    }

    /*
     * destructor
     */
    ~Block();

    /*
     * return the predecessor of the block
     */
    Block* getPredecessor();

    /*
     * set the predecessor to the given one
     */
    void setPredecessor(Block *predecessor);

    /*
     * add a successor to the list
     */
    void addSuccessor(Block* toAdd);

    /*
     * return the list of successors
     */
    list<Block *> &getSuccessors()
    {
        return _successors;
    }

    /*
     * set the flag the indicates if the block should be attached to the real-time longest chain
     */
    void setToLongest(bool toLongest)
    {
        _toLongest = toLongest;
    }

    /*
     * return the flag the indicates if the block should be attached to the real-time longest chain
     */
    bool isToLongest() const
    {
        return _toLongest;
    }

    /*
     * set the hashed data to the given one
     */
    void setHashedData(char *hashedData)
    {
        _hashedData = hashedData;
    }

    /*
     * return the blockNum
     */
    int getBlockNum() const
    {
        return _blockNum;
    }
};


#endif //OS_BLOCK_H
