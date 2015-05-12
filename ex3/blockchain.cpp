//
// Author: orenbm21, jirsch
//

#include "blockchain.h"

using std::vector;
using std::list;


BlockChainManager *gChainManager=NULL;

int init_blockchain()
{
    if (gChainManager == NULL)
    {
        gChainManager = new BlockChainManager();
    }

    return gChainManager->init();
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
    gChainManager = NULL;
    return res;
}

