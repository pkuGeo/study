/**
 * cityhashtable.h
 *
 * Author: Geo Zhou
 *
 * Date: 2012-06-25
 *
 * Description:
 *    该文件提供“城市编码->和该城市相关的数据”的哈希表相关的类
 *    其中有模板类CCityHashTable和CCityHashTableIterator
 *
 *    CCityHashTable是处理哈希逻辑的主要的类
 *    CCityHashTableIterator采用Iterator模式提供遍历访问CCityHashTable的每个city code的方法
 *    CCityHashTable::CDuplicateCityException是在向哈希表中添加同一个城市代码时会抛出异常类
 *    CCityHashTableIterator::CInvalidIterator在迭代器试图访问哈希表空间以外的数据时会抛出的异常类
 *
 *    需要最少的时间快速简单使用可只看CCityHashTable类
 *
 *    基本思想：默认的哈希表空间为512，采用拉链法解决碰撞，故元素个数不限于512个，但是如果碰撞过多，效率会下降明显
 *
 * Sample Code:
 *	//构建哈希表
 *	CCityHashTable<FILE*> cityHash;
 *
 *	foreach int citycode
 *	{
 *		char *relatedCityFilePath = ...;
 *		FILE *fp = fopen(relatedCityFilePath,"rb");
 *		cityHash.addCityAndVal(citycode,fp);
 *	}
 *
 *	//使用哈希表
 *	int someCityCode = ...;
 *	CCityHashTable<FILE*>::iterator iter = cityHash.iteratorOfCityCode(someCityCode);
 *	assert(iter!=cityhash.end());//确保该城市在哈希表中
 *	fseek(*iter);
 *	fread(*iter,...);
 *	...
 *
 *  //释放资源，非必须，取决于T的类型，如果放进去的元素需要释放，譬如这里的FILE*,则需要以下代码
 *	for (CCityHashTable<FILE*>::iterator iter = cityHash.begin();
 *		iter != cityHash.end();
 *		++iter) {
 *			fclose(*iter);
 *	}
 */



#ifndef _LT_CITYHASH_TABLE_H_
#define _LT_CITYHASH_TABLE_H_

#include <exception>
#include <cstring>
//前向声明
template <class T> class CCityHashTable;
template <class T> class CCityHashTableIterator;



/**
 * 城市的哈希表模板类
 * 注意该类非线程安全，并且没有处理const对象的函数
 * 拷贝构造函数和赋值操作符暂时被禁用
 */
template <class T>
class CCityHashTable
{
    
public:
    
	/**
	 * 和哈希表有关的异常：
	 *     在往哈希表里添加同一个城市（city code前四位相同）会抛出如下异常
	 */
	class CDuplicateCityException : public std::exception
	{
	public:
		virtual const char* what() const throw()
		{
			return "CDuplicateCityException: A city code is being added for a second time.";
		}
	};
    
	typedef CCityHashTableIterator<T> iterator;
    
	/*
	 * 构造函数
	 */
	CCityHashTable();
    
	/*
	 * 析构函数
	 */
	virtual ~CCityHashTable(void);
    
	/*
	 * 获得指向hash表第一个元素的迭代器
	 */
	iterator begin();
    
	
	/*
	 * 获得指向hash表最末一个元素后一个位置的迭代器
	 */
	iterator end();
    
	//暂不提供reverse_iterator
	///*
	// * 获得指向hash表倒数第一个元素的迭代器
	// */
	//reverse_iterator rbegin();
	//
	///*
	// * 获得指向hash表第一个元素前一个位置的迭代器
	// */
	//reverse_iterator rend();
    
    
	/*
	 * 获得哈希表中城市的个数
	 */
	int size();
    
	/*
	 * 返回该哈希表是否为空，即size()==0
	 */
	bool empty();
    
	/*
	 * 获得指向citycode对应的元素的迭代器，如果该citycode不在哈希表中，返回end()
	 */
	CCityHashTableIterator<T> iteratorOfCityCode(int citycode);
    
	/*
	 * 判断某城市是否在该哈希表中
	 */
	bool isCityCodeExist(int citycode);
	
	/*
	 * 增加城市代码和value，如果增加同一个城市，会抛出异常
	 */
	void addCityAndVal(int citycode,T &val) throw(...);
    
	/*
	 * 删除城市，成功则返回true，如果该城市在哈希表中不存在则返回false
	 */
	bool removeCity(int citycode);
    
