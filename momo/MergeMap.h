/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/MergeMap.h

  namespace momo:
    class MergeMapKeyValueTraits
    class MergeMapSettings
    class MergeMap

\**********************************************************/

#pragma once

#include "MergeSet.h"
#include "MapUtility.h"

namespace momo
{

namespace internal
{
	template<typename TMergeSetPosition,
		bool tIsConst = false>
	class MergeMapPosition
	{
	protected:
		typedef TMergeSetPosition MergeSetPosition;

		static const bool isConst = tIsConst;

	public:
		//typedef MapForwardIterator<typename MergeSetPosition::Iterator, isConst> Iterator;

		//typedef typename Iterator::Reference Reference;
		//typedef typename Iterator::Pointer Pointer;

		typedef MapReference<decltype(*MergeSetPosition()), isConst> Reference;
		typedef IteratorPointer<Reference> Pointer;

		typedef MergeMapPosition<MergeSetPosition, true> ConstPosition;

	private:
		struct ReferenceProxy : public Reference
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(Reference)
		};

		struct ConstPositionProxy : public ConstPosition
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstPosition)
		};

		//struct IteratorProxy : public Iterator
		//{
		//	MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
		//	MOMO_DECLARE_PROXY_FUNCTION(Iterator, GetSetIterator)
		//};

	public:
		explicit MergeMapPosition() noexcept
			: mMergeSetPosition()
		{
		}

		//template<typename ArgIterator>
		//requires std::is_convertible_v<ArgIterator, Iterator>
		//MergeMapPosition(ArgIterator iter) noexcept
		//	: mMergeSetPosition(IteratorProxy::GetSetIterator(static_cast<Iterator>(iter)))
		//{
		//}

		operator ConstPosition() const noexcept
		{
			return ConstPositionProxy(mMergeSetPosition);
		}

		//template<typename ResIterator>
		//requires std::is_convertible_v<Iterator, ResIterator>
		//operator ResIterator() const noexcept
		//{
		//	Iterator iter = IteratorProxy(mMergeSetPosition);
		//	return static_cast<ResIterator>(iter);
		//}

		Pointer operator->() const
		{
			return Pointer(ReferenceProxy(*mMergeSetPosition));
		}

		friend bool operator==(MergeMapPosition pos1, MergeMapPosition pos2) noexcept
		{
			return pos1.mMergeSetPosition == pos2.mMergeSetPosition;
		}

		MOMO_MORE_POSITION_OPERATORS(MergeMapPosition)

	protected:
		explicit MergeMapPosition(MergeSetPosition mergeSetPos) noexcept
			: mMergeSetPosition(mergeSetPos)
		{
		}

		MergeSetPosition ptGetMergeSetPosition() const noexcept
		{
			return mMergeSetPosition;
		}

	private:
		MergeSetPosition mMergeSetPosition;
	};

	template<typename TKeyValueTraits>
	class MergeMapNestedSetItemTraits : public MapNestedSetItemTraits<TKeyValueTraits>
	{
	private:
		typedef internal::MapNestedSetItemTraits<TKeyValueTraits> MapNestedSetItemTraits;

	protected:
		using typename MapNestedSetItemTraits::KeyValueTraits;
		using typename MapNestedSetItemTraits::Value;

	public:
		using typename MapNestedSetItemTraits::Key;
		using typename MapNestedSetItemTraits::Item;
		using typename MapNestedSetItemTraits::MemManager;

	public:
		template<typename SrcIterator, typename DstIterator, typename ItemCreator>
		static void RelocateCreate(MemManager& memManager, SrcIterator srcBegin, DstIterator dstBegin,
			size_t count, ItemCreator&& itemCreator, Item* newItem)
		{
			auto srcKeyGen = [srcIter = srcBegin] () mutable
				{ return MapNestedSetItemTraits::ptGenerateKeyPtr(srcIter); };
			auto srcValueGen = [srcIter = srcBegin] () mutable
				{ return MapNestedSetItemTraits::ptGenerateValuePtr(srcIter); };
			auto dstKeyGen = [dstIter = dstBegin] () mutable
				{ return MapNestedSetItemTraits::ptGenerateKeyPtr(dstIter); };
			auto dstValueGen = [dstIter = dstBegin] () mutable
				{ return MapNestedSetItemTraits::ptGenerateValuePtr(dstIter); };
			auto func = [&itemCreator, newItem] ()
				{ std::forward<ItemCreator>(itemCreator)(newItem); };
			KeyValueTraits::RelocateExec(memManager,
				InputIterator(srcKeyGen), InputIterator(srcValueGen),
				InputIterator(dstKeyGen), InputIterator(dstValueGen), count, func);
		}
	};

	template<typename TMergeMapSettings>
	class MergeMapNestedSetSettings //: public MergeSetSettings
	{
	protected:
		typedef TMergeMapSettings MergeMapSettings;

	public:
		static const CheckMode checkMode = MergeMapSettings::checkMode;
		static const ExtraCheckMode extraCheckMode = MergeMapSettings::extraCheckMode;
		static const bool checkVersion = MergeMapSettings::checkVersion;
	};
}

