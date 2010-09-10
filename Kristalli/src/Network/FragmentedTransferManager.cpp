#include "clb/Network/FragmentedTransferManager.h"
#include "clb/Network/MessageConnection.h"

void FragmentedSendManager::FragmentedTransfer::AddMessage(NetworkMessage *message)
{
	fragments.push_back(message);
	message->transfer = this;
}

FragmentedSendManager::FragmentedTransfer *FragmentedSendManager::AllocateNewFragmentedTransfer()
{
//	LOG("Allocated new fragmented transfer.");

	transfers.push_back(FragmentedTransfer());
	FragmentedTransfer *transfer = &transfers.back();
	transfer->id = -1;
	transfer->totalNumFragments = 0;
	return transfer;
}

void FragmentedSendManager::FreeFragmentedTransfer(FragmentedTransfer *transfer)
{
//	LOG("Freeing fragmented transfer ID=%d, numFragments: %d.", transfer->id, transfer->totalNumFragments);

	for(TransferList::iterator iter = transfers.begin(); iter != transfers.end(); ++iter)
		if (&*iter == transfer)
		{
			transfers.erase(iter);
			return;
		}
	LOG("Trying to free a fragmented send struct that didn't exist!");
}

void FragmentedSendManager::RemoveMessage(FragmentedTransfer *transfer, NetworkMessage *message)
{
//	LOG("Removing message with seqnum %d (fragnum %d) from transfer ID %d.", message->messageNumber, message->fragmentIndex, transfer->id);

	for(std::list<NetworkMessage*>::iterator iter = transfer->fragments.begin(); iter != transfer->fragments.end();)
	{
		std::list<NetworkMessage*>::iterator next = iter;
		++next;
		if ((*iter) == message)
		{
			transfer->fragments.erase(iter);
			if (transfer->fragments.size() == 0)
				FreeFragmentedTransfer(transfer);
			return;
		}
		iter = next;
	}
	LOG("Trying to remove a nonexisting message from a fragmented send struct!");
}

bool FragmentedSendManager::AllocateFragmentedTransferID(FragmentedTransfer &transfer)
{
	assert(transfer.id == -1);

	int transferID = 0;
	///\todo Maintain a sorted order in Insert() instead of doing a search here - better for performance.
	bool used = true;
	while(used)
	{
		used = false;
		for(TransferList::iterator iter = transfers.begin(); iter != transfers.end(); ++iter)
		{
			if (iter->id == transferID)
			{
				++transferID;
				used = true;
			}
		}
	}
	if (transferID >= 256)
		return false;
	transfer.id = transferID;
//	LOG("Allocated a transferID %d to a transfer of %d fragments.", transfer.id, transfer.totalNumFragments);

	return true;
}

void FragmentedReceiveManager::NewFragmentStartReceived(int transferID, int numTotalFragments, const char *data, size_t numBytes)
{
//	LOG("Received a fragmentStart of size %db (#total fragments %d) for a transfer with ID %d.", numBytes, numTotalFragments, transferID);

	if (numBytes == 0)
	{
		LOG("Discarding fragmentStart of size 0!");
		return;
	}

	for(size_t i = 0; i < transfers.size(); ++i)
		if (transfers[i].transferID == transferID)
		{
			LOG("An existing transfer with ID %d existed! Deleting it.", transferID);
			transfers.erase(transfers.begin() + i);
			--i;
		}

	transfers.push_back(ReceiveTransfer());
	ReceiveTransfer &transfer = transfers.back();
	transfer.transferID = transferID;
	transfer.numTotalFragments = numTotalFragments;

	///\todo Can optimize by passing the pre-searched transfer struct.
	NewFragmentReceived(transferID, 0, data, numBytes);
}

bool FragmentedReceiveManager::NewFragmentReceived(int transferID, int fragmentNumber, const char *data, size_t numBytes)
{
//	LOG("Received a fragment of size %db (index %d) for a transfer with ID %d.", numBytes, fragmentNumber, transferID);

	if (numBytes == 0)
	{
		LOG("Discarding fragment of size 0!");
		return false;
	}

	for(size_t i = 0; i < transfers.size(); ++i)
		if (transfers[i].transferID == transferID)
		{
			ReceiveTransfer &transfer = transfers[i];

			for(size_t j = 0; j < transfer.fragments.size(); ++j)
				if (transfer.fragments[j].fragmentIndex == fragmentNumber)
				{
					LOG("A fragment with fragmentNumber %d already exists for transferID %d. Discarding the new fragment! Old size: %db, discarded size: %db",
						fragmentNumber, transferID, transfer.fragments[j].data.size(), numBytes);
					return false;
				}

			transfer.fragments.push_back(ReceiveFragment());
			ReceiveFragment &fragment = transfer.fragments.back();
			fragment.fragmentIndex = fragmentNumber;
			fragment.data.insert(fragment.data.end(), data, data + numBytes);

			if (transfer.fragments.size() >= (size_t)transfer.numTotalFragments)
			{
//				LOG("Finished receiving a fragmented transfer that consisted of %d fragments (transferID=%d).",
//					transfer.fragments.size(), transfer.transferID);
				return true;
			}
			else
				return false;
		}
	LOG("Received a fragment of size %db (index %d) for a transfer with ID %d, but that transfer had not been initiated!",
		numBytes, fragmentNumber, transferID);
	return false;
}

void FragmentedReceiveManager::AssembleMessage(int transferID, std::vector<char> &assembledData)
{
	for(size_t i = 0; i < transfers.size(); ++i)
		if (transfers[i].transferID == transferID)
		{
			ReceiveTransfer &transfer = transfers[i];
			size_t totalSize = 0;

			for(size_t j = 0; j < transfer.fragments.size(); ++j)
				totalSize += transfer.fragments[j].data.size();

			assembledData.resize(totalSize);

			///\todo Sort by fragmentIndex.
			
			size_t offset = 0;
			for(size_t j = 0; j < transfer.fragments.size(); ++j)
			{
				assert(transfer.fragments[j].data.size() > 0);
				memcpy(&assembledData[offset], &transfer.fragments[j].data[0], transfer.fragments[j].data.size());
				offset += transfer.fragments[j].data.size();
				assert(offset <= assembledData.size());
			}
		}
}

void FragmentedReceiveManager::FreeMessage(int transferID)
{
	for(size_t i = 0; i < transfers.size(); ++i)
		if (transfers[i].transferID == transferID)
		{
			transfers.erase(transfers.begin() + i);
			return;
		}
}