	/*
	 * 设置城市对应的值，如果哈希表中没有对应的城市，则会添加该城市
	 */
	void setValOfCity(int citycode, T& val);
	
private:
	//友元，便于迭代器访问内部元素
	friend class CCityHashTableIterator<T>;
    
	//哈希表中的链表（为解决碰撞）
	template <class Type>
	struct SCityValPair
	{
		int m_nCityCode;
		SCityValPair<Type> *m_pLast;
		SCityValPair<Type> *m_pNext;
		Type m_Val;
	};
    
	/*
	 * 计算citycode对应的城市代码
	 */
	int hashValOfCityCode(int citycode);
    
	/*
	 * 禁用拷贝构造函数和赋值操作符
	 */
	CCityHashTable(const CCityHashTable&);
	CCityHashTable& operator=(const CCityHashTable&);
    
	static const int HASH_SIZE = 512;
	SCityValPair<T> **m_arrHashTable;
	int m_cityCnt;
	int m_lastIdx;//最后一个有元素链表头结点所在的数组下标
	int m_firstIdx;//第一个有元素链表头结点所在的数组下标
};

template <class T>
class CCityHashTableIterator
{
public:
	CCityHashTableIterator(const CCityHashTableIterator& iter);
	CCityHashTableIterator& operator=(const CCityHashTableIterator& iter);
	T& operator*() throw(...);
	T* operator->() throw(...);
	CCityHashTableIterator& operator++();
	CCityHashTableIterator operator++(int);
	bool operator==(const CCityHashTableIterator&);
	bool operator!=(const CCityHashTableIterator&);
	/**
	 * 和迭代器有关的异常：
	 *     访问越界会抛出异常
	 */
	class CInvalidIterator : public std::exception
	{
	public:
		virtual const char* what() const throw()
		{
			return "CInvalidIterator: Trying access a city element which is not in the hash table.";
		}
	};
private:
	CCityHashTableIterator();
	bool isInRange();
	friend class CCityHashTable<T>;
	CCityHashTable<T> *m_pCityHashTable;//对应的hash表
	int m_nIdx;//在哈希表中的下标
	typename CCityHashTable<T>::template SCityValPair<T> *m_pPairOfCityVal;
};



template <class T>
void CCityHashTable<T>::addCityAndVal(int citycode, T& val) throw(...)
{
	//哈希值
	int hashval = this->hashValOfCityCode(citycode);
    
	SCityValPair<T> *node = this->m_arrHashTable[hashval];
    
	//没有碰撞，直接加入
	if (node==NULL) {
		node = new SCityValPair<T>();
		node->m_pNext = NULL;
		node->m_pLast = NULL;
		node->m_Val = val;
		node->m_nCityCode = citycode;
		this->m_arrHashTable[hashval] = node;
	}
	else {
		//拉链法解决碰撞
		while (node->m_pNext!=NULL) {
			if (node->m_nCityCode==citycode) {
				throw CDuplicateCityException();
			}
			node = node->m_pNext;
		}
		SCityValPair<T> *newnode = new SCityValPair<T>();
		node->m_pNext = newnode;
		newnode->m_pNext = NULL;
		newnode->m_pLast = node;
		newnode->m_Val = val;
		newnode->m_nCityCode = citycode;
        
		
	}
	++this->m_cityCnt;
	if (this->m_lastIdx < hashval) {
		this->m_lastIdx = hashval;
	}
	if (this->m_firstIdx > hashval) {
		this->m_firstIdx = hashval;
	}
}

template <class T>
typename CCityHashTable<T>::iterator CCityHashTable<T>::begin()
{
	if (this->m_cityCnt == 0) {
		return this->end();
	}
	iterator iter;
	iter.m_pCityHashTable = this;
	iter.m_nIdx = this->m_firstIdx;
	iter.m_pPairOfCityVal = this->m_arrHashTable[iter.m_nIdx];
	return iter;
}


template <class T>
CCityHashTable<T>::CCityHashTable()
: m_lastIdx(-1)
, m_cityCnt(0)
, m_firstIdx(HASH_SIZE)
{
	m_arrHashTable = new SCityValPair<T>* [HASH_SIZE];
	memset(m_arrHashTable,0,HASH_SIZE*sizeof(SCityValPair<T>*));
}

template <class T>
bool CCityHashTable<T>::empty()
{
	return this->m_cityCnt == 0;
}

