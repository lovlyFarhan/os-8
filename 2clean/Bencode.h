#ifndef __Bencode_h__
#define __Bencode_h__

#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <cstdlib>
#include <inttypes.h>
#include <stdint.h>

#ifndef trace_scope
	#define trace_scope ///< in case Tracer.h is not included
#endif
#ifndef trace_bool
	#define trace_bool(x) (x) ///< in case Tracer.h is not included
#endif

#define bencodeNULLEntry	"0:"

/** Allows encoding and decoding Bit Torrent originated b-encoded data.
	@todo Only allow string keys for Dictionary
	@todo Allow whitespace before e and before i/l/d
*/
namespace bencode {

	/** Benocde Item types
	*/
	enum Type {
		TypeString,		//< String or buffer type
		TypeInteger,	//< Signed Integer type
		TypeList,		//< List of Items
		TypeDictionary,	//< Dictionary from String to Item
		TypeInvalid		//< Not a valid Type
	};

	/** Interface for reading Bencode data.
	*/
	class Input {
		public:
			Input();
			virtual ~Input();
			/** Reads a number of bytes from source.
				@param bytes	The number of bytes to read from the source
				@param storage	The buffer to hold the bytes read (appended, never cleared)
				@return			Reference to storage
			*/
			virtual std::string &read(size_t bytes, std::string &storage);
			/** Reads one byte from the source.
				@return The byte read
			*/
			virtual char read()= 0;
			/** Are we at the end of the source?
				@return	true if there are no more bytes to be read, false if you can read more data
			*/
			virtual bool end()= 0;
	};

	/** An example Input class that reads Bencode data from a std::string.
	*/
	class ReferencedStringInput : public Input {
		public:
			/**
				@param buffer	Reference to buffer that will be read.
			*/
			ReferencedStringInput(const std::string &buffer);
			virtual ~ReferencedStringInput();
			virtual std::string &read(size_t bytes, std::string &storage);
			virtual char read();
			/**
				@return true if we've reached the end of the string.
			*/
			virtual bool end();
		private:
			const std::string		&_buffer;	//< Reference to the original buffer
			std::string::size_type	_position;	//< The position to read from next
	};

	/** Interface for writing Bencode data
	*/
	class Output {
		public:
			Output();
			virtual ~Output();
			/** @param data	The bencoded data to write to the destination */
			virtual void write(const std::string &data);
			/** @param byte	The bencoded byte to write to the destination */
			virtual void write(char byte)= 0;
	};

	/** An example Ouput class that writes Bencode data to a std::string.
	*/
	class ReferencedStringOutput : public Output {
		public:
			/** @param buffer	The string to fill with bencoded data.
									Never cleared, always appended
			*/
			ReferencedStringOutput(std::string &buffer);
			virtual ~ReferencedStringOutput();
			virtual void write(const std::string &data);
			virtual void write(char byte);
		private:
			std::string	&_buffer;	//< Bencode data is appended to this string
	};

	/** Interface for all nodes in a Benocoded data structure
	*/
	class Item {
		public:
			typedef Item 		*Ptr;
			typedef const Item	*ConstPtr;
			/** Creates a root Item for bencoded data.
				@param in	Source that contains at least one bencoded data element.
				@return		The next bencoded element from in, although it may be complex
			*/
			static Item::Ptr read(Input &in);
			Item();
			virtual ~Item();
			virtual Type type() const;
			virtual void write(Output &);
			/** Cast this Item to a specific node to call node specific methods.
				<code>I</code> is the type, like String, Integer, List or Dictionary.
				Make sure you check type() before calling as().
			*/
			template<typename I> I &as();
			template<typename I> const I &as() const;
			bool operator<(const Item &other) const;
			bool operator<=(const Item &other) const;
			bool operator>(const Item &other) const;
			bool operator>=(const Item &other) const;
			bool operator==(const Item &other) const;
			bool operator!=(const Item &other) const;
			virtual uint32_t componentCount() const=0;
			virtual std::string &component(uint32_t index, std::string &buffer) const=0;
			virtual Item::Ptr clone() const;
			virtual std::string &display(std::string &buffer);
			int compare(const Item &other) const;
			class Assignment {
				public:
					Assignment(Item::Ptr &item);
					Assignment(const Assignment &other);
					~Assignment();
					Assignment &operator=(intmax_t value);
					Assignment &operator=(const std::string &value);
					Assignment &operator=(Item::Ptr value);
					operator Item::Ptr();
					Item::Ptr operator->();
					bool operator==(Item::Ptr item);
					bool operator!=(Item::Ptr item);
				private:
					Item::Ptr	&_item;
			};
		protected:
			static Item::Ptr _read(Input &in, char type, std::string &buffer);
	};

