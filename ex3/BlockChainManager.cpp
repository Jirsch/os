//
// Created by orenbm21 on 5/10/2015.
//

#include "BlockChainManager.h"

int BlockChainManager::getChainSize() const
{
    int retVal;

    if (isInited() || isClosing())
    {
        retVal = _chainSize;
    }
    else
    {
        retVal = FAILURE;
    }

    return retVal;
}

BlockChainManager::BlockChainManager()
{
    srand(time(NULL));
}

int BlockChainManager::getMinVacantNum()
{
    if (pthread_mutex_lock(&_vacantLock) != SUCCESS)
    {
        exit(FAILURE);
    }

    int blockNum;

    if (!this->_vacantBlockNums.empty())
    {

        // the min num will be lower than the current number
        int minVacantNum = _currentBlockNum;

        vector<int>::iterator minIt;

        // finding the minimum vacant number
        for (vector<int>::iterator it = _vacantBlockNums.begin();
             it != _vacantBlockNums.end(); ++it)
        {

            if (*it < minVacantNum)
            {
                minVacantNum = *it;
                minIt = it;
            }
        }

        // assigning the minimum number to the new block
        blockNum = minVacantNum;
        _vacantBlockNums.erase(minIt);
    }
    else
    {
        // there are no vacant numbers lower than the current one
        blockNum = ++this->_currentBlockNum;
    }

    if (pthread_mutex_unlock(&_vacantLock) != SUCCESS)
    {
        exit(FAILURE);
    }

    return blockNum;
}


NewBlockData *BlockChainManager::isPending(int blockNum)
{
    NewBlockData *retVal = NULL;

    // looking for the given blockNum in the pending list
    for (list<NewBlockData *>::iterator it = getPendingBlocks()->begin();
         it != getPendingBlocks()->end(); ++it)
    {
        NewBlockData *pendingBlockData = (NewBlockData *) *it;

        if (pendingBlockData->_block->getBlockNum() == blockNum)
        {
            retVal = pendingBlockData;
            break;
        }
    }

    return retVal;
}

bool BlockChainManager::isVacant(int blockNum)
{
    bool isVacant = false;

    // looking for the given blockNum in the vacant blockNum vector
    if (std::find(_vacantBlockNums.begin(), _vacantBlockNums.end(), blockNum) !=
        _vacantBlockNums.end())
    {
        isVacant = true;
    }

    // will be true if the given blockNum is lower than the current one and is not vacant
    return blockNum > _currentBlockNum || isVacant;
}

bool BlockChainManager::isInLongestChain(Block *block)
{
    return block->getChainLength() == _longestChains[DEF_INDEX]->getChainLength();
}

char *BlockChainManager::hashBlockData(int blockNum, int predecessorBlockNum, char *dataToHash,
                                       int dataLength)
{
    // getting the nonce and hashing the data
    int nonce = generate_nonce(blockNum, predecessorBlockNum);

    return generate_hash(dataToHash, dataLength, nonce);
}

void BlockChainManager::reInitLongestChains(Block *longest)
{
    _longestChains.clear();
    _longestChains.push_back(longest);
}

void *BlockChainManager::processBlocks(void *args)
{
    BlockChainManager *chain = (BlockChainManager *) args;

    while (!chain->isClosing())
    {

        if (pthread_mutex_lock(&(chain->_pendingLock)) != SUCCESS)
        {
            exit(FAILURE);
        }

        if (chain->getPendingBlocks()->size() == 0)
        {
            if (pthread_cond_wait(&(chain->_pendingEmptyCond), &(chain->_pendingLock)) != SUCCESS)
            {
                exit(FAILURE);
            }
        }

        NewBlockData *blockData = chain->getPendingBlocks()->front();
        chain->getPendingBlocks()->pop_front();

        if (pthread_mutex_unlock(&(chain->_pendingLock)) != SUCCESS)
        {
            exit(FAILURE);
        }

        // checking if the block's predecessor should be switched to the real-time longest chain
        if ((blockData->_block->isToLongest() &&
             !chain->isInLongestChain(blockData->_block->getPredecessor()))
            || chain->isVacant(blockData->_block->getPredecessor()->getBlockNum()))
        {

            // find a new predecessor
            Block *predecessor = chain->getRandomLongestChain();

            blockData->_block->setPredecessor(predecessor);
        }

        // hashing the data
        char *hash = chain->hashBlockData(blockData->_block->getBlockNum(),
                                          blockData->_block->getPredecessor()->getBlockNum(),
                                          blockData->_dataToHash, blockData->_dataLength);

        // attaching the block to the tree:
        // setting the new block's hashed data
        blockData->_block->setHashedData(hash);

        // adding the new block to it's predecessor's successors list
        blockData->_block->getPredecessor()->addSuccessor(blockData->_block);

        // modifying the longestChains list
        if (chain->isInLongestChain(blockData->_block))
        {
            chain->_longestChains.push_back(blockData->_block);
        }

        else if (blockData->_block->getChainLength() >
                 chain->_longestChains[DEF_INDEX]->getChainLength())
        {
            chain->reInitLongestChains(blockData->_block);
        }

        chain->_chainSize++;
        delete blockData;
    }

    // after the closing flag was turned on
    chain->processClosing();

    pthread_exit(NULL);
}

