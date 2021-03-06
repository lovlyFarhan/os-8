#ifndef __Hash_h__
#define __Hash_h__

/** @file Hash.h
	@todo evaluate the ability to do partial hashes so not all the data needs to be in memory.
	@todo document
	@todo trace_bool in for loops
*/

#include <string>
#include <string.h>
#include <ctype.h>
#include "Exception.h"

#if __APPLE_CC__ || __APPLE__
	#include <CommonCrypto/CommonDigest.h>
#endif

#ifndef trace_scope
	#define trace_scope ///< in case Tracer.h is not included
#endif
#ifndef trace_bool
	#define trace_bool(x) (x) ///< in case Tracer.h is not included
#endif

/** Hashing support.
*/
namespace hash {
	/** Pure virtual parent of all specific hashers.
	*/
	class Hash {
		public:
			/// Default constructor.
			Hash();
			/// Nothing to do in destructor.
			virtual ~Hash();
			/// Fills <code>value</code> with the hex value of the hash.
			virtual std::string &hex(std::string &value) const= 0;
			/// Returns the number of bytes in the hash.
			virtual uint32_t size() const= 0;
			/// Returns the pointer to the internal hash buffer.
			virtual const uint8_t *buffer() const= 0;
			/// Resets the hash and starts hashing a new string.
			virtual void reset(const char *hash)= 0;
			/// Resets the hash and starts hashing new data.
			virtual void reset(const void *data, size_t count)= 0;
			/// Resets the hash and starts hashing a new string.
			virtual void reset(const std::string &data)= 0;
			/// The name of the hashing function.
			virtual const char *name() const= 0;
	};
	/** The specific instantiation of a Hash.
		@tparam Hasher	See SHA256Hasher below as an example.
						Must implement name(), hash() and have a member Size.
	*/
	template<class Hasher>
	class SpecificHash : public Hash {
		public:
			enum {
				Size = Hasher::Size ///< The number of bytes in the hash.
			};
			/// The default constructor.
			SpecificHash();
			/// Construct and initialize with hash of a string.
			SpecificHash(const char *hash);
			/// Construct and initialize with hash of data.
			SpecificHash(const void *data, size_t count);
			/// Construct and initialize with hash of a string.
			SpecificHash(const std::string &data);
			/// Copy another hash's data.
			SpecificHash(const SpecificHash &other);
			/// Nothing to do in destructor.
			virtual ~SpecificHash();
			/// Assign another hasher's data to us.
			SpecificHash &operator=(const SpecificHash &other);
			/// Determine if our current hash is the same as another.
			bool operator==(const SpecificHash &other);
			/// Determine if we are different than another hash.
			bool operator!=(const SpecificHash &other);
			bool valid() const;
			bool same(const SpecificHash &other);
			operator bool() const;
			uint8_t *buffer();
			virtual const uint8_t *buffer() const;
			virtual uint32_t size() const;
			virtual std::string &hex(std::string &value) const;
			virtual void reset(const char *hash);
			virtual void reset(const void *data, size_t count);
			virtual void reset(const std::string &data);
			virtual const char *name() const;
		private:
			uint8_t	_hash[Size];
	};

#if __APPLE_CC__ || __APPLE__

	/// An MD5 example of a hasher. See SpecificHash.
	struct MD5Hasher {
		enum {
			Size= CC_MD5_DIGEST_LENGTH ///< Every SpecificHash hasher must have a Size enum. This is the number of bytes in the hash.
		};
		/** Every SpecificHash hasher must have a name() method. This is the name of this hash.
			@return	The name of this hash method.
		*/
		static const char *name() {trace_scope return "md5";}
		/** Every SpecificHash hasher must have a hash() method. This computes the hash of the given data.
			@param data		The data to hash.
			@param dataSize	The number of bytes in <code>data</code> to hash.
			@param hash		An area of memory, at least <code>Size</code> bytes in length that will hold the hash.
		*/
		static void hash(const void *data, size_t dataSize, void *hash) {trace_scope CC_MD5(data, dataSize, reinterpret_cast<unsigned char*>(hash));}
	};

	/// A SHA256 example of a hasher. See SpecificHash.
	struct SHA256Hasher {
		enum {
			Size= CC_SHA256_DIGEST_LENGTH ///< Every SpecificHash hasher must have a Size enum. This is the number of bytes in the hash.
		};
		/** Every SpecificHash hasher must have a name() method. This is the name of this hash.
			@return	The name of this hash method.
		*/
		static const char *name() {trace_scope return "sha256";}
		/** Every SpecificHash hasher must have a hash() method. This computes the hash of the given data.
			@param data		The data to hash.
			@param dataSize	The number of bytes in <code>data</code> to hash.
			@param hash		An area of memory, at least <code>Size</code> bytes in length that will hold the hash.
		*/
		static void hash(const void *data, size_t dataSize, void *hash) {trace_scope CC_SHA256(data, dataSize, reinterpret_cast<unsigned char*>(hash));}
	};

#endif


	/**
	*/
	inline Hash::Hash() {trace_scope
	}
	/**
	*/
	inline Hash::~Hash() {trace_scope
	}