	class String : public Item {
		public:
			String(const std::string &string);
			virtual ~String();
			virtual Type type() const;
			std::string &value();
			const std::string &value() const;
			virtual void write(Output &out);
			virtual uint32_t componentCount() const;
			virtual std::string &component(uint32_t index, std::string &buffer) const;
			virtual Item::Ptr clone() const;
			virtual std::string &display(std::string &buffer);
		private:
			std::string	_value;
	};

	class Integer : public Item {
		public:
			Integer(intmax_t value);
			Integer(const std::string &value);
			virtual ~Integer();
			virtual Type type() const;
			virtual void write(Output &out);
			intmax_t &value();
			const intmax_t &value() const;
			virtual uint32_t componentCount() const;
			virtual std::string &component(uint32_t index, std::string &buffer) const;
			virtual Item::Ptr clone() const;
			virtual std::string &display(std::string &buffer);
		private:
			intmax_t	_value;
	};

	class List : public Item {
		public:
			List();
			virtual ~List();
			virtual Type type() const;
			virtual void write(Output &out);
			uint32_t count() const;
			Assignment operator[](uint32_t index);
			Item::ConstPtr operator[](uint32_t index) const;
			Assignment value(uint32_t index);
			Item::ConstPtr value(uint32_t index) const;
			void insert(Item::Ptr item, uint32_t before= 0);
			void insert(const std::string &value, uint32_t before= 0);
			void insert(intmax_t value, uint32_t before= 0);
			Item::Ptr remove(uint32_t index);
			void push(Item::Ptr item);
			void push(const std::string &value);
			void push(intmax_t value);
			Item::Ptr pop();
			virtual uint32_t componentCount() const;
			virtual std::string &component(uint32_t index, std::string &buffer) const;
			virtual Item::Ptr clone() const;
			virtual std::string &display(std::string &buffer);
		private:
			typedef std::vector<Item::Ptr>	_Items;
			_Items	_items;
	};

	class Dictionary : public Item {
		public:
			class key_iterator {
				public:
					key_iterator(const key_iterator &other);
					key_iterator(const Dictionary *c, uint32_t i);
					~key_iterator();
					key_iterator &operator++();
					key_iterator operator++(int);
					key_iterator &operator--();
					key_iterator operator--(int);
					key_iterator &operator+=(int count);
					key_iterator operator+(int count);
					key_iterator operator-(int count);
					key_iterator &operator-=(int count);
					key_iterator &operator=(const key_iterator &other);
					bool operator==(const key_iterator &other) const;
					bool operator!=(const key_iterator &other) const;
					bool operator<(const key_iterator &other) const;
					bool operator<=(const key_iterator &other) const;
					bool operator>(const key_iterator &other) const;
					bool operator>=(const key_iterator &other) const;
					Item &operator*();
					Item::Ptr operator->();
					operator bool() const;
				private:
					const Dictionary	*_container;
					uint32_t	_index;
					bool _valid() const;
			};
			Dictionary();
			virtual ~Dictionary();
			virtual Type type() const;
			virtual void write(Output &out);
			key_iterator keys() const;
			Assignment operator[](Item::ConstPtr key);
			Item::ConstPtr operator[](Item::ConstPtr key) const;
			Assignment operator[](const std::string &key);
			Item::ConstPtr operator[](const std::string &key) const;
			bool has_key(Item::Ptr key);
			bool has_key(const std::string &key);
			Item::Ptr remove(Item::Ptr key);
			Item::Ptr remove(const std::string &key);
			virtual uint32_t componentCount() const;
			virtual std::string &component(uint32_t index, std::string &buffer) const;
			virtual Item::Ptr clone() const;
			virtual std::string &display(std::string &buffer);
		private:
			typedef std::pair<Item::Ptr,Item::Ptr>	_Element;
			typedef std::vector<_Element>			_Items;
			_Items	_items;
			bool _find(Item::ConstPtr key, _Items::iterator &position) const;
	};