template <class T>
typename CCityHashTable<T>::iterator CCityHashTable<T>::end()
{
	iterator iter;
	iter.m_pCityHashTable = this;
	iter.m_nIdx = HASH_SIZE;
	iter.m_pPairOfCityVal = NULL;
	return iter;
}

template <class T>
int CCityHashTable<T>::hashValOfCityCode(int citycode)
{
	return (citycode*2654435761)%HASH_SIZE;
}

template <class T>
bool CCityHashTable<T>::isCityCodeExist(int citycode)
{
	//哈希值
	int hashval = this->hashValOfCityCode(citycode);
    
	SCityValPair<T> *node = this->m_arrHashTable[hashval];
    
	while(node!=NULL) {
		if (node->m_nCityCode==citycode) {
			return true;
		}
		node=node->m_pNext;
	}
	return false;
}

template <class T>
typename CCityHashTable<T>::iterator CCityHashTable<T>::iteratorOfCityCode(int citycode)
{
	iterator iter = this->end();
    
	//哈希值
	int hashval = this->hashValOfCityCode(citycode);
    
	SCityValPair<T> *node = this->m_arrHashTable[hashval];
    
	while(node!=NULL) {
		if (node->m_nCityCode==citycode) {
			iter.m_nIdx = hashval;
			iter.m_pPairOfCityVal = node;
			return iter;
		}
		node=node->m_pNext;
	}
	return iter;
    
}

//template <class T>
//typename CCityHashTable<T>::reverse_iterator CCityHashTable<T>::rbegin()
//{
//	if (this->m_cityCnt == 0) {
//		return this->rend();
//	}
//	iterator iter;
//	iter.m_pCityHashTable = this;
//	iter.m_nIdx = this->m_lastIdx;
//	iter.m_pPairOfCityVal = this->m_arrHashTable[iter.m_nIdx];
//	return iter;
//}


template <class T>
bool CCityHashTable<T>::removeCity(int citycode)
{
	iterator iter = this->iteratorOfCityCode(citycode);
	if (iter==this->end()) {
		return false;
	}
	SCityValPair<T> *last = iter.m_pPairOfCityVal->last;
	SCityValPair<T> *next = iter.m_pPairOfCityVal->next;
    
	//删除内存
	delete iter.m_pPairOfCityVal;
	//减少hash表元素计数
	--this->m_cityCnt;
    
	//将链表节点从双链表中删除
	if (last!=NULL) {
		last->m_pNext = next;
	}
	if (next!=NULL) {
		next->m_pLast = last;
	}
    
	//该节点为链表头节点,修改新的头结点
	if (last==NULL) {
		this->m_arrHashTable[iter.m_nIdx] = next;
		
	}
    
	//修改新的firstIdx和lastIdx
	if (next==NULL && last==NULL) {
		if (iter.m_nIdx == m_firstIdx) {
			//往后寻找新的firstIdx，修改firstIdx
			int i = m_firstIdx+1;
			m_firstIdx = HASH_SIZE;
			for (; i<=m_lastIdx; ++i) {
				if (this->m_arrHashTable[i]!=NULL) {
					m_firstIdx = i;
					break;
				}
			}
		}
		if (iter.m_nIdx == m_lastIdx) {
			//往前寻找新的lastIdx，修改lastIdx
			int i = m_lastIdx-1;
			m_lastIdx = -1;
			for (; i>=m_firstIdx; --i) {
				if (this->m_arrHashTable[i]!=NULL) {
					m_lastIdx = i;
					break;
				}
			}
		}
	}
}

//template <class T>
//typename CCityHashTable<T>::reverse_iterator CCityHashTable<T>::rend()
//{
//	iterator iter;
//	iter.m_pCityHashTable = this;
//	iter.m_nIdx = -1;
//	iter.m_pPairOfCityVal = NULL;
//	return iter;
//}


template <class T>
void CCityHashTable<T>::setValOfCity(int citycode, T& val)
{
	//哈希值
	int hashval = this->hashValOfCityCode(citycode);
    
	SCityValPair<T> *node = this->m_arrHashTable[hashval];
    
	while(node->m_pNext!=NULL) {
		if (node->m_nCityCode==citycode) {
			node->m_Val = val;
			return;
		}
		node=node->m_pNext;
	}
    
	//最后一个结点
	if (node->m_nCityCode==citycode) {
		node->m_Val = val;
		return;
	}
    
	this->addCityAndVal(citycode,val);
}