template<conceptObject TKey, conceptObject TValue, conceptMemManager TMemManager>
class MergeMapKeyValueTraits : public internal::MapKeyValueTraits<TKey, TValue, TMemManager>
{
};

class MergeMapSettings
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
	static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION;
};

/*!
	All `MergeMap` functions and constructors have strong exception safety,
	but not the following cases:
	1. Functions `Insert` receiving many items have basic exception safety.
	2. In case default `KeyValueTraits`: if insert/add function receiving
	argument `Key&& key` throws exception, this argument may be changed.

	Swap and move operations invalidate all container iterators.
*/

template<conceptObject TKey, conceptObject TValue,
	typename TMergeTraits = MergeTraits<TKey>,
	conceptMemManager TMemManager = MemManagerDefault,
	typename TKeyValueTraits = MergeMapKeyValueTraits<TKey, TValue, TMemManager>,
	typename TSettings = MergeMapSettings>
requires conceptMergeTraits<TMergeTraits, TKey>
class MergeMap
{
public:
	typedef TKey Key;
	typedef TValue Value;
	typedef TMergeTraits MergeTraits;
	typedef TMemManager MemManager;
	typedef TKeyValueTraits KeyValueTraits;
	typedef TSettings Settings;

private:
	typedef internal::MergeMapNestedSetItemTraits<KeyValueTraits> MergeSetItemTraits;
	typedef typename MergeSetItemTraits::Item KeyValuePair;

	typedef internal::MergeMapNestedSetSettings<Settings> MergeSetSettings;

	typedef momo::MergeSet<Key, MergeTraits, MemManager, MergeSetItemTraits, MergeSetSettings> MergeSet;

	typedef typename MergeSet::ConstIterator MergeSetConstIterator;
	typedef typename MergeSet::ConstPosition MergeSetConstPosition;

public:
	typedef internal::MapForwardIterator<MergeSetConstIterator> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::MergeMapPosition<MergeSetConstPosition> Position;
	typedef typename Position::ConstPosition ConstPosition;

	typedef internal::InsertResult<Position> InsertResult;

private:
	typedef internal::MapValueReferencer<MergeMap, Position> ValueReferencer;

public:
	typedef typename ValueReferencer::template ValueReference<Key&&> ValueReferenceRKey;
	typedef typename ValueReferencer::template ValueReference<const Key&> ValueReferenceCKey;

private:
	template<typename... ValueArgs>
	using ValueCreator = typename KeyValueTraits::template ValueCreator<ValueArgs...>;

	struct ConstIteratorProxy : public ConstIterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
	};

	struct IteratorProxy : public Iterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
	};

	struct ConstPositionProxy : public ConstPosition
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstPosition)
		MOMO_DECLARE_PROXY_FUNCTION(ConstPosition, GetMergeSetPosition)
	};

	struct PositionProxy : public Position
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Position)
	};

