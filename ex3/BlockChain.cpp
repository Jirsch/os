//
// Created by orenbm21 on 5/2/2015.
//

#include "Block.h"
#include "blockchain.h"
#include "hash.h"
#include <pthread.h>

#define FAILURE -1


class BlockChain
{

private:
    Block *_endOfLongest;
    Block *_genesis;
    int _chainSize;
    bool _inited;
    bool _closing;


    // will hold the Block numbers that aren't in use
    priority_queue <int, std::vector<int>, std::greater<int>> _VacantBlockNums;
    int _currentBlockNum;

    int getMinVacantNum();

    void *addBlock(void *args);

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
        return _isInited;
    }
};

int BlockChain::getChainSize() const
{
    return _chainSize;
}

BlockChain::BlockChain()
{
    init();
}

int BlockChain::getMinVacantNum()
{
    int blockNum;
    if (!this->_VacantBlockNums.empty()) {
        blockNum = this->_VacantBlockNums.top();
        this->_VacantBlockNums.pop();
    }

    else {
        blockNum = ++this->_currentBlockNum; // verify that this works
    }
    return blockNum;
}

typedef struct NewBlockData
{
    Block *_predecessor;
    int _blockNum;
    char *_dataToHash;
    int _dataLength;

    NewBlockData(Block *predecessor, int blockNum, char *dataToHash, int dataLength) : _predecessor(
            predecessor), _blockNum(blockNum), _dataLength(dataLength)
    {
        newBlock.dataToHash = new char[_dataLength]; // todo: delete
        memcpy(_dataToHash, dataToHash, _dataLength); // todo: check if +1 is necessary
    }

} NewBlockData;

void *BlockChain::addBlock(void *args)
{
    NewBlockData *data = (NewBlockData *) args;

    // constructing a new block
    Block *block = new Block(data->_blockNum, data->_predecessor); // todo: delete

    // getting the nonce and hashing the data
    int nonce = generate_nonce(data->_blockNum, data->_predecessor->getBlockNum());
    char *hash = generate_hash(data->_dataToHash, data->_dataLength, nonce);

    // setting the new block's hashed data
    block->setHashedData(hash);

    // adding the new block to it's predecessor's successors list
    block->getPredecessor()->addSuccessor(block);

    // updating the _endOfLongest block
//    if (block->getChainLength() > _endOfLongest->getChainLength())
//    {
//        _endOfLongest = block;
//    }

    if (block->getPredecessor() == _endOfLongest)
    {
        _endOfLongest = block;
    }
}

int BlockChain::addBlock(char *data, int length)
{
    NewBlockData *newBlockData = new NewBlockData(_endOfLongest, getMinVacantNum(), data, length);

    // in other thread- create block, hash data, attach new block to predecessor, insert new block
    // into predecessor's successors
    pthread thread;
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
    _endOfLongest = _genesis;
    _chainSize = 0;
    _currentBlockNum = 0;
    _VacantBlockNums = priority_queue<int, std::vector<int>(), std::greater<int>()>();
    _isInited = true;
    _isClosing = false;
}

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
    return gChain->addBlock();
}

int to_longest(int block_num);

int attach_now(int block_num);

int was_added(int block_num);

int chain_size();

int prune_chain();

void close_chain();

int return_on_close();