	/**
		@todo TEST!
	*/
	template<class Hasher> inline SpecificHash<Hasher>::SpecificHash()
		:_hash() {trace_scope
		memset(_hash, 0, sizeof(_hash));
	}
	template<class Hasher> inline SpecificHash<Hasher>::SpecificHash(const char *hash)
		:_hash() {trace_scope
		reset(hash);
	}
	template<class Hasher> inline SpecificHash<Hasher>::SpecificHash(const void *data, size_t count)
		:_hash() {trace_scope
		Hasher::hash(data, count, _hash);
	}
	/**
		@todo TEST!
	*/
	template<class Hasher> inline SpecificHash<Hasher>::SpecificHash(const std::string &data)
		:_hash() {trace_scope
		Hasher::hash(data.data(), data.size(), _hash);
	}
	/**
		@todo TEST!
	*/
	template<class Hasher> inline SpecificHash<Hasher>::SpecificHash(const SpecificHash &other)
		:Hash(other), _hash() {trace_scope
		memcpy(_hash, other._hash, sizeof(_hash));
	}
	template<class Hasher> inline SpecificHash<Hasher>::~SpecificHash() {trace_scope
	}
	template<class Hasher> inline SpecificHash<Hasher> &SpecificHash<Hasher>::operator=(const SpecificHash<Hasher> &other) {trace_scope
		if(this != &other) {trace_scope
			memcpy(_hash, other._hash, sizeof(_hash));
		}
		return *this;
	}
	template<class Hasher> inline bool SpecificHash<Hasher>::operator==(const SpecificHash<Hasher> &other) {trace_scope
		return trace_bool(memcmp(_hash, other._hash, sizeof(_hash)) == 0);
	}
	/**
		@todo TEST!
	*/
	template<class Hasher> inline bool SpecificHash<Hasher>::operator!=(const SpecificHash<Hasher> &other) {trace_scope
		return trace_bool(memcmp(_hash, other._hash, sizeof(_hash)) != 0);
	}
	template<class Hasher> inline bool SpecificHash<Hasher>::valid() const {trace_scope
		for(int i= 0; trace_bool(i < static_cast<int>(sizeof(_hash))); ++i) {trace_scope
			if(_hash[i] != 0) {trace_scope
				return true;
			}
		}
		return false;
	}
	/**
		@todo TEST!
	*/
	template<class Hasher> inline bool SpecificHash<Hasher>::same(const SpecificHash<Hasher> &other) {trace_scope
		return trace_bool(memcmp(_hash, other._hash, sizeof(_hash)) == 0);
	}
	/**
		@todo TEST!
	*/
	template<class Hasher> inline SpecificHash<Hasher>::operator bool() const {trace_scope
		return trace_bool(valid());
	}
	/**
		@todo TEST!
	*/
	template<class Hasher> inline uint8_t *SpecificHash<Hasher>::buffer() {trace_scope
		return _hash;
	}
	/**
		@todo TEST!
	*/
	template<class Hasher> inline const uint8_t *SpecificHash<Hasher>::buffer() const {trace_scope
		return _hash;
	}
	/**
		@todo TEST!
	*/
	template<class Hasher> inline uint32_t SpecificHash<Hasher>::size() const {trace_scope
		return Hasher::Size;
	}
	template<class Hasher> inline std::string &SpecificHash<Hasher>::hex(std::string &value) const {trace_scope
		const char * const	hexDigits= "0123456789abcdef";

		value.clear();
		for(int i= 0; trace_bool(i < static_cast<int>(sizeof(_hash))); ++i) {trace_scope
			value.append(1, hexDigits[_hash[i] & 0x0F]);
			value.append(1, hexDigits[_hash[i] >> 4]);
		}
		return value;
	}
	template<class Hasher> inline void SpecificHash<Hasher>::reset(const char *hash) {trace_scope
		std::string	hexDigits("0123456789abcdef");
		bool		eosFound= false;
		AssertMessageException(strlen(hash) <= Size * 2);
		for(int byte= 0; trace_bool(byte < static_cast<int>(sizeof(_hash))); ++byte) {trace_scope
			const int	nibble1= byte * 2;
			eosFound= trace_bool(trace_bool(eosFound) || trace_bool(hash[nibble1] == '\0'));
			std::string::size_type	found1= eosFound ? 0 : hexDigits.find(hash[nibble1]);

			const int	nibble2= nibble1 + 1;
			eosFound= trace_bool(trace_bool(eosFound) || trace_bool(hash[nibble1] == '\0'));
			std::string::size_type	found2= eosFound ? 0 : hexDigits.find(hash[nibble2]);

			AssertMessageException(found1 != std::string::npos);
			AssertMessageException(found2 != std::string::npos);
			_hash[byte]= (found2 << 4) | found1;
		}
		AssertMessageException(hash[sizeof(_hash)*2] == '\0');
	}
	/**
		@todo TEST!
	*/
	template<class Hasher> inline void SpecificHash<Hasher>::reset(const void *data, size_t count) {trace_scope
		Hasher::hash(data, count, _hash);
	}
	/**
		@todo TEST!
	*/
	template<class Hasher> inline void SpecificHash<Hasher>::reset(const std::string &data) {trace_scope
		Hasher::hash(data.data(), data.size(), _hash);
	}
	/**
		@todo TEST!
	*/
	template<class Hasher> inline const char *SpecificHash<Hasher>::name() const {trace_scope
		return Hasher::name();
	}
}

#endif // __Hash_h__