public:
	MergeMap()
		: MergeMap(MergeTraits())
	{
	}

	explicit MergeMap(const MergeTraits& mergeTraits, MemManager memManager = MemManager())
		: mMergeSet(mergeTraits, std::move(memManager))
	{
	}

	template<typename Pair = std::pair<Key, Value>>
	MergeMap(std::initializer_list<Pair> pairs)
		: MergeMap(pairs, MergeTraits())
	{
	}

	template<typename Pair = std::pair<Key, Value>>
	explicit MergeMap(std::initializer_list<Pair> pairs, const MergeTraits& mergeTraits,
		MemManager memManager = MemManager())
		: MergeMap(mergeTraits, std::move(memManager))
	{
		Insert(pairs);
	}

	MergeMap(MergeMap&& mergeMap) noexcept
		: mMergeSet(std::move(mergeMap.mMergeSet))
	{
	}

	MergeMap(const MergeMap& mergeMap)
		: mMergeSet(mergeMap.mMergeSet)
	{
	}

	explicit MergeMap(const MergeMap& mergeMap, MemManager memManager)
		: mMergeSet(mergeMap.mMergeSet, std::move(memManager))
	{
	}

	~MergeMap() = default;

	MergeMap& operator=(MergeMap&& mergeMap) noexcept
	{
		MergeMap(std::move(mergeMap)).Swap(*this);
		return *this;
	}

	MergeMap& operator=(const MergeMap& mergeMap)
	{
		if (this != &mergeMap)
			MergeMap(mergeMap).Swap(*this);
		return *this;
	}

	void Swap(MergeMap& mergeMap) noexcept
	{
		mMergeSet.Swap(mergeMap.mMergeSet);
	}

	ConstIterator GetBegin() const noexcept
	{
		return ConstIteratorProxy(mMergeSet.GetBegin());
	}

	Iterator GetBegin() noexcept
	{
		return IteratorProxy(mMergeSet.GetBegin());
	}

	ConstIterator GetEnd() const noexcept
	{
		return ConstIterator();
	}

	Iterator GetEnd() noexcept
	{
		return Iterator();
	}

	MOMO_FRIEND_SWAP(MergeMap)
	MOMO_FRIENDS_SIZE_BEGIN_END(MergeMap)

	const MergeTraits& GetMergeTraits() const noexcept
	{
		return mMergeSet.GetMergeTraits();
	}

	const MemManager& GetMemManager() const noexcept
	{
		return mMergeSet.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mMergeSet.GetMemManager();
	}

	size_t GetCount() const noexcept
	{
		return mMergeSet.GetCount();
	}

	bool IsEmpty() const noexcept
	{
		return mMergeSet.IsEmpty();
	}

	void Clear() noexcept
	{
		mMergeSet.Clear();
	}

	ConstPosition Find(const Key& key) const
	{
		return ConstPositionProxy(mMergeSet.Find(key));
	}

	Position Find(const Key& key)
	{
		return PositionProxy(mMergeSet.Find(key));
	}

	bool ContainsKey(const Key& key) const
	{
		return mMergeSet.ContainsKey(key);
	}

	template<typename ValueCreator>
	requires std::invocable<ValueCreator&&, Value*>
	InsertResult InsertCrt(Key&& key, ValueCreator&& valueCreator)
	{
		return pvInsert(std::move(key), std::forward<ValueCreator>(valueCreator));
	}

	template<typename... ValueArgs>
	InsertResult InsertVar(Key&& key, ValueArgs&&... valueArgs)
	{
		return pvInsert(std::move(key),
			ValueCreator<ValueArgs...>(GetMemManager(), std::forward<ValueArgs>(valueArgs)...));
	}

	InsertResult Insert(Key&& key, Value&& value)
	{
		return InsertVar(std::move(key), std::move(value));
	}

	InsertResult Insert(Key&& key, const Value& value)
	{
		return InsertVar(std::move(key), value);
	}

	template<typename ValueCreator>
	requires std::invocable<ValueCreator&&, Value*>
	InsertResult InsertCrt(const Key& key, ValueCreator&& valueCreator)
	{
		return pvInsert(key, std::forward<ValueCreator>(valueCreator));
	}

	template<typename... ValueArgs>
	InsertResult InsertVar(const Key& key, ValueArgs&&... valueArgs)
	{
		return pvInsert(key,
			ValueCreator<ValueArgs...>(GetMemManager(), std::forward<ValueArgs>(valueArgs)...));
	}

	InsertResult Insert(const Key& key, Value&& value)
	{
		return InsertVar(key, std::move(value));
	}

	InsertResult Insert(const Key& key, const Value& value)
	{
		return InsertVar(key, value);
	}

	template<internal::conceptInputIterator ArgIterator,
		typename = decltype(internal::MapPairConverter<ArgIterator>::Convert(*std::declval<ArgIterator>()))>
	size_t Insert(ArgIterator begin, ArgIterator end)
	{
		size_t count = 0;
		for (ArgIterator iter = begin; iter != end; ++iter)
		{
			auto pair = internal::MapPairConverter<ArgIterator>::Convert(*iter);
			typedef decltype(pair.first) KeyArg;
			typedef decltype(pair.second) ValueArg;
			static_assert((std::is_same_v<Key, std::decay_t<KeyArg>>));
			InsertResult res = InsertVar(std::forward<KeyArg>(pair.first),
				std::forward<ValueArg>(pair.second));
			count += res.inserted ? 1 : 0;
		}
		return count;
	}

	template<typename Pair = std::pair<Key, Value>>
	size_t Insert(std::initializer_list<Pair> pairs)
	{
		return Insert(pairs.begin(), pairs.end());
	}

	template<typename PairCreator,
		bool extraCheck = true>
	requires std::invocable<PairCreator&&, Key*, Value*>
	Position AddCrt(ConstPosition pos, PairCreator&& pairCreator)
	{
		auto itemCreator = [&pairCreator] (KeyValuePair* newItem)
		{
			std::forward<PairCreator>(pairCreator)(newItem->GetKeyPtr(), newItem->GetValuePtr());
		};
		return PositionProxy(mMergeSet.template AddCrt<decltype(itemCreator), extraCheck>(
			ConstPositionProxy::GetMergeSetPosition(pos), std::move(itemCreator)));
	}

	template<typename ValueCreator,
		bool extraCheck = true>
	requires std::invocable<ValueCreator&&, Value*>
	Position AddCrt(ConstPosition pos, Key&& key, ValueCreator&& valueCreator)
	{
		return pvAdd<extraCheck>(pos, std::move(key), std::forward<ValueCreator>(valueCreator));
	}

	template<typename... ValueArgs>
	Position AddVar(ConstPosition pos, Key&& key, ValueArgs&&... valueArgs)
	{
		return AddCrt(pos, std::move(key),
			ValueCreator<ValueArgs...>(GetMemManager(), std::forward<ValueArgs>(valueArgs)...));
	}

	Position Add(ConstPosition pos, Key&& key, Value&& value)
	{
		return AddVar(pos, std::move(key), std::move(value));
	}

	Position Add(ConstPosition pos, Key&& key, const Value& value)
	{
		return AddVar(pos, std::move(key), value);
	}

	template<typename ValueCreator,
		bool extraCheck = true>
	requires std::invocable<ValueCreator&&, Value*>
	Position AddCrt(ConstPosition pos, const Key& key, ValueCreator&& valueCreator)
	{
		return pvAdd<extraCheck>(pos, key, std::forward<ValueCreator>(valueCreator));
	}

	template<typename... ValueArgs>
	Position AddVar(ConstPosition pos, const Key& key, ValueArgs&&... valueArgs)
	{
		return AddCrt(pos, key,
			ValueCreator<ValueArgs...>(GetMemManager(), std::forward<ValueArgs>(valueArgs)...));
	}

	Position Add(ConstPosition pos, const Key& key, Value&& value)
	{
		return AddVar(pos, key, std::move(value));
	}

	Position Add(ConstPosition pos, const Key& key, const Value& value)
	{
		return AddVar(pos, key, value);
	}

	ValueReferenceRKey operator[](Key&& key)
	{
		Position pos = Find(static_cast<const Key&>(key));
		return !!pos ? ValueReferencer::template GetReference<Key&&>(*this, pos)
			: ValueReferencer::template GetReference<Key&&>(*this, pos, std::move(key));
	}

	ValueReferenceCKey operator[](const Key& key)
	{
		Position pos = Find(key);
		return !!pos ? ValueReferencer::template GetReference<const Key&>(*this, pos)
			: ValueReferencer::template GetReference<const Key&>(*this, pos, key);
	}

private:
	template<typename RKey, typename ValueCreator>
	InsertResult pvInsert(RKey&& key, ValueCreator&& valueCreator)
	{
		Position pos = Find(static_cast<const Key&>(key));
		if (!!pos)
			return { pos, false };
		pos = pvAdd<false>(pos, std::forward<RKey>(key),
			std::forward<ValueCreator>(valueCreator));
		return { pos, true };
	}

	template<bool extraCheck, typename RKey, typename ValueCreator>
	Position pvAdd(ConstPosition pos, RKey&& key, ValueCreator&& valueCreator)
	{
		auto itemCreator = [this, &key, &valueCreator] (KeyValuePair* newItem)
		{
			KeyValueTraits::Create(GetMemManager(), std::forward<RKey>(key),
				std::forward<ValueCreator>(valueCreator), newItem->GetKeyPtr(),
				newItem->GetValuePtr());
		};
		return PositionProxy(mMergeSet.template AddCrt<decltype(itemCreator), extraCheck>(
			ConstPositionProxy::GetMergeSetPosition(pos), std::move(itemCreator)));
	}

private:
	MergeSet mMergeSet;
};

} // namespace momo