	/**
		template parameter IntegerType requires:
							operator<(int)
							operator=(IntegerType)
							size_t std::abs(operator%(int))
							operator/=(int)
	*/
	template<typename IntegerType> std::string &itoa(IntegerType value, std::string &buffer, int base= 10);
	template<typename IntegerType> std::string &itoa(IntegerType value, int base= 10);

	inline Input::Input() {trace_scope}
	inline Input::~Input() {trace_scope}
	inline std::string &Input::read(size_t bytes, std::string &storage) {trace_scope
		storage.clear();
		while( (bytes > 0) && !end() ) {storage.append(1, read()); --bytes;}
		return storage;
	}

	inline ReferencedStringInput::ReferencedStringInput(const std::string &buffer)
		:_buffer(buffer), _position(0) {trace_scope}
	inline ReferencedStringInput::~ReferencedStringInput() {trace_scope}
	inline std::string &ReferencedStringInput::read(size_t bytes, std::string &storage) {trace_scope
		if(_position < _buffer.size()) {
			bytes= _position + bytes < _buffer.size()
					? bytes
					: _buffer.size() - _position;
			storage.assign(_buffer, _position, bytes);
			_position+= bytes;
		} else {
			storage.clear();
		}
		return storage;
	}
	inline char ReferencedStringInput::read() {trace_scope
		return _position < _buffer.size() ? _buffer[_position++] : '\0';
	}
	inline bool ReferencedStringInput::end() {trace_scope
		return (_buffer.size() == 0) || (_position < _buffer.size() - 1);
	}

	inline Output::Output() {trace_scope}
	inline Output::~Output() {trace_scope}
	inline void Output::write(const std::string &data) {trace_scope
		for(size_t c= 0; trace_bool(c < data.size()); ++c) {write(data[c]);}
	}

	inline ReferencedStringOutput::ReferencedStringOutput(std::string &buffer):_buffer(buffer) {trace_scope}
	inline ReferencedStringOutput::~ReferencedStringOutput() {trace_scope}
	inline void ReferencedStringOutput::write(const std::string &data) {trace_scope printf("data.size()=%ld _buffer.size()=%ld &data=%p &_buffer=%p this=%p\n", data.size(),_buffer.size(), &data, &_buffer, this); printf("data='%s'\n", data.c_str());printf("_buffer='%s'\n", _buffer.c_str());_buffer.append(data);}
	inline void ReferencedStringOutput::write(char byte) {trace_scope _buffer.append(1,byte);}

	inline Item::Assignment::Assignment(Item::Ptr &item):_item(item) {trace_scope}
	inline Item::Assignment::~Assignment() {trace_scope}
	inline Item::Assignment &Item::Assignment::operator=(intmax_t value) {trace_scope
		if(NULL != _item) {
			delete _item;
		}
		_item= new Integer(value);
		return *this;
	}
	inline Item::Assignment &Item::Assignment::operator=(const std::string &value) {trace_scope
		if(NULL != _item) {
			delete _item;
		}
		_item= new String(value);
		return *this;
	}
	inline Item::Assignment &Item::Assignment::operator=(Item::Ptr value) {trace_scope
		if(value == _item) {
			return *this;
		}
		if(NULL != _item) {
			delete _item;
		}
		_item= value;
		return *this;
	}
	inline Item::Assignment::operator Item::Ptr() {trace_scope return _item;}
	inline Item::Ptr Item::Assignment::operator->() {trace_scope return _item;}
	inline bool Item::Assignment::operator==(Item::Ptr item) {trace_scope return _item == item;}
	inline bool Item::Assignment::operator!=(Item::Ptr item) {trace_scope return _item != item;}
	inline Item::Assignment::Assignment(const Assignment &other)
		:_item(other._item) {trace_scope}

