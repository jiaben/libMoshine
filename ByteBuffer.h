/****************************************************************************
 *
 * ByteBuffer Class
*
 */
#include <stdint.h>
#ifndef _BYTEBUFFER_H
#define _BYTEBUFFER_H

/* Use correct types for x64 platforms, too */
#if COMPILER != COMPILER_GNU
typedef signed __int64 int64;
typedef signed __int32 int32;
typedef signed __int16 int16;
typedef signed __int8 int8;

typedef unsigned __int64 uint64;
typedef unsigned __int32 uint32;
typedef unsigned __int16 uint16;
typedef unsigned __int8 uint8;
typedef float	Real;
#else

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;
//typedef uint32_t DWORD;
typedef float	Real;

#endif

//#include "Vector3.h"
#include "cocos2d.h"
class  ByteBuffer :public cocos2d::CCObject
{
#define DEFAULT_SIZE 0x1000
#define DEFAULT_INCREASE_SIZE 200
	uint8 * m_buffer;
	size_t m_readPos;
	size_t m_writePos;
	uint32 m_buffersize;
	uint16 m_opcode;

private:
    static void _memcpy(uint8* dest, const uint8* src, size_t sz)
	{
		for(size_t i = 0; i < sz; ++i)
			*(dest+i) = *(src+i);
	}
    
public:
	/** Creates a bytebuffer with the default size
	 */
	ByteBuffer()
	{
		m_opcode=0;
	    m_buffer =0;
	    m_readPos = m_writePos = 0;
	    m_buffersize = 0;
	    reserve(DEFAULT_SIZE);
	}

	/** Creates a bytebuffer with the specified size
	 */
	ByteBuffer(size_t res)
	{
	    m_buffer =0;
	    m_readPos = m_writePos = 0;
	    m_buffersize = 0;
            reserve(res);
	}

	/** Frees the allocated buffer
	 */
	~ByteBuffer()
	{
		free(m_buffer);
	}


	/** Allocates/reallocates buffer with specified size.
	 */
	void reserve(size_t res)
	{
		if(m_buffer)
			m_buffer = (uint8*)realloc(m_buffer, res);
		else
			m_buffer = (uint8*)malloc(res);

		m_buffersize = res;
	}


	/** Resets read/write indexes
	 */
	inline void clear()
	{
		m_readPos = m_writePos = 0;
	}

	/** Sets write position
	 */
	inline void resize(size_t size)
	{
		m_writePos = size;
	}
	/**
	reset readpos
	*/

	inline void resizeReadPos(size_t size)
	{
		m_readPos = size;
	}

	/** Returns the buffer pointer
	 */
	inline const uint8 * contents()
	{
		return m_buffer;
	}


	/** Gets the buffer size.
	 */
	uint32 GetBufferSize() { return m_writePos; }

	/** Reads sizeof(T) bytes from the buffer
	 * @return the bytes read
	 */
	template<typename T>
		T Read()
	{
		if(m_readPos + sizeof(T) > m_writePos)
			return (T)0;

		const size_t sz = sizeof(T);
		uint8 buf[sz];

		_memcpy(buf, &m_buffer[m_readPos], sz);
		T ret = *(T*)(buf);
		m_readPos += sizeof(T);
		return ret;
	}

	void skip(size_t len)
	{
	       if(m_readPos + len > m_writePos)
                  len = (m_writePos - m_readPos);
	       m_readPos += len;
	}

	/** Reads x bytes from the buffer
	 */
	uint8 *  read(size_t len)
	{
		uint8 * buffer=new uint8[len];
		memset(buffer,0,len);
		if(m_readPos + len > m_writePos)
			len = (m_writePos - m_readPos);

		_memcpy(buffer, &m_buffer[m_readPos], len);
		m_readPos += len;
		return buffer;
	}

	/** Writes sizeof(T) bytes to the buffer, while checking for overflows.
	 * @param T data The data to be written
	 */
	template<typename T>
		void Write(const T & data)
	{
		size_t  new_size = m_writePos + sizeof(T);
		if(new_size > m_buffersize)
		{
			new_size = (new_size / DEFAULT_INCREASE_SIZE + 1) * DEFAULT_INCREASE_SIZE;
			reserve(new_size);
		}
        
        uint8 buf[sizeof(T)];
        *(T*)&buf[0] = data;
        _memcpy(&(m_buffer[m_writePos]), buf, sizeof(T));

//		*(T*)&m_buffer[m_writePos] = data;
		m_writePos += sizeof(T);
	}

