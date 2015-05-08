//
// Author: orenbm, jirsch
//

#include "Block.h"
#include "blockchain.h"
#include "hash.h"
#include <pthread.h>
#include <algorithm>
#include <vector>
#include <queue>
#include <stdlib.h>
#include <time.h>

#define FAILURE -1

using std::vector;
using std::list;

class BlockChain
{

    private:

    // blocks that are in the end of the longest chains
    vector<Block *> _longestChains;

    // blocks that are waiting to be attached
    list<NewBlockData *> _pendingBlocks;

    Block *_genesis;
    int _chainSize;
    bool _inited;
    bool _closing;

    // will hold the Block numbers that aren't in use and are lower than _currentBlockNum
    vector<int> _vacantBlockNums;

    int _currentBlockNum;

    pthread_mutex_t _pendingLock;
    pthread_cond_t _pendingEmptyCond;

    int getMinVacantNum();

    /*
     * returns a random block from one of the longest chains
     */
    Block *getRandomLongestChain();

    void *processBlocks(void *args);

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

    public:

    /*
     * constructor
     */
    BlockChain();

    int getChainSize() const;

    void init();

    bool isInited() const
    {
        return _inited;
    }

    int addBlock(char *data, int length);

    int toLongest(int blockNum);

    bool isClosing() const
    {
        return _closing;
    }

    list<NewBlockData *> *getPendingBlocks() const
    {
        return &_pendingBlocks;
    }

    /*
     * return true if the given block is attached to the BlockChain, false otherwise
     */
    bool isAttached(int blockNum);
};

int BlockChain::getChainSize() const
{
    return _chainSize;
}

BlockChain::BlockChain()
{
    srand(time(NULL));
    init();
}

// TODO: implement with one loop
// TODO: maybe keep sorted and move from O(N)->O(1)
int BlockChain::getMinVacantNum()
{
    int blockNum;
    if (!this->_vacantBlockNums.empty())
    {

        // the min num will be lower than the current number
        int minVacantNum = _currentBlockNum;

        // finding the minimum vacant number
        for (vector<int>::iterator it = _vacantBlockNums.begin();
             it != _vacantBlockNums.end(); ++it)
        {
            if (*it < minVacantNum)
            {
                minVacantNum = *it;
            }
        }

        // assigning the minimum number to the new block
        blockNum = minVacantNum;

        // removing the minimum number from the vector
        for (vector<int>::iterator it = _vacantBlockNums.begin();
             it != _vacantBlockNums.end(); ++it)
        {
            if (*it == minVacantNum)
            {
                _vacantBlockNums.erase(it);
            }
        }
    }

    else
    {

        // there are no vacant numbers lower than the current one
        blockNum = ++this->_currentBlockNum; // verify that this works
    }

    return blockNum;
}