	inline Item::Ptr Item::read(Input &in) {trace_scope
		const char	type= in.read();
		std::string	buffer;

		return _read(in, type, buffer);
	}
	inline Item::Item() {trace_scope}
	inline Item::~Item() {trace_scope}
	inline Type Item::type() const {trace_scope return TypeInvalid;}
	inline void Item::write(Output &) {trace_scope}
	template<typename I> inline I &Item::as() {trace_scope return *reinterpret_cast<I*>(this);}
	template<typename I> inline const I &Item::as() const {trace_scope return *reinterpret_cast<const I*>(this);}
	inline bool Item::operator<(const Item &other) const {trace_scope return compare(other) < 0;}
	inline bool Item::operator<=(const Item &other) const {trace_scope return compare(other) <= 0;}
	inline bool Item::operator>(const Item &other) const {trace_scope return compare(other) > 0;}
	inline bool Item::operator>=(const Item &other) const {trace_scope return compare(other) >= 0;}
	inline bool Item::operator==(const Item &other) const {trace_scope return compare(other) == 0;}
	inline bool Item::operator!=(const Item &other) const {trace_scope return compare(other) != 0;}
	inline Item::Ptr Item::clone() const {trace_scope return NULL;}
	inline std::string &Item::display(std::string &buffer) {buffer.clear(); return buffer;}
	inline int Item::compare(const Item &other) const {trace_scope
		const uint32_t	c1= componentCount();
		const uint32_t	c2= other.componentCount();
		const uint32_t	max= c1<c2 ? c1 : c2;
		std::string		s1, s2;

		for(uint32_t i= 0; trace_bool(i < max); ++i) {
			const int result= component(i,s1).compare(other.component(i,s2));

			if(result != 0) {
				return result;
			}
		}
		return (c1 < c2) ? -1 : ( (c1 > c2) ? 1 : 0 );
	}
	inline Item::Ptr Item::_read(Input &in, char type, std::string &buffer) {trace_scope
		Item::Ptr				result= NULL;
		char					byte;
		std::string::size_type	size;
		switch(type) {
			case 'i':
				buffer.clear();
				while('e' != (byte= in.read())) {
					buffer.append(1, byte);
				}
				result= new Integer(buffer);
				break;
			case '0':case '1':case '2':case '3':case '4':
			case '5':case '6':case '7':case '8':case '9':
				byte= type;
				buffer.clear();
				do {
					if( (byte < '0') || (byte > '9') ) {
						return NULL;
					}
					buffer.append(1, byte);
					byte= in.read();
				} while(':' != byte);
				size= strtoimax(buffer.c_str(), static_cast<char**>(NULL), 10);
				buffer.clear();
				in.read(size, buffer);
				result= new String(buffer);
				break;
			case 'l':
				result= new List();
				while('e' != (byte= in.read())) {
					Item::Ptr	item= _read(in, byte, buffer);

					if(NULL == item) {
						delete result;
						return NULL;
					}
					result->as<List>().push(item);
				}
				break;
			case 'd':
				result= new Dictionary();
				while('e' != (byte= in.read())) {
					Item::Ptr	key= _read(in, byte, buffer);
					Item::Ptr	value= _read(in, in.read(), buffer);

					if( (NULL == key) || (NULL == value) ) {
						delete result;
						delete value;
						delete key;
						return NULL;
					}
					result->as<Dictionary>()[key]= value;
				}
				break;
			default:
				break;
		};
		return result;
	}

