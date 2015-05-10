//
// Created by orenbm21 on 5/10/2015.
//

#ifndef EX3_BLOCKCHAINMANAGER_H
#define EX3_BLOCKCHAINMANAGER_H


class BlockChainManager
{
private:

    // blocks that are in the end of the longest chains
    vector<Block *> _longestChains;

    // blocks that are waiting to be attached
    list<NewBlockData *> _pendingBlocks;

    // the root of the block chain
    Block *_genesis;

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

    /*
     * the lock of the pending list
     */
    pthread_mutex_t _pendingLock;

    pthread_cond_t _pendingEmptyCond;

    /*
     * return true if the given blockNum is not in use
     */
    bool isVacant(int blockNum);

    /*
     * return the minimum vacant block number
     */
    int getMinVacantNum();

    /*
     * checks if the given blockNum is in the pending list. if so, return the Block. else, return
     * NULL
     */
    NewBlockData *isPending(int blockNum);

    /*
     * returns a random block from one of the longest chains
     */
    Block *getRandomLongestChain();

    /*
     * will process pending blocks while the chain isn't in a closing process
     */
    void *processBlocks(void *args);

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
     * initializes the chain and its members
     */
    void init();

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
     *
     */
    int wasAdded(int blockNum);

    int prune();

    void close();

    int returnOnClose();

    bool isClosing() const
    {
        return _closing;
    }

    void startCLosing()
    {
        _closing = true;
    }

    void finishClosing()
    {
        _finishedClosing = true;
    }

    bool finishedClosing()
    {
        return _finishedClosing;
    }

    list<NewBlockData *> *getPendingBlocks() const
    {
        return &_pendingBlocks;
    }
};


#endif //EX3_BLOCKCHAINMANAGER_H