/*
 * wrapping a new block with it's dataToHash
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
        dataToHash = new char[_dataLength];
        memcpy(_dataToHash, dataToHash, _dataLength); // todo: check if +1 is necessary
    }

    ~NewBlockData()
    {
        delete[] _dataToHash;
    }
} NewBlockData;

bool BlockChain::isAttached(int blockNum)
{
    bool isVacant = false;
    //TODO: lock?
    //TODO: not in pending!
    // looking for the given blockNum in the vector
    if (std::find(_vacantBlockNums.begin(), _vacantBlockNums.end(), blockNum) !=
        _vacantBlockNums.end())
    {
        isVacant = true;
    }

    // will be true if the given blockNum is lower than the current one and is not vacant
    return blockNum <= _currentBlockNum && !isVacant;
}

bool BlockChain::isInLongestChain(Block *block)
{
    return block->getChainLength() == _longestChains[0]->getChainLength();
}

char *BlockChain::hashBlockData(int blockNum, int predecessorBlockNum, char *dataToHash, int
dataLength)
{
    // getting the nonce and hashing the data
    int nonce = generate_nonce(blockNum, predecessorBlockNum);

    return generate_hash(dataToHash, dataLength, nonce);
}

void *BlockChain::processBlocks(void *args)
{
    // TODO: need to lock closing
    while (!isClosing())
    {
        pthread_mutex_lock(&_pendingLock);
        if (getPendingBlocks()->size() == 0)
        {
            pthread_cond_wait(&_pendingEmptyCond, &_pendingLock);
        }

        NewBlockData *blockData = getPendingBlocks()->front();
        getPendingBlocks()->pop_front();

        pthread_mutex_unlock(&_pendingLock);

        //TODO: make sure father exists

        // checking if the block's predecessor should be switched to the real-time longest chain
        if (blockData->_block->isToLongest() &&
            !isInLongestChain(blockData->_block->getPredecessor()))
        {

            // find a new predecessor
            Block *predecessor = getRandomLongestChain();

            blockData->_block->setPredecessor(predecessor);
        }

        // hashing the data
        char *hash = hashBlockData(blockData->_block->getBlockNum(),
                                   blockData->_block->getPredecessor()->getBlockNum(),
                                   blockData->_dataToHash, blockData->_dataLength);

        // attaching the block to the tree:
        // setting the new block's hashed data
        blockData->_block->setHashedData(hash);

        // adding the new block to it's predecessor's successors list
        blockData->_block->getPredecessor()->addSuccessor(blockData->_block);

        if (isInLongestChain(blockData->_block))
        {
            _longestChains.push_back(blockData->_block);
        }
        else if (blockData->_block->getChainLength() > _longestChains[0]->getChainLength())
        {
            _longestChains.clear();
            _longestChains.push_back(blockData->_block);
        }

        delete blockData;
    }

}

Block *BlockChain::getRandomLongestChain()
{
    int blockIndex = rand() % _longestChains.size();
    return _longestChains[blockIndex];
}

Block *BlockChain::buildNewBlock()
{
    int blockNum = getMinVacantNum();

    // finding the predecessor of the new block
    Block *predecessor = getRandomLongestChain();

    // constructing a new block
    return new Block(blockNum, predecessor); // todo: delete
}

int BlockChain::addBlock(char *data, int length)
{
    if (!isInited() || getChainSize() == INT_MAX)
    {
        return FAILURE;
    }

    // constructing a new block
    Block *block = buildNewBlock();

    // constructing the struct that wraps the block and its dataToHash
    NewBlockData *newBlockData = new NewBlockData(block, data, length);

    // pushing the new block to the pending blocks' vector

    pthread_mutex_lock(&_pendingLock);
    _pendingBlocks.push_back(newBlockData);
    pthread_cond_signal(&_pendingEmptyCond);
    pthread_mutex_unlock(&_pendingLock);

    return newBlockData->_block->getBlockNum();
}

void BlockChain::init()
{
    _genesis = new Block(0, NULL); // TODO: delete
    _longestChains.push_back(_genesis);
    _chainSize = 0;
    _currentBlockNum = 0;
    _vacantBlockNums = vector<int>();

    // TODO: error handle on return != 0
    pthread_mutex_init(&_pendingLock, NULL);
    // TODO: error handle on return != 0
    pthread_cond_init(&_pendingEmptyCond, NULL);

    _inited = true;
    _closing = false;

    // TODO: maybe make member?
    // TODO: check errors
    pthread_t blockProcessor;
    pthread_create(&blockProcessor, NULL, processBlocks, NULL);
}

int BlockChain::toLongest(int blockNum)
{
    int returnVal;

    //TODO: make sure blockNUm is valid
    if (isInited())
    {
        // checking if the block is already attached
        if (isAttached(blockNum))
        {
            returnVal = 1;
        }
        else
        {
            // the blockNum does not exist, unless in pending - todo: verify
            returnVal = -2;
        }

        pthread_mutex_lock(&_pendingLock);

        // checking if the block is waiting to be attached
        for (list<NewBlockData *>::iterator it = getPendingBlocks()->begin();
             it != getPendingBlocks()->end(); ++it)
        {
            NewBlockData *pendingBlockData = (NewBlockData *) *it;

            if (pendingBlockData->_block->getBlockNum() == blockNum)
            {
                pendingBlockData->_block->setToLongest(true);
                returnVal = 0;
                break;
            }
        }

        pthread_mutex_unlock(&_pendingLock); //TODO: ok?
    }
    else
    {
        returnVal = FAILURE;
    }

    return returnVal;
}

// ****************
// End of class implementation
// Start of library
// ****************

BlockChain *gChain;

int init_blockchain()
{
    if (gChain == NULL)
    {
        gChain = new BlockChain(); // todo: delete
    }

    if (gChain->isInited())
    {
        return -1;
    }

    gChain->init();

    return 0;
}

int add_block(char *data, int length)
{
    return gChain->addBlock(data, length);
}

int to_longest(int block_num)
{
    return gChain->toLongest(block_num);
}

int attach_now(int block_num);

int was_added(int block_num);

int chain_size();

int prune_chain();

void close_chain();

int return_on_close();