	inline String::String(const std::string &string):_value(string) {trace_scope}
	inline String::~String() {trace_scope}
	inline Type String::type() const {trace_scope return TypeString;}
	inline std::string &String::value() {trace_scope return _value;}
	inline const std::string &String::value() const {trace_scope return _value;}
	inline void String::write(Output &out) {trace_scope
		printf("_value.size()==%ld\n", _value.size());
		out.write(itoa(_value.size()));out.write(':');out.write(_value);
	}
	inline uint32_t String::componentCount() const {trace_scope
		return 1;
	}
	inline std::string &String::component(uint32_t index, std::string &buffer) const {trace_scope
		if(index == 0) {
			buffer.assign(_value);
		}
		return buffer;
	}
	inline Item::Ptr String::clone() const {trace_scope return new String(_value);}
	inline std::string &String::display(std::string &buffer) {buffer= '"'+_value+'"'; return buffer;}

	inline Integer::Integer(intmax_t intValue):_value(intValue) {trace_scope}
	inline Integer::Integer(const std::string &strValue)
		:_value(strtoimax(strValue.c_str(), static_cast<char**>(NULL), 10)) {trace_scope}
	inline Integer::~Integer() {trace_scope}
	inline Type Integer::type() const {trace_scope return TypeInteger;}
	inline void Integer::write(Output &out) {trace_scope
		out.write('i');out.write(itoa(_value));out.write('e');
	}
	inline intmax_t &Integer::value() {trace_scope return _value;}
	inline const intmax_t &Integer::value() const {trace_scope return _value;}
	inline uint32_t Integer::componentCount() const {trace_scope return 1;}
	inline std::string &Integer::component(uint32_t index, std::string &buffer) const {trace_scope
		if(index == 0) {
			buffer.clear();
			itoa(_value, buffer);
		}
		return buffer;
	}
	inline Item::Ptr Integer::clone() const {trace_scope return new Integer(_value);}
	inline std::string &Integer::display(std::string &buffer) {buffer.clear(); itoa(_value, buffer); return buffer;}