Block *BlockChainManager::getRandomLongestChain()
{
    int blockIndex = rand() % _longestChains.size();

    return _longestChains[blockIndex];
}

Block *BlockChainManager::buildNewBlock()
{
    int blockNum = getMinVacantNum();

    // finding the predecessor of the new block
    Block *predecessor = getRandomLongestChain();

    // constructing a new block
    return new Block(blockNum, predecessor);
}

int BlockChainManager::addBlock(char *data, int length)
{
    if (!isInited() || getChainSize() == INT_MAX)
    {
        return FAILURE;
    }

    // constructing a new block
    Block *block = buildNewBlock();

    // constructing the struct that wraps the block and its dataToHash
    NewBlockData *newBlockData = new NewBlockData(block, data, length);

    // pushing the new block to the pending blocks' vector (with locking)
    pthread_mutex_lock(&_pendingLock);

    getPendingBlocks()->push_back(newBlockData);

    pthread_cond_signal(&_pendingEmptyCond);

    pthread_mutex_unlock(&_pendingLock);

    // returning the blockNum
    return newBlockData->_block->getBlockNum();
}

int BlockChainManager::init()
{
    if (isInited() || isClosing())
    {
        return FAILURE;
    }

    // creating the genesis block and pushing it to the longestChains list
    _genesis = new Block(0, NULL);
    _longestChains.push_back(_genesis);

    _chainSize = 0;
    _currentBlockNum = 0;

    _inited = true;
    _closing = false;

    // initializing the locks
    if (pthread_mutex_init(&_pendingLock, NULL) != SUCCESS ||
        pthread_cond_init(&_pendingEmptyCond, NULL) != SUCCESS ||
        pthread_mutex_init(&_vacantLock, NULL) != SUCCESS)
    {
        exit(FAILURE);
    }

    // creating the thread that will process blocks
    if (pthread_create(&_blockProcessor, NULL, &BlockChainManager::processBlocks, this) !=
        SUCCESS)
    {
        exit(FAILURE);
    }

    return SUCCESS;
}

int BlockChainManager::toLongest(int blockNum)
{
    int returnVal;

    if (!isInited())
    {
        return FAILURE;
    }

    if (pthread_mutex_lock(&_pendingLock)!=SUCCESS)
    {
        exit(FAILURE);
    }

    // checking if the block is already attached
    if (isVacant(blockNum))
    {
        returnVal = NOT_EXIST;
    }

    else
    {
        // the blockNum is already attached, unless in pending
        returnVal = ATTACHED;

        NewBlockData *pendingBlockData = isPending(blockNum);
        if (pendingBlockData != NULL)
        {
            pendingBlockData->_block->setToLongest(true);
            returnVal = SUCCESS;
        }
    }

    if (pthread_mutex_unlock(&_pendingLock)!=SUCCESS)
    {
        exit(FAILURE);
    }

    return returnVal;
}

