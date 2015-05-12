//
// Created by orenbm21 on 5/10/2015.
//

#ifndef EX3_BLOCKCHAINMANAGER_H
#define EX3_BLOCKCHAINMANAGER_H

#include "Block.h"
#include "hash.h"
#include <pthread.h>
#include <vector>
#include <algorithm>
#include <queue>
#include <string.h>
#include <iostream>


#define SUCCESS 0
#define FAILURE -1

#define NOT_EXIST -2
#define ATTACHED 1
#define FALSE 0
#define TRUE 1
#define IS_NOT_CLOSING -2
#define DEF_INDEX 0

using std::vector;

/*
 * wrapping a new block with its dataToHash
 */
typedef struct NewBlockData
{
    Block *_block;
    char *_dataToHash;
    int _dataLength;

    // constructor
    NewBlockData(Block *block, char *dataToHash, int dataLength) : _block(
            block), _dataLength(dataLength)
    {
        _dataToHash = new char[dataLength];
        memcpy(_dataToHash, dataToHash, _dataLength); // todo: check if +1 is necessary
    }

    Block* getBlock()
    {
        return _block;
    }



    // destructor
    ~NewBlockData()
    {
        delete[] _dataToHash;
        _block = NULL;
    }
} NewBlockData;

class BlockChainManager
{
private:

    // the root of the block chain
    Block *_genesis;

    // the number of blocks in the chain
    int _chainSize;

    // indicates if the chain is initialized
    bool _inited;

    // indicates if the chain is in a closing process
    bool _closing;

    // indicates if the chain has finished its closing process
    bool _finishedClosing;

    // will hold the Block numbers that aren't in use and are lower than _currentBlockNum
    vector<int> _vacantBlockNums;

    // the highest block number in use
    int _currentBlockNum;

    // blocks that are in the end of the longest chains
    vector<Block *> _longestChains;

    // blocks that are waiting to be attached
    list<NewBlockData *> _pendingBlocks;

    // the lock of the pending list
    pthread_mutex_t _pendingLock;

    // the condition variable of the pending list
    pthread_cond_t _pendingEmptyCond;

    // the thread that processes the pending blocks
    pthread_t _blockProcessor;


    /*
     * return true if the given blockNum is not in use
     */
    bool isVacant(int blockNum);

    /*
     * return the minimum vacant block number
     */
    int getMinVacantNum();

    /*
     * returns a random block from one of the longest chains
     */
    Block *getRandomLongestChain();

    /*
     * will process pending blocks while the chain isn't in a closing process
     */
    static void *processBlocks(void *args);

    /*
     * closes the chain
     */
    void processClosing();

    /*
     * return true if the given block is in the end of one of the longest chains, false otherwise
     */
    bool isInLongestChain(Block *block);

    /*
     * hash a block's data, given it's predecessor's block num, it's data and data length
     */
    char *hashBlockData(int blockNum, int predecessorBlockNum, char *dataToHash, int dataLength);

    /*
     * construct a new block with it's number and predecessor
     */
    Block *buildNewBlock();

    /*
     * detaches the tree that is rooted in the given block
     */
    void detach(Block *root);

    /*
     * detaches the tree that is rooted in the given block, except for the given child
     */
    void detachChildren(Block *root, Block *except);

    /*
     * add the given blockNum to the _vacantNum list
     */
    void addVacancy(int blockNum);

public:

    /*
     * constructor
     */
    BlockChainManager();

    /*
     * return the chain size
     */
    int getChainSize() const;

    /*
     * initializes the chain and its members. return 0 on success, -1 on failure
     */
    int init();

    /*
     * return true if the chain is initialized, false otherwise
     */
    bool isInited() const
    {
        return _inited;
    }

    /*
     * sets _isInited to the given status
     */
    void setIsInited(bool status)
    {
        _inited = status;
    }

    /*
     * checks if the given blockNum is in the pending list. if so, return the Block. else, return
     * NULL
     */
    NewBlockData* isPending(int blockNum);

    /*
     * add a block with the given data to the pending list.
     * return the block num in success, -1 in failure
     */
    int addBlock(char *data, int length);

    /*
     * the given blockNum should be attached to the real-time longest chain
     */
    int toLongest(int blockNum);

    /*
     * the given blockNum should be the next block to be attached to the chain. moves it to
     * the head of the pending list
     */
    int attachNow(int blockNum);

    /*
     * return 1 if the given blockNum was added, 0 if it is still pending and -2
     * if it does not exist
     */
    int wasAdded(int blockNum);

    /*
     * prune all the chains that are not the longest one.
     * return 0 on success, -1 otherwise
     */
    int prune();

    /*
     * start the closing process of the chain
     */
    void close();

    /*
     * return only when the chain finished its closing process
     * return 0 on success, -2 if the chain wasn't in a closing process, and -1 on failure
     */
    int returnOnClose();

    /*
     * return true if the chain is in a closing process, false otherwise
     */
    bool isClosing() const
    {
        return _closing;
    }

    /*
     * start a closing process
     */
    void startClosing()
    {
        _closing = true;
    }

    /*
     * indicate that the closing process is finished
     */
    void finishClosing()
    {
        _finishedClosing = true;
    }

    /*
     * initialize the longest Chains list
     */
    void initLongestChains(Block *longest);

    /*
     * return true if the chain has finished its closing process, false otherwise
     */
    bool finishedClosing()
    {
        return _finishedClosing;
    }

    /*
     * return the pending blocks list
     */
    list<NewBlockData *> *getPendingBlocks()
    {
        return &_pendingBlocks;
    }
};


#endif //EX3_BLOCKCHAINMANAGER_H