	inline List::List():_items() {trace_scope}
	inline List::~List() {trace_scope
		while(_items.size() > 0) {
			delete pop();
		}
	}
	inline Type List::type() const {trace_scope return TypeList;}
	inline void List::write(Output &out) {trace_scope
		out.write('l');
		for(_Items::iterator item= _items.begin(); trace_bool(item != _items.end()); ++item) {
			if(NULL != *item) {
				(*item)->write(out);
			} else {
				out.write(std::string(bencodeNULLEntry));
			}
		}
		out.write('e');
	}
	inline uint32_t List::count() const {trace_scope return static_cast<uint32_t>(_items.size());}
	inline Item::Assignment List::operator[](uint32_t index) {trace_scope
		for(size_t fill= _items.size(); trace_bool(fill <= index); ++fill) {
			push(reinterpret_cast<Item::Ptr>(NULL));
		}
		return Assignment(_items[index]);
	}
	inline Item::ConstPtr List::operator[](uint32_t index) const {trace_scope
		for(size_t fill= _items.size(); trace_bool(fill <= index); ++fill) {
			const_cast<List*>(this)->push(reinterpret_cast<Item::Ptr>(NULL));
		}
		return _items[index];
	}
	inline Item::Assignment List::value(uint32_t index) {trace_scope return (*this)[index];}
	inline Item::ConstPtr List::value(uint32_t index) const {trace_scope return (*this)[index];}
	inline void List::insert(Item::Ptr item, uint32_t before) {trace_scope
		_items.insert(_items.begin()+before, item);
	}
	inline void List::insert(const std::string &strValue, uint32_t before) {trace_scope
		insert(new String(strValue), before);
	}
	inline void List::insert(intmax_t intValue, uint32_t before) {trace_scope
		insert(new Integer(intValue), before);
	}
	inline Item::Ptr List::remove(uint32_t index) {trace_scope
		Item::Ptr	item= index < _items.size() ? _items[index] : NULL;

		_items.erase(_items.begin() + index);
		return item;
	}
	inline void List::push(Item::Ptr item) {trace_scope insert(item, _items.size());}
	inline void List::push(const std::string &strValue) {trace_scope
		push(reinterpret_cast<Item::Ptr>(new String(strValue)));
	}
	inline void List::push(intmax_t intValue) {trace_scope
		push(reinterpret_cast<Item::Ptr>(new Integer(intValue)));
	}
	inline Item::Ptr List::pop() {trace_scope return remove(_items.size() - 1);}
	inline uint32_t List::componentCount() const {trace_scope
		uint32_t components= 0;

		for(_Items::const_iterator item= _items.begin(); trace_bool(item != _items.end()); ++item) {
			if(NULL != *item) {
				components+= (*item)->componentCount();
			} else {
				++components;
			}
		}
		return components;
	}
	inline std::string &List::component(uint32_t index, std::string &buffer) const {trace_scope
		for(_Items::const_iterator item= _items.begin(); trace_bool(item != _items.end()); ++item) {
			const uint32_t	itemCount= (NULL == *item) ? 1 : (*item)->componentCount();

			if(index >= itemCount) {
				index-= itemCount;
			} else if(NULL == *item) {
				buffer.clear();
				return buffer;
			} else {
				return (*item)->component(index, buffer);
			}
		}
		buffer.clear();
		return buffer;
	}
	inline Item::Ptr List::clone() const {trace_scope
		List	*result= new List();

		for(_Items::const_iterator item= _items.begin(); trace_bool(item != _items.end()); ++item) {
			result->push(NULL == *item ? NULL : (*item)->clone());
		}
		return result;
	}
	inline std::string &List::display(std::string &buffer) {
		std::string	subitemValue;
		std::string	separator= "";

		buffer= "[";
		for(_Items::const_iterator item= _items.begin(); trace_bool(item != _items.end()); ++item) {
			if(NULL == *item) {
				buffer.append(separator+"[NULL]");
			} else {
				buffer.append(separator+(*item)->display(subitemValue));
			}
			separator= ", ";
		}
		buffer.append("]");
		return buffer;
	}

	inline Dictionary::key_iterator::key_iterator(const key_iterator &other)
		:_container(other._container),_index(other._index) {trace_scope }
	inline Dictionary::key_iterator::~key_iterator() {trace_scope }
	inline Dictionary::key_iterator &Dictionary::key_iterator::operator++() {trace_scope ++_index; return *this;}
	inline Dictionary::key_iterator Dictionary::key_iterator::operator++(int) {trace_scope
		++_index; return key_iterator(_container, _index - 1);
	}
	inline Dictionary::key_iterator &Dictionary::key_iterator::operator--() {trace_scope if(_index > 0) {--_index;} return *this;}
	inline Dictionary::key_iterator Dictionary::key_iterator::operator--(int) {trace_scope
		int increment= _index > 0 ? 1 : 0;

		_index-= increment;
		return key_iterator(_container, _index + increment);
	}
	/**
		@todo count can be negative, += and -= need to check we don't go below zero
	*/
	inline Dictionary::key_iterator &Dictionary::key_iterator::operator+=(int count) {trace_scope _index+= count; return *this;}
	inline Dictionary::key_iterator &Dictionary::key_iterator::operator-=(int count) {trace_scope
		_index= (count > static_cast<int>(_index)) ? 0 : _index - count; return *this;
	}
	inline Dictionary::key_iterator Dictionary::key_iterator::operator+(int count) {trace_scope
		key_iterator result(*this); return result+= count;
	}
	inline Dictionary::key_iterator Dictionary::key_iterator::operator-(int count) {trace_scope
		key_iterator result(*this); return result-= count;
	}
	inline Dictionary::key_iterator &Dictionary::key_iterator::operator=(const key_iterator &other) {trace_scope
		_container= other._container;
		_index= other._index;
		return *this;
	}
	inline bool Dictionary::key_iterator::operator==(const key_iterator &other) const {trace_scope
		return (_container == other._container)
				&& (_index == other._index);
	}
	inline bool Dictionary::key_iterator::operator!=(const key_iterator &other) const {trace_scope
		return !(*this == other);
	}
	inline bool Dictionary::key_iterator::operator<(const key_iterator &other) const {trace_scope
		return (_container == other._container)
				&& (_index < other._index);
	}
	inline bool Dictionary::key_iterator::operator<=(const key_iterator &other) const {trace_scope
		return (_container == other._container)
				&& (_index <= other._index);
	}
	inline bool Dictionary::key_iterator::operator>(const key_iterator &other) const {trace_scope return other < *this;}
	inline bool Dictionary::key_iterator::operator>=(const key_iterator &other) const {trace_scope return other <= *this;}
	inline Item &Dictionary::key_iterator::operator*() {trace_scope
		return *_container->_items[_index].first;
	}
	inline Item::Ptr Dictionary::key_iterator::operator->() {trace_scope
		return _valid() ? _container->_items[_index].first : NULL;
	}
	inline Dictionary::key_iterator::operator bool() const {trace_scope return _valid();}
	inline Dictionary::key_iterator::key_iterator(const Dictionary *c, uint32_t i)
		:_container(c),_index(i) {trace_scope }
	inline bool Dictionary::key_iterator::_valid() const {trace_scope
		return _index < _container->_items.size();
	}