	/** writes x bytes to the buffer, while checking for overflows
	 * @param ptr the data to be written
	 * @param size byte count
	*/
	void Write(const uint8 * data, size_t size)
	{
		size_t  new_size = m_writePos + size;
		if(new_size > m_buffersize)
		{
			new_size = (new_size / DEFAULT_INCREASE_SIZE + 1) * DEFAULT_INCREASE_SIZE;
			reserve(new_size);
		}

		memcpy(&m_buffer[m_writePos], data, size);
		m_writePos += size;
	}


    __inline uint16 GetOpcode() const { return m_opcode; }
    __inline void SetOpcode(const uint16 & opcode) { m_opcode = opcode; }
    __inline void SetLength(const uint16 & len)	{ 
		uint16 * plen = (uint16 * ) &(contents()[2]);
		*plen = len;
	}

	__inline std::string getString()
	{
		return (const char*)contents();
	}
	
 template<typename T>
   	void SetOffset(const uint16 & offset, const T value ) {
		T * pval = (T *) &(contents()[offset]);
		*pval = value;
	}
	//*****
	/** Ensures the buffer is big enough to fit the specified number of bytes.
	 * @param bytes number of bytes to fit
	 */
	inline void EnsureBufferSize(uint32 Bytes)
	{
		size_t  new_size = m_writePos + Bytes;
		if(new_size > m_buffersize)
                {
                        new_size = (new_size / DEFAULT_INCREASE_SIZE + 1) * DEFAULT_INCREASE_SIZE;
                        reserve(new_size);
                }

	}

	/** These are the default read/write operators.
	 */
#define DEFINE_BUFFER_READ_OPERATOR(type) void operator >> (type& dest) { dest = Read<type>(); }
#define DEFINE_BUFFER_WRITE_OPERATOR(type) void operator << (const type src) { Write<type>(src); }

	/** Fast read/write operators without using the templated read/write functions.
	 */
#define DEFINE_FAST_READ_OPERATOR(type, size) ByteBuffer& operator >> (type& dest) { if(m_readPos + size > m_writePos) { dest = (type)0; return *this; } else { dest = *(type*)&m_buffer[m_readPos]; m_readPos += size; return *this; } }
#define DEFINE_FAST_WRITE_OPERATOR(type, size) ByteBuffer& operator << (const type src) { if(m_writePos + size > m_buffersize) { reserve(m_buffersize + DEFAULT_INCREASE_SIZE); } *(type*)&m_buffer[m_writePos] = src; m_writePos += size; return *this; }

	/** Integer/float r/w operators
	*/
	DEFINE_FAST_READ_OPERATOR(uint64, 8);
	DEFINE_FAST_READ_OPERATOR(uint32, 4);
	DEFINE_FAST_READ_OPERATOR(uint16, 2);
	DEFINE_FAST_READ_OPERATOR(uint8, 1);
	DEFINE_FAST_READ_OPERATOR(int64, 8);
	DEFINE_FAST_READ_OPERATOR(int32, 4);
	DEFINE_FAST_READ_OPERATOR(int16, 2);
	DEFINE_FAST_READ_OPERATOR(int8, 1);
	DEFINE_FAST_READ_OPERATOR(float, 4);
	DEFINE_FAST_READ_OPERATOR(double, 8);

	DEFINE_FAST_WRITE_OPERATOR(uint64, 8);
	DEFINE_FAST_WRITE_OPERATOR(uint32, 4);
	DEFINE_FAST_WRITE_OPERATOR(uint16, 2);
	DEFINE_FAST_WRITE_OPERATOR(uint8, 1);
	DEFINE_FAST_WRITE_OPERATOR(int64, 8);
	DEFINE_FAST_WRITE_OPERATOR(int32, 4);
	DEFINE_FAST_WRITE_OPERATOR(int16, 2);
	DEFINE_FAST_WRITE_OPERATOR(int8, 1);
	DEFINE_FAST_WRITE_OPERATOR(float, 4);
	DEFINE_FAST_WRITE_OPERATOR(double, 8);