int BlockChainManager::attachNow(int blockNum)
{
    int returnVal;

    if (!isInited())
    {
        return FAILURE;
    }

    if (pthread_mutex_lock(&_pendingLock)!=SUCCESS)
    {
        exit(FAILURE);
    }

    // checking if the block is already attached
    if (isVacant(blockNum))
    {
        returnVal = NOT_EXIST;
    }

    else
    {
        NewBlockData *pendingBlockData = isPending(blockNum);

        // not in pending -> already attached
        if (pendingBlockData != NULL)
        {
            // move blockData to front of pending queue
            getPendingBlocks()->remove(pendingBlockData);
            getPendingBlocks()->push_front(pendingBlockData);
        }

        returnVal = SUCCESS;
    }

    if (pthread_mutex_unlock(&_pendingLock)!=SUCCESS)
    {
        exit(FAILURE);
    }

    return returnVal;

}

int BlockChainManager::wasAdded(int blockNum)
{
    int returnVal;

    if (!isInited())
    {
        return FAILURE;
    }

    if (pthread_mutex_lock(&_pendingLock)!=SUCCESS)
    {
        exit(FAILURE);
    }

    // checking if the block is already attached
    if (isVacant(blockNum))
    {
        returnVal = NOT_EXIST;
    }

    else
    {
        if (isPending(blockNum) == NULL)
        {
            returnVal = TRUE;
        }

        else
        {
            returnVal = FALSE;
        }
    }

    if (pthread_mutex_unlock(&_pendingLock)!=SUCCESS)
    {
        exit(FAILURE);
    }

    return returnVal;
}

void BlockChainManager::addVacancy(int blockNum)
{
    _vacantBlockNums.push_back(blockNum);
}

void BlockChainManager::detach(Block *root)
{
    list<Block *>::iterator it = root->getSuccessors().begin();

    while (it != root->getSuccessors().end())
    {
        detach(*it);
        it = root->getSuccessors().erase(it);
    }

    addVacancy(root->getBlockNum());

    delete root;
}

void BlockChainManager::detachChildren(Block *root, Block *except)
{
    list<Block *>::iterator it = root->getSuccessors().begin();

    while (it != root->getSuccessors().end())
    {
        if ((*it) != except)
        {
            detach(*it);
            it = root->getSuccessors().erase(it);
        }
        else
        {
            ++it;
        }
    }
}

int BlockChainManager::prune()
{
    int retVal;

    // prevent new block from being attached to pruned parents
    if (pthread_mutex_lock(&_pendingLock)!=SUCCESS)
    {
        exit(FAILURE);
    }

    if (isInited() && !isClosing())
    {
        Block *desiredTail = getRandomLongestChain();

        Block *prev, *curr;
        curr = desiredTail;

        while (curr != _genesis)
        {
            prev = curr;
            curr = prev->getPredecessor();
            detachChildren(curr, prev);
        }

        reInitLongestChains(desiredTail);

        retVal = SUCCESS;
    }
    else
    {
        retVal = FAILURE;
    }

    if (pthread_mutex_unlock(&_pendingLock))
    {
        exit(FAILURE);
    }

    return retVal;
}

void BlockChainManager::processClosing()
{
    setIsInited(false);

    // printing the pending blocks and deleting them
    for (list<NewBlockData *>::iterator it = getPendingBlocks()->begin();
         it != getPendingBlocks()->end(); ++it)
    {
        NewBlockData *pendingBlock = (NewBlockData *) *it;

        // hashing the data
        char *hash = hashBlockData(pendingBlock->getBlock()->getBlockNum(),
                                   pendingBlock->getBlock()->getPredecessor()->getBlockNum(),
                                   pendingBlock->_dataToHash, pendingBlock->_dataLength);

        std::cout << hash << std::endl;

        delete hash;
        delete pendingBlock->_block;
        delete pendingBlock;
    }

    // deleting the chain
    detach(_genesis);

    _vacantBlockNums.clear();
    _longestChains.clear();
    _pendingBlocks.clear();

    // destroy locks
    if (pthread_mutex_destroy(&_pendingLock) != SUCCESS ||
        pthread_cond_destroy(&_pendingEmptyCond) != SUCCESS ||
        pthread_mutex_destroy(&_vacantLock) != SUCCESS)
    {
        exit(FAILURE);
    }

    // indicate that we finished closing the chain
    _closing = false;
}

void BlockChainManager::close()
{
    if (isInited())
    {
        startClosing();
    }
}

int BlockChainManager::returnOnClose()
{
    if (!isClosing())
    {
        return IS_NOT_CLOSING;
    }

    if (pthread_join(_blockProcessor, NULL) != SUCCESS)
    {
        exit(FAILURE);
    }

    return SUCCESS;
}
