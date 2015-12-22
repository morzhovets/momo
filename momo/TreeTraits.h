/**********************************************************\

  momo/TreeTraits.h

  namespace momo:
    class TreeTraits

\**********************************************************/

#pragma once

#include "details/TreeNode.h"

namespace momo
{

template<typename TKey>
class TreeTraits
{
public:
	typedef TKey Key;

	static const size_t nodeCapacity = 8;

public:
	TreeTraits() MOMO_NOEXCEPT
	{
	}

	bool Less(const Key& key1, const Key& key2) const
	{
		return key1 < key2;
	}
};

} // namespace momo