	Dictionary::Dictionary():_items() {trace_scope }
	Dictionary::~Dictionary() {trace_scope
		while(_items.size() > 0) {
			delete _items[0].first;
			delete _items[0].second;
			_items.erase(_items.begin());
		}
	}
	Type Dictionary::type() const {trace_scope return TypeDictionary;}
	void Dictionary::write(Output &out) {trace_scope
		out.write('d');
		for(_Items::iterator item= _items.begin(); trace_bool(item != _items.end()); ++item) {
			if(NULL == item->first) {
				out.write(bencodeNULLEntry);
			} else {
				item->first->write(out);
			}
			if(NULL == item->second) {
				out.write(bencodeNULLEntry);
			} else {
				item->second->write(out);
			}
		}
		out.write('e');
	}
	Dictionary::key_iterator Dictionary::keys() const {trace_scope return key_iterator(this, 0);}
	Item::Assignment Dictionary::operator[](Item::ConstPtr key) {trace_scope
		_Items::iterator	position= _items.end();

		if(!_find(key, position)) {
			_items.insert(position, _Element(NULL == key ? NULL : key->clone(), NULL));
			_find(key, position);
		}
		return Assignment(position->second);
	}
	Item::ConstPtr Dictionary::operator[](Item::ConstPtr key) const {trace_scope
		_Items::iterator	position= const_cast<Dictionary*>(this)->_items.end();

		if(!_find(key, position)) {
			const_cast<Dictionary*>(this)->_items.insert(position, _Element(NULL == key ? NULL : key->clone(), NULL));
			_find(key, position);
		}
		return position->second;
	}
	Item::Assignment Dictionary::operator[](const std::string &key) {trace_scope
		String	keyString(key);

		return (*this)[reinterpret_cast<Item::Ptr>(&keyString)];
	}
	Item::ConstPtr Dictionary::operator[](const std::string &key) const {trace_scope
		String	keyString(key);

		return (*this)[reinterpret_cast<Item::Ptr>(&keyString)];
	}
	bool Dictionary::has_key(Item::Ptr key) {trace_scope
		_Items::iterator	position= _items.end();

		return _find(key, position);
	}
	bool Dictionary::has_key(const std::string &key) {trace_scope
		String	keyString(key);

		return has_key(reinterpret_cast<Item::Ptr>(&keyString));
	}
	Item::Ptr Dictionary::remove(Item::Ptr key) {trace_scope
		_Items::iterator	position= _items.end();

		if(_find(key, position)) {
			Item::Ptr	value= position->second;

			delete position->first;
			_items.erase(position);
			return value;
		}
		return NULL;
	}
	Item::Ptr Dictionary::remove(const std::string &key) {trace_scope
		String	keyString(key);

		return remove(reinterpret_cast<Item::Ptr>(&keyString));
	}
	uint32_t Dictionary::componentCount() const {trace_scope
		uint32_t components= 0;

		for(_Items::const_iterator item= _items.begin(); trace_bool(item != _items.end()); ++item) {
			if(NULL != item->first) {
				components+= item->first->componentCount();
			}
			if(NULL != item->second) {
				components+= item->second->componentCount();
			}
		}
		return components;
	}
	std::string &Dictionary::component(uint32_t index, std::string &buffer) const {trace_scope
		for(_Items::const_iterator item= _items.begin(); trace_bool(item != _items.end()); ++item) {
			const uint32_t	firstCount= NULL == item->first ? 1 : item->first->componentCount();
			const uint32_t	secondCount= NULL == item->second ? 1 : item->second->componentCount();

			if(index >= firstCount) {
				index-= firstCount;
			} else if(NULL == item->first) {
				buffer.clear();
				return buffer;
			} else {
				return item->first->component(index, buffer);
			}
			if(index >= secondCount) {
				index-= secondCount;
			} else if(NULL == item->second) {
				buffer.clear();
				return buffer;
			} else {
				return item->second->component(index, buffer);
			}
		}
		buffer.clear();
		return buffer;
	}
	Item::Ptr Dictionary::clone() const {trace_scope
		Dictionary	*result= new Dictionary();

		for(_Items::const_iterator item= _items.begin(); trace_bool(item != _items.end()); ++item) {
			(*result)[NULL == item->first ? NULL : item->first->clone()]
				= NULL == item->second ? NULL : item->second->clone();
		}
		return result;
	}
	inline std::string &Dictionary::display(std::string &buffer) {
		std::string	value;
		std::string	separator= "";

		buffer= "{";
		for(_Items::const_iterator item= _items.begin(); trace_bool(item != _items.end()); ++item) {
			if(NULL == item->first) {
				buffer.append(separator+"[NULL]");
			} else {
				buffer.append(separator+(item->first)->display(value));
			}
			buffer+=':';
			if(NULL == item->second) {
				buffer.append(separator+"[NULL]");
			} else {
				buffer.append((item->second)->display(value));
			}
			separator= ", ";
		}
		buffer.append("}");
		return buffer;
	}
	/**
		@todo Improve speed with binary search
	*/
	bool Dictionary::_find(Item::ConstPtr key, _Items::iterator &position) const {trace_scope
		position= const_cast<Dictionary*>(this)->_items.begin();
		while( (position != const_cast<Dictionary*>(this)->_items.end()) && (key->compare(*position->first) > 0) ) {
			++position;
		}
		if(position == const_cast<Dictionary*>(this)->_items.end()) {
			return false;
		}
		if(position->first == key) {
			return true;
		}
		if( (NULL == position->first) || (NULL == key) ) {
			return false;
		}
		return position->first->compare(*key) == 0;
	}

	template<typename IntegerType>
	inline std::string &itoa(IntegerType value, std::string &buffer, int base) {trace_scope
		const size_t	kMaxDigits= 35;
		IntegerType		quotient= value;

		if( (base < 2) || (base > 16) ) {
			return buffer;
		}
		buffer.reserve(buffer.size() + kMaxDigits);
		do	{
			const int	digit= std::abs(static_cast<int>(quotient%base));

			buffer+= "0123456789abcdef"[digit];
			quotient/= base;
		} while(quotient != 0);
		// goofy math to get rid of warning when IntegerType is unsigned
		if( (value + 1) < 1) {
			buffer+= '-';
		}
		std::reverse(buffer.begin(), buffer.end());
		return buffer;
	}
	template<typename IntegerType>
	inline std::string &itoa(IntegerType value, int base) {trace_scope
		std::string	buffer;

		return itoa(value, buffer, base);
	}

}

#endif // __bencode_h__