template <class T>
int CCityHashTable<T>::size()
{
	return this->m_cityCnt;
}

template <class T>
CCityHashTable<T>::~CCityHashTable()
{
	//删除每个链表
	for (int linkidx = 0; linkidx < HASH_SIZE; ++linkidx) {
		SCityValPair<T> *linkHead = m_arrHashTable[linkidx];
		while (linkHead != NULL) {
			SCityValPair<T> *temp = linkHead;
			linkHead = linkHead->m_pNext;
			delete temp;
		}
	}
	delete[] m_arrHashTable;
}

template <class T>
CCityHashTableIterator<T>::CCityHashTableIterator()
: m_pCityHashTable(NULL)
, m_nIdx(CCityHashTable<T>::HASH_SIZE)
, m_pPairOfCityVal(NULL)
{
}

template <class T>
CCityHashTableIterator<T>::CCityHashTableIterator(const CCityHashTableIterator<T> &iter)
: m_pCityHashTable(iter.m_pCityHashTable)
, m_nIdx(iter.m_nIdx)
, m_pPairOfCityVal(iter.m_pPairOfCityVal)
{
}

template <class T>
bool CCityHashTableIterator<T>::isInRange()
{
	if (this->m_pCityHashTable == NULL) {
		return false;
	}
    
	if (this->m_nIdx < 0 || this->m_nIdx >= this->m_pCityHashTable->HASH_SIZE) {
		return false;
	}
	
	typename CCityHashTable<T>::template SCityValPair<T> *node = this->m_pCityHashTable->m_arrHashTable[this->m_nIdx];
	
	if (node==NULL) {
		return false;
	}
	while (node!=NULL) {
		if (this->m_pPairOfCityVal==node) {
			return true;
		}
		node = node->m_pNext;
	}
	return false;
}

template <class T>
T& CCityHashTableIterator<T>::operator*()
{
	if (!this->isInRange()) {
		throw CInvalidIterator();
	}
	return this->m_pPairOfCityVal->m_Val;
}

template <class T>
CCityHashTableIterator<T>& CCityHashTableIterator<T>::operator++()
{
	if (this->m_pCityHashTable == NULL) {
		return *this;
	}
	if (this->m_nIdx >= 0
		&& this->m_nIdx < CCityHashTable<T>::HASH_SIZE
		&& this->m_pCityHashTable->m_arrHashTable[this->m_nIdx]!=NULL) {
        if (this->m_pPairOfCityVal->m_pNext != NULL) {
            this->m_pPairOfCityVal = this->m_pPairOfCityVal->m_pNext;
            return *this;
        }
        
        for (int i = this->m_nIdx+1; i < this->m_pCityHashTable->m_lastIdx; ++i) {
            if (this->m_pCityHashTable->m_arrHashTable[i]!=NULL) {
                this->m_nIdx = i;
                this->m_pPairOfCityVal = this->m_pCityHashTable->m_arrHashTable[i];
                return *this;
            }
        }
	}
    
	this->m_nIdx = CCityHashTable<T>::HASH_SIZE;
	this->m_pPairOfCityVal = NULL;
	return *this;
}

template <class T>
CCityHashTableIterator<T> CCityHashTableIterator<T>::operator++(int)
{
	CCityHashTableIterator<T> iter(*this);
	++(*this);
	return iter;
}


template <class T>
T* CCityHashTableIterator<T>::operator->()
{
	if (!this->isInRange()) {
		throw CInvalidIterator();
	}
	return &(this->m_pPairOfCityVal->m_Val);
}

template <class T>
CCityHashTableIterator<T>& CCityHashTableIterator<T>::operator=(const CCityHashTableIterator<T>& iter)
{
	this->m_nIdx = iter.m_nIdx;
	this->m_pCityHashTable = iter.m_pCityHashTable;
	this->m_pPairOfCityVal = iter.m_pPairOfCityVal;
	return *this;
}

template <class T>
bool CCityHashTableIterator<T>::operator==(const CCityHashTableIterator<T>& iter)
{
	if (this->m_pCityHashTable == iter.m_pCityHashTable
		&& this->m_nIdx == iter.m_nIdx
		&& this->m_pPairOfCityVal == iter.m_pPairOfCityVal) {
        return true;
	}
	return false;
}

template <class T>
bool CCityHashTableIterator<T>::operator!=(const CCityHashTableIterator<T>& iter)
{
	return !(*this==iter);
}
#endif