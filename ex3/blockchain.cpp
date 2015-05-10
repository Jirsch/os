//
// Author: orenbm21, jirsch
//

#include "Block.h"
#include "blockchain.h"
#include "hash.h"
#include <pthread.h>
#include <vector>
#include <queue>

#define FAILURE -1

using std::vector;
using std::list;

//TODO: check lock return values
//TODO: make sure blockNUm is valid
//TODO: should isVacant lock?
//TODO: extract constants, specifically SUCCESS=0


BlockChainManager *gChainManager;

int init_blockchain()
{
    if (gChainManager == NULL)
    {
        gChainManager = new BlockChain();
    }

    if (gChainManager->isInited())
    {
        return FAILURE;
    }

    gChainManager->init();

    return 0;
}

int add_block(char *data, int length)
{
    return gChainManager->addBlock(data, length);
}

int to_longest(int block_num)
{
    return gChainManager->toLongest(block_num);
}

int attach_now(int block_num)
{
    return gChainManager->attachNow(block_num);
}

int was_added(int block_num)
{
    return gChainManager->wasAdded(block_num);
}

int chain_size()
{
    return gChainManager->getChainSize();
}

int prune_chain()
{
    return gChainManager->prune();
}

void close_chain()
{
	gChainManager->close();
}

int return_on_close()
{

	int res = gChainManager->returnOnClose();
    delete gChainManager;
    return res;
}

