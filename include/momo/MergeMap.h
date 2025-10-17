/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/MergeMap.h

  namespace momo:
    class MergeMapKeyValueTraits
    class MergeMapSettings
    class MergeMapCore
    class MergeMap
    class MergeMapHash

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
	class MergeMapPosition : public ForwardIteratorBase
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

template<conceptObject TKey, conceptObject TValue,
	conceptMemManager TMemManager = MemManagerDefault,
	bool tUseValuePtr = false>	//?
class MergeMapKeyValueTraits : public internal::MapKeyValueTraits<TKey, TValue, TMemManager, tUseValuePtr>
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
	All `MergeMapCore` functions and constructors have strong exception safety,
	but not the following cases:
	1. Functions `Insert` receiving many items have basic exception safety.
	2. In case default `KeyValueTraits`: if insert/add function receiving
	argument `Key&& key` throws exception, this argument may be changed.

	Swap and move operations invalidate all container iterators.
*/

template<typename TKeyValueTraits,
	conceptMergeTraits<typename TKeyValueTraits::Key> TMergeTraits
		= MergeTraits<typename TKeyValueTraits::Key>,
	typename TSettings = MergeMapSettings>
requires conceptMapKeyValueTraits<TKeyValueTraits, typename TKeyValueTraits::Key,
	typename TKeyValueTraits::Value, typename TKeyValueTraits::MemManager>