	/** boolean (1-byte) read/write operators
	 */
	DEFINE_FAST_WRITE_OPERATOR(bool, 1);
	ByteBuffer& operator >> (bool & dst) { dst = (Read<char>() > 0 ? true : false); return *this; }

	/** string (null-terminated) operators
	 */
	ByteBuffer& operator << (const std::string & value) { EnsureBufferSize(value.length() + 1); memcpy(&m_buffer[m_writePos], value.c_str(), value.length()+1); m_writePos += (value.length() + 1); return *this; }
	ByteBuffer& operator >> (std::string & dest)
	{
		dest.clear();
		char c;
		for(;;)
		{
			c = Read<char>();
			if(c == 0) break;
			dest += c;
		}
		return *this;
	}

	/** Gets the write position
	 * @return buffer size
	 */
	inline size_t size() { return m_writePos; }

	/** read/write position setting/getting
	 */
	inline size_t rpos() { return m_readPos; }
	inline size_t wpos() { return m_writePos; }
	inline void rpos(size_t p) { assert(p <= m_writePos); m_readPos = p; }
	inline void wpos(size_t p) { assert(p <= m_buffersize); m_writePos = p; }

	template<typename T> size_t writeVector(std::vector<T> &v)
	{
        	for (typename std::vector<T>::const_iterator i = v.begin(); i != v.end(); i++) {
                	Write<T>(*i);
        	}
        	return v.size();
		
	}
	template<typename T> size_t readVector(size_t vsize, std::vector<T> &v)
	{
		
		v.clear();
		while(vsize--) {
			T t = Read<T>();
			v.push_back(t);
		}
		return v.size();
	}

	template<typename T> size_t writeList(std::list<T> &v)
	{
        	for (typename std::list<T>::const_iterator i = v.begin(); i != v.end(); i++) {
                	Write<T>(*i);
        	}
        	return v.size();
		
	}
	template<typename T> size_t readList(size_t vsize, std::list<T> &v)
	{
		
		v.clear();
		while(vsize--) {
			T t = Read<T>();
			v.push_back(t);
		}
		return v.size();
	}

	template <typename K, typename V> size_t writeMap(const std::map<K, V> &m)
	{
		for (typename std::map<K, V>::const_iterator i = m.begin(); i != m.end(); i++) {
			Write<K>(i->first);
			Write<V>(i->second);
		}
		return m.size();
	}

	template <typename K, typename V> size_t readMap(size_t msize, std::map<K, V> &m)
	{
		m.clear();
		while(msize--) {
			K k = Read<K>();
			V v = Read<V>();
			m.insert(make_pair(k, v));
		}
		return m.size();
	}

};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <typename T> ByteBuffer &operator<<(ByteBuffer &b, const std::vector<T> & v)
{
	b << (uint32)v.size();
	for (typename std::vector<T>::const_iterator i = v.begin(); i != v.end(); i++) {
		b << *i;
	}
	return b;
}

template <typename T> ByteBuffer &operator>>(ByteBuffer &b, std::vector<T> &v)
{
	uint32 vsize;
	b >> vsize;
	v.clear();
	while(vsize--) {
		T t;
		b >> t;
		v.push_back(t);
	}
	return b;
}

template <typename T> ByteBuffer &operator<<(ByteBuffer &b, const std::list<T> & v)
{
	b << (uint32)v.size();
	for (typename std::list<T>::const_iterator i = v.begin(); i != v.end(); i++) {
		b << *i;
	}
	return b;
}

template <typename T> ByteBuffer &operator>>(ByteBuffer &b, std::list<T> &v)
{
	uint32 vsize;
	b >> vsize;
	v.clear();
	while(vsize--) {
		T t;
		b >> t;
		v.push_back(t);
	}
	return b;
}

template <typename K, typename V> ByteBuffer &operator<<(ByteBuffer &b, const std::map<K, V> &m)
{
	b << (uint32)m.size();
	for (typename std::map<K, V>::const_iterator i = m.begin(); i != m.end(); i++) {
		b << i->first << i->second;
	}
	return b;
}

template <typename K, typename V> ByteBuffer &operator>>(ByteBuffer &b, std::map<K, V> &m)
{
	uint32 msize;
	b >> msize;
	m.clear();
	while(msize--) {
		K k;
		V v;
		b >> k >> v;
		m.insert(make_pair(k, v));
	}
	return b;
}

#endif
