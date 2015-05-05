//
// Author: orenbm, jirsch
//

#include "Block.h"
#include "blockchain.h"
#include "hash.h"
#include <pthread.h>
#include <vector>
#include <queue>
#include <stdlib.h>
#include <time.h>

#define FAILURE -1

using std::vector;

class BlockChain
{

private:

    // blocks that are in the end of the longest chains
    vector<Block *> _longestChains;

    // blocks that are waiting to be attached
    vector<Block *> _pendingBlocks;

    Block *_genesis;
    int _chainSize;
    bool _inited;
    bool _closing;

    // will hold the Block numbers that aren't in use and are lower than _currentBlockNum
    vector <int> _vacantBlockNums;

    int _currentBlockNum;

    int getMinVacantNum();

    /*
     * returns a random block from one of the longest chains
     */
    Block *getRandomLongestChain();

    void *addBlock(void *args);

    /*
     * return true if the given block is in the end of one of the longest chains, false otherwise
     */
    bool isInLongestChain(Block *block);

    /*
     * hash a block's data, given it's predecessor's block num, it's data and data length
     */
    char *hashBlockData(int blockNum, int predecessorBlockNum, dataToHash, dataLength);

    /*
     * construct a new block with it's number and predecessor
     */
    Block *buildNewBlock();

    /*
     * return true if the given block is attached to the BlockChain, false otherwise
     */
    bool isAttached(int blockNum);


public:

    /*
     * constructor
     */
    BlockChain();

    int getChainSize() const;

    void init();

    int addBlock(char *data, int length);

    bool isInited() const
    {
        return _inited;
    }

    vector getPendingBlocks() const
    {
        return _pendingBlocks;
    }
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

int BlockChain::getMinVacantNum()
{
    int blockNum;
    if (!this->_vacantBlockNums.empty()) {

        // the min num will be lower than the current number
        int minVacantNum = _currentBlockNum;

        // finding the minimum vacant number
        for (vector<int>::iterator it = _vacantBlockNums.begin();
             it != _vacantBlockNums.end(); ++it) {
            if (*it < minVacantNum) {
                minVacantNum = *it;
            }
        }

        // assigning the minimum number to the new block
        blockNum = minVacantNum;

        // removing the minimum number from the vector
        for (vector<int>::iterator it = _vacantBlockNums.begin();
             it != _vacantBlockNums.end(); ++it) {
            if (*it == minVacantNum) {
                _vacantBlockNums.erase(it);
            }
        }
    }

    else {

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
    NewBlockData(Block *block, char *dataToHash) : _block(
            block), _dataLength(dataLength)
    {
        dataToHash = new char[_dataLength]; // todo: delete
        memcpy(_dataToHash, dataToHash, _dataLength); // todo: check if +1 is necessary
    }
} NewBlockData;

bool BlockChain::isAttached(int blockNum)
{
    bool isVacant = false;

    // looking for the given blockNum in the vector
    if (std::find(_vacantBlockNums.begin(), _vacantBlockNums.end(), blockNum) !=
        _vacantBlockNums.end()) {
        isVacant = true;
    }

    // will be true if the given blockNum is lower than the current one and is not vacant
    return blockNum <= _currentBlockNum && !isVacant;
}

bool BlockChain::isInLongestChain(Block *block)
{
    return block->getChainLength() == _longestChains[0]->getChainLength();
}

char *BlockChain::hashBlockData(int blockNum, int predecessorBlockNum, dataToHash, dataLength)
{
    // getting the nonce and hashing the data
    int nonce = generate_nonce(blockNum, predecessorBlockNum);

    return generate_hash(dataToHash, dataLength, nonce);
}

void *BlockChain::addBlock(void *args)
{
    NewBlockData *blockData = (NewBlockData *) args;

    // hashing the data
    char *hash = hashBlockData(blockData->_block->getBlockNum(),
                               blockData->_block->getPredecessor()->getBlockNum(),
                               blockData->_dataToHash, blockData->_dataLength);

    // checking if the block's predecessor should be switched to the real-time longest chain
    if (blockData->_block->isToLongest() &&
        !isInLongestChain(blockData->_block->getPredecessor())) {

        // todo: block other threads

        // find a new predecessor
        Block *predecessor = getRandomLongestChain();

        // rehash with the new predecessor
        hash = hashBlockData(blockData->_block->getBlockNum(), predecessor->getBlockNum(),
                             blockData->_dataToHash, blockData->_dataLength);
    }

    // attaching the block to the tree:
    // setting the new block's hashed data
    blockData->_block->setHashedData(hash);

    // adding the new block to it's predecessor's successors list
    blockData->_block->getPredecessor()->addSuccessor(block);

    if (isInLongestChain(blockData->_block)) {
        _longestChains.push_back(blockData->_block);
    }

    else if (blockData->_block->getChainLength() > _longestChains[0]->getChainLength()) {
        _longestChains.clear();
        _longestChains.push_back(blockData->_block);
    }

    // todo: removing the block from the pending vector

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
    // constructing a new block
    Block *block = buildNewBlock();

    // constructing the struct that wraps the block and its dataToHash
    NewBlockData *newBlockData = new NewBlockData(block, data, length);    // todo: delete

    // pushing the new block to the pending blocks' vector
    _pendingBlocks.push_back(newBlockData);

    // in another thread- hash data, attach new block to predecessor, insert new block
    // into predecessor's successors
    pthread_t thread;
    int result = pthread_create(&thread, NULL, addBlock, newBlockData);

    // if the creation of the thread failed return FAILURE
    if (result != 0) {
        return FAILURE;
    }

    return newBlockData->_blockNum;
}

void BlockChain::init()
{
    _genesis = new Block(0, NULL); // TODO: delete
    _longestChains.push_back(_genesis);
    _chainSize = 0;
    _currentBlockNum = 0;
    _vacantBlockNums = vector<int>();
    _inited = true;
    _closing = false;
}

// ****************
// End of class implementation
// Start of library
// ****************

BlockChain *gChain;

int init_blockchain()
{
    if (gChain == NULL) {
        gChain = new BlockChain(); // todo: delete
    }

    if (gChain->isInited()) {
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
    // checking if the block is waiting to be attached
    for (vector<Block *>::iterator it = gChain->getPendingBlocks().begin();
         it != gChain->getPendingBlocks().end(); ++it) {

        Block *pendingBlock = (Block *) *it;

        if (pendingBlock->getBlockNum() == block_num) {
            pendingBlock->setToLongest(true);
            return 0;
        }
    }

    // checking if the block is already attached
    if (gChain->isAttached(block_num)) {
        return 1;
    }

    // the blockNum does not exist - todo: verify
    return -2;
}

int attach_now(int block_num);

int was_added(int block_num);

int chain_size();

int prune_chain();

void close_chain();

int return_on_close();