class MOMO_EMPTY_BASES MergeMapCore
	: public internal::Rangeable,
	public internal::Swappable<MergeMapCore>
{
public:
	typedef TKeyValueTraits KeyValueTraits;
	typedef TMergeTraits MergeTraits;
	typedef TSettings Settings;
	typedef typename KeyValueTraits::Key Key;
	typedef typename KeyValueTraits::Value Value;
	typedef typename KeyValueTraits::MemManager MemManager;

private:
	typedef internal::MergeMapNestedSetItemTraits<KeyValueTraits> MergeSetItemTraits;
	typedef typename MergeSetItemTraits::Item KeyValuePair;

	typedef internal::MergeMapNestedSetSettings<Settings> MergeSetSettings;

	typedef MergeSetCore<MergeSetItemTraits, MergeTraits, MergeSetSettings> MergeSet;

	typedef typename MergeSet::ConstIterator MergeSetConstIterator;
	typedef typename MergeSet::ConstPosition MergeSetConstPosition;

public:
	typedef internal::MapForwardIterator<MergeSetConstIterator> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::MergeMapPosition<MergeSetConstPosition> Position;
	typedef typename Position::ConstPosition ConstPosition;

	typedef internal::InsertResult<Position> InsertResult;

private:
	typedef internal::MapValueReferencer<MergeMapCore, Position> ValueReferencer;

public:
	template<typename KeyReference>
	using ValueReference = ValueReferencer::template ValueReference<KeyReference>;

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
	MergeMapCore()
		: MergeMapCore(MergeTraits())
	{
	}

	explicit MergeMapCore(const MergeTraits& mergeTraits, MemManager memManager = MemManager())
		: mMergeSet(mergeTraits, std::move(memManager))
	{
	}

	template<internal::conceptMapArgIterator<Key> ArgIterator,
		internal::conceptSentinel<ArgIterator> ArgSentinel>
	explicit MergeMapCore(ArgIterator begin, ArgSentinel end,
		const MergeTraits& mergeTraits = MergeTraits(), MemManager memManager = MemManager())
		: MergeMapCore(mergeTraits, std::move(memManager))
	{
		Insert(std::move(begin), std::move(end));
	}

	template<typename Pair = std::pair<Key, Value>>
	MergeMapCore(std::initializer_list<Pair> pairs)
		: MergeMapCore(pairs, MergeTraits())
	{
	}

	template<typename Pair = std::pair<Key, Value>>
	explicit MergeMapCore(std::initializer_list<Pair> pairs, const MergeTraits& mergeTraits,
		MemManager memManager = MemManager())
		: MergeMapCore(pairs.begin(), pairs.end(), mergeTraits, std::move(memManager))
	{
	}

	MergeMapCore(MergeMapCore&& mergeMap) noexcept
		: mMergeSet(std::move(mergeMap.mMergeSet))
	{
	}

	MergeMapCore(const MergeMapCore& mergeMap)
		: mMergeSet(mergeMap.mMergeSet)
	{
	}

	explicit MergeMapCore(const MergeMapCore& mergeMap, MemManager memManager)
		: mMergeSet(mergeMap.mMergeSet, std::move(memManager))
	{
	}

	~MergeMapCore() noexcept = default;

	MergeMapCore& operator=(MergeMapCore&& mergeMap) noexcept
	{
		return internal::ContainerAssigner::Move(std::move(mergeMap), *this);
	}

	MergeMapCore& operator=(const MergeMapCore& mergeMap)
	{
		return internal::ContainerAssigner::Copy(mergeMap, *this);
	}

	void Swap(MergeMapCore& mergeMap) noexcept
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

	template<internal::conceptObjectCreator<Value> ValueCreator>
	InsertResult InsertCrt(Key&& key, ValueCreator valueCreator)
	{
		return pvInsert(std::move(key), FastMovableFunctor(std::forward<ValueCreator>(valueCreator)));
	}

	template<typename... ValueArgs>
	requires requires { typename ValueCreator<ValueArgs...>; }
	InsertResult InsertVar(Key&& key, ValueArgs&&... valueArgs)
	{
		return InsertCrt(std::move(key),
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

	template<internal::conceptObjectCreator<Value> ValueCreator>
	InsertResult InsertCrt(const Key& key, ValueCreator valueCreator)
	{
		return pvInsert(key, FastMovableFunctor(std::forward<ValueCreator>(valueCreator)));
	}

	template<typename... ValueArgs>
	requires requires { typename ValueCreator<ValueArgs...>; }
	InsertResult InsertVar(const Key& key, ValueArgs&&... valueArgs)
	{
		return InsertCrt(key,
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

	template<internal::conceptMapArgIterator<Key> ArgIterator,
		internal::conceptSentinel<ArgIterator> ArgSentinel>
	size_t Insert(ArgIterator begin, ArgSentinel end)
	{
		size_t initCount = GetCount();
		for (ArgIterator iter = std::move(begin); iter != end; ++iter)
		{
			auto pair = internal::MapArgReferencer<>::GetReferencePair(iter);
			InsertVar(std::forward<decltype(pair.first)>(pair.first),
				std::forward<decltype(pair.second)>(pair.second));
		}
		return GetCount() - initCount;
	}

	template<typename Pair = std::pair<Key, Value>>
	size_t Insert(std::initializer_list<Pair> pairs)
	{
		return Insert(pairs.begin(), pairs.end());
	}

	template<internal::conceptMapPairCreator<Key, Value> PairCreator,
		bool extraCheck = true>
	Position AddCrt(ConstPosition pos, PairCreator pairCreator)
	{
		return pvAdd<extraCheck>(pos, FastMovableFunctor(std::forward<PairCreator>(pairCreator)));
	}

	template<internal::conceptObjectCreator<Value> ValueCreator,
		bool extraCheck = true>
	Position AddCrt(ConstPosition pos, Key&& key, ValueCreator valueCreator)
	{
		return pvAdd<extraCheck>(pos, std::move(key),
			FastMovableFunctor(std::forward<ValueCreator>(valueCreator)));
	}

	template<typename... ValueArgs>
	requires requires { typename ValueCreator<ValueArgs...>; }
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

	template<internal::conceptObjectCreator<Value> ValueCreator,
		bool extraCheck = true>
	Position AddCrt(ConstPosition pos, const Key& key, ValueCreator valueCreator)
	{
		return pvAdd<extraCheck>(pos, key,
			FastMovableFunctor(std::forward<ValueCreator>(valueCreator)));
	}

	template<typename... ValueArgs>
	requires requires { typename ValueCreator<ValueArgs...>; }
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

	template<typename ValueArg>
	requires requires { typename ValueCreator<ValueArg>; }
	InsertResult InsertOrAssign(Key&& key, ValueArg&& valueArg)
	{
		return pvInsertOrAssign(std::move(key), std::forward<ValueArg>(valueArg));
	}

	template<typename ValueArg>
	requires requires { typename ValueCreator<ValueArg>; }
	InsertResult InsertOrAssign(const Key& key, ValueArg&& valueArg)
	{
		return pvInsertOrAssign(key, std::forward<ValueArg>(valueArg));
	}

	ValueReference<Key&&> operator[](Key&& key)
	{
		Position pos = Find(std::as_const(key));
		return !!pos ? ValueReferencer::template GetReference<Key&&>(*this, pos)
			: ValueReferencer::template GetReference<Key&&>(*this, pos, std::move(key));
	}

	ValueReference<const Key&> operator[](const Key& key)
	{
		Position pos = Find(key);
		return !!pos ? ValueReferencer::template GetReference<const Key&>(*this, pos)
			: ValueReferencer::template GetReference<const Key&>(*this, pos, key);
	}

private:
	template<typename RKey, internal::conceptObjectCreator<Value> ValueCreator>
	InsertResult pvInsert(RKey&& key, FastMovableFunctor<ValueCreator> valueCreator)
	{
		auto itemCreator = [this, &key, valueCreator = std::move(valueCreator)]
			(KeyValuePair* newItem) mutable
		{
			KeyValuePair::template Create<KeyValueTraits>(newItem, mMergeSet.GetMemManager(),
				std::forward<RKey>(key), std::move(valueCreator));
		};
		typename MergeSet::InsertResult res = mMergeSet.template InsertCrt<decltype(itemCreator), false>(
			std::as_const(key), std::move(itemCreator));
		return { PositionProxy(res.position), res.inserted };
	}

	template<bool extraCheck, internal::conceptMapPairCreator<Key, Value> PairCreator>
	Position pvAdd(ConstPosition pos, FastMovableFunctor<PairCreator> pairCreator)
	{
		auto itemCreator = [this, pairCreator = std::move(pairCreator)] (KeyValuePair* newItem) mutable
			{ std::construct_at(newItem, mMergeSet.GetMemManager(), std::move(pairCreator)); };
		return PositionProxy(mMergeSet.template AddCrt<decltype(itemCreator), extraCheck>(
			ConstPositionProxy::GetHashSetPosition(pos), std::move(itemCreator)));
	}

	template<bool extraCheck, typename RKey, internal::conceptObjectCreator<Value> ValueCreator>
	Position pvAdd(ConstPosition pos, RKey&& key, FastMovableFunctor<ValueCreator> valueCreator)
	{
		auto itemCreator = [this, &key, valueCreator = std::move(valueCreator)]
			(KeyValuePair* newItem) mutable
		{
			KeyValuePair::template Create<KeyValueTraits>(newItem, mMergeSet.GetMemManager(),
				std::forward<RKey>(key), std::move(valueCreator));
		};
		return PositionProxy(mMergeSet.template AddCrt<decltype(itemCreator), extraCheck>(
			ConstPositionProxy::GetMergeSetPosition(pos), std::move(itemCreator)));
	}

	template<typename RKey, typename ValueArg>
	InsertResult pvInsertOrAssign(RKey&& key, ValueArg&& valueArg)
	{
		MemManager& memManager = GetMemManager();
		InsertResult res = pvInsert(std::forward<RKey>(key),
			FastMovableFunctor(ValueCreator<ValueArg>(memManager, std::forward<ValueArg>(valueArg))));
		if (!res.inserted)
			KeyValueTraits::AssignValue(memManager, std::forward<ValueArg>(valueArg), res.position->value);
		return res;
	}

private:
	MergeSet mMergeSet;
};

template<conceptObject TKey, conceptObject TValue,
	conceptMergeTraits<TKey> TMergeTraits = MergeTraits<TKey>,
	conceptMemManager TMemManager = MemManagerDefault>
using MergeMap = MergeMapCore<MergeMapKeyValueTraits<TKey, TValue, TMemManager>, TMergeTraits>;

template<conceptObject TKey>
using MergeMapHash = MergeMap<TKey, MergeTraitsHash<TKey>>;

} // namespace momo